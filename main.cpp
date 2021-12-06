#include "main.hpp"
#include <unordered_map>
#include <utility>
#include "util.h"
#include "datatypes.hpp"
#include <iomanip>
#include <iostream>
#include <fstream>

libtrace_t* currenttrace;
static volatile int done = 0;
static volatile int StartTime = 0;
static volatile int EndTime = 0;

struct threadlocal {
    FlowManager *flowmanager;
};

/* This data structure is used to demonstrate how to use the 'extension' 
 * pointer to store custom data for a flow */
typedef struct ident {
    uint8_t init_dir;
    uint64_t in_bytes;
    uint64_t out_bytes;
    uint64_t in_pkts;
    uint64_t out_pkts;
    double start_ts;
    double end_ts;

    bool classified;
    char proto_name[50];
    lpi_data_t lpi;
} IdentFlow;

int IPToValue(const std::string& strIP)
{
	int a[4];
	std::string IP = strIP;
	std::string strTemp;
	size_t pos;
	size_t i=3;
 
	do
	{
		pos = IP.find("."); 
	
		if(pos != std::string::npos)
		{		
			strTemp = IP.substr(0,pos);	
			a[i] = atoi(strTemp.c_str());		
			i--;		
			IP.erase(0,pos+1);
		}
		else
		{					
			strTemp = IP;
			a[i] = atoi(strTemp.c_str());			
			break;
		}
 
	}while(1);
 
	int nResult = (a[3]<<24) + (a[2]<<16)+ (a[1]<<8) + a[0];
	return nResult;
}

std::string ValueToIP(const int& nValue)
{
	char strTemp[20];
	sprintf( strTemp,"%d.%d.%d.%d",
		(nValue&0xff000000)>>24,
		(nValue&0x00ff0000)>>16,
		(nValue&0x0000ff00)>>8,
		(nValue&0x000000ff) );
 
	return std::string(strTemp);
}

static void *start_processing(libtrace_t *trace, libtrace_thread_t *thread, 
    void *global) {

    bool opt_false = false;
    struct globalopts *opts = (struct globalopts *)global;

    struct threadlocal *tl = (struct threadlocal *)malloc(sizeof(
        struct threadlocal));
    tl->flowmanager = new FlowManager();

    /* This tells libflowmanager to ignore any flows where an RFC1918
	 * private IP address is involved */
    if (tl->flowmanager->setConfigOption(LFM_CONFIG_IGNORE_RFC1918, 
        &(opts->ignore_rfc1918)) == 0) {
        fprintf(stderr, "Failed to set IGNORE RFC 1918 option in libflowmanager\n");
    }

	/* This tells libflowmanager not to replicate the TCP timewait
	 * behaviour where closed TCP connections are retained in the Flow
	 * map for an extra 2 minutes */
    if (tl->flowmanager->setConfigOption(LFM_CONFIG_TCP_TIMEWAIT, 
        &opt_false) == 0) {
        fprintf(stderr, "Failed to set TCP TIMEWAIT option in libflowmanager\n");
    }

	/* This tells libflowmanager not to utilise the fast expiry rules for
	 * short-lived UDP connections - these rules are experimental 
	 * behaviour not in line with recommended "best" practice */
	if (tl->flowmanager->setConfigOption(LFM_CONFIG_SHORT_UDP, 
        &opt_false) == 0) {
        fprintf(stderr, "Failed to set SHORT UDP option in libflowmanager\n");
    }
    
    return tl;
}

static void *start_reporter(libtrace_t *trace, libtrace_thread_t *thread, 
    void *global) {
    return NULL;
}

static void stop_reporter(libtrace_t *trace, libtrace_thread_t *thread, 
    void *global, void *tls) {
    if (tls)
        free(tls);
}

/* Initialises the custom data for the given flow. Allocates memory for a
 * IdentFlow structure and ensures that the extension pointer points at
 * it.
 */
void init_ident_flow(Flow *f, uint8_t dir, double ts) {
    IdentFlow *ident = NULL;

    ident = (IdentFlow *)malloc(sizeof(IdentFlow));
    ident->init_dir = dir;
    ident->in_bytes = 0;
    ident->out_bytes = 0;
    ident->in_pkts = 0;
    ident->out_pkts = 0;
    ident->start_ts = ts;
    ident->end_ts = ts;

    ident->classified = false;

    lpi_init_data(&ident->lpi);
    f->extension = ident;
}

void dump_payload(lpi_data_t lpi, uint8_t dir, char *space, int spacelen) {

    int i;
    uint8_t *pl = (uint8_t *)(&(lpi.payload[dir]));

    char ascii[4][5];

    for (i = 0; i < 4; i++) {
	    if (*pl > 32 && *pl < 126) {
            snprintf(ascii[i], 5, "%c", *pl);
	    } else {
            snprintf(ascii[i], 5, ".");
	    }
	    pl++;
    }

    snprintf(space, spacelen - 1, "%08x %s%s%s%s %u", 
        ntohl(lpi.payload[dir]), ascii[0], ascii[1], 
        ascii[2], ascii[3], lpi.payload_len[dir]);
}

char *display_ident(Flow *f, IdentFlow *ident, struct globalopts *opts) {

    char s_ip[100];
    char c_ip[100];
    //char pload_out[100];
    //char pload_in[100];
    char *str;
    lpi_module_t *proto;

    if (opts->only_dir0 && ident->init_dir == 1)
	    return NULL;
    if (opts->only_dir1 && ident->init_dir == 0)
	    return NULL;
    if (opts->require_both) {
	    if (ident->lpi.payload_len[0] == 0 || 
            ident->lpi.payload_len[1] == 0) {
		    return NULL;
	    }
    }

    if (opts->nat_hole) {
        if (ident->init_dir != 1)
            return NULL;
        if (ident->lpi.payload_len[0] == 0 && ident->in_pkts <= 3)
            return NULL;
    }

    proto = lpi_guess_protocol(&ident->lpi);
    if(proto == NULL || proto->protocol == LPI_PROTO_INVALID || proto->protocol == LPI_PROTO_NO_PAYLOAD || proto->protocol == LPI_PROTO_NO_FIRSTPKT || proto->protocol == LPI_PROTO_ICMP || proto->protocol == LPI_PROTO_UNKNOWN || proto->protocol == LPI_PROTO_UDP || proto->protocol == LPI_PROTO_UNSUPPORTED) {
	return NULL;
    }

    f->id.get_server_ip_str(s_ip);
    f->id.get_client_ip_str(c_ip);

    //dump_payload(ident->lpi, 0, pload_out, 100);
    //dump_payload(ident->lpi, 1, pload_in, 100);
    str = (char *)malloc(750);
    /*snprintf(str, 750, "%s %s %s %u %u %u %.3f %.3f %" PRIu64 " %" PRIu64 " %s %s\n", 
        proto->name, s_ip, c_ip, 
        f->id.get_server_port(), f->id.get_client_port(), 
        f->id.get_protocol(), ident->start_ts, 
        ident->end_ts, 
        ident->out_bytes, ident->in_bytes, 
        pload_out, pload_in);*/

    uint64_t bytes_sum = ident->out_bytes + ident->in_bytes;
    snprintf(str, 750, "%s %s %s %u %u %u %" PRIu64 "\n", 
        proto->name, s_ip, c_ip, 
        f->id.get_server_port(), f->id.get_client_port(), 
        f->id.get_protocol(), bytes_sum);

    return str;
}

char *display_packet(Flow *f, IdentFlow *ident, struct globalopts *opts, uint64_t packet_size) {

    char s_ip[100];
    char c_ip[100];
    char *str;

    //opts->require_both changed!!!
    if (opts->only_dir0 && ident->init_dir == 1)
	    return NULL;
    if (opts->only_dir1 && ident->init_dir == 0)
	    return NULL;
    if (opts->require_both) {
	    /*if (ident->lpi.payload_len[0] == 0 || 
            ident->lpi.payload_len[1] == 0) {
		    return NULL;
	    }*/
    }

    if (opts->nat_hole) {
        if (ident->init_dir != 1)
            return NULL;
        if (ident->lpi.payload_len[0] == 0 && ident->in_pkts <= 3)
            return NULL;
    }

    f->id.get_server_ip_str(s_ip);
    f->id.get_client_ip_str(c_ip);

    str = (char *)malloc(750);
    snprintf(str, 750, "%s %s %s %u %u %u %" PRIu64 "\n", 
        ident->proto_name, s_ip, c_ip, 
        f->id.get_server_port(), f->id.get_client_port(), 
        f->id.get_protocol(), packet_size);

    return str;
}

/* Expires all flows that libflowmanager believes have been idle for too
 * long. The exp_flag variable tells libflowmanager whether it should force
 * expiry of all flows (e.g. if you have reached the end of the program and
 * want the stats for all the still-active flows). Otherwise, only flows
 * that have been idle for longer than their expiry timeout will be expired.
 */
void expire_ident_flows(libtrace_t *trace, libtrace_thread_t *thread, 
    struct globalopts *opts, FlowManager *fm, double ts, 
    bool exp_flag) {
    Flow *expired;
    char *result = NULL;
    libtrace_generic_t gen;

    /* Loop until libflowmanager has no more expired flows available */
    while ((expired = fm->expireNextFlow(ts, exp_flag)) != NULL) {

        IdentFlow *ident = (IdentFlow *)expired->extension;
	if(!ident->classified) {
	    result = display_ident(expired, ident, opts);
	    if (result) {
		gen.ptr = result;
		trace_publish_result(trace, thread, ident->end_ts, gen, RESULT_USER);
	    }
	}
        /*result = display_ident(expired, ident, opts);
        if (result) {
            gen.ptr = result;
            trace_publish_result(trace, thread, ident->end_ts, 
                gen, RESULT_USER);
        }*/
        /* Don't forget to free our custom data structure */
        free(ident);

        fm->releaseFlow(expired);
    }
}

static void Updating_Sketch(void *global, tuple_t t){

	struct globalopts *opts = (struct globalopts *)global;

	double updatetime=0, updatetime_t=0;
	
	opts->sum += t.bytes_sum;
	key_tp key;
	key.key[0] = (uint32_t)t.key.AppProto;
	key.key[1] = t.key.src_ip;
	if (opts->groundedgetmp.find(key) != opts->groundedgetmp.end()) {
		opts->groundedgetmp[key][0] += t.bytes_sum;
	} else {
		val_tp* valtuple = new val_tp[2]();
		opts->groundedgetmp[key] = valtuple;
		opts->groundedgetmp[key][0] += t.bytes_sum;
	}

	key_tp keynode;
	keynode.key[0] = (uint32_t)t.key.AppProto;
	if (opts->groundnodetmp.find(keynode) != opts->groundnodetmp.end()) {
		opts->groundnodetmp[keynode][0] += t.bytes_sum;
	} else {
		val_tp* valtuple = new val_tp[2]();
		opts->groundnodetmp[keynode] = valtuple;
		opts->groundnodetmp[keynode][0] += t.bytes_sum;
	}

	key_tp keynode2;
	keynode2.key[1] = t.key.src_ip;
	if (opts->groundnodetmp2.find(keynode2) != opts->groundnodetmp2.end()) {
		opts->groundnodetmp2[keynode2][0] += t.bytes_sum;
	} else {
		val_tp* valtuple = new val_tp[2]();
		opts->groundnodetmp2[keynode2] = valtuple;
		opts->groundnodetmp2[keynode2][0] += t.bytes_sum;
	}
	//std::cout << opts->groundedgetmp.size() << std::endl;
	//std::cout << opts->groundnodetmp.size() << std::endl;
	//std::cout << opts->groundnodetmp2.size() << std::endl;

	//Update AppSketch
	uint64_t t1, t2;
	DMatrix* cursk = (DMatrix*)opts->heavychangerdm->GetCurSketch();
	t1 = now_us();

	//key_tp key;
	key.key[0] = (uint32_t)t.key.AppProto;
	key.key[1] = t.key.src_ip;
	uint64_t total_size = t.bytes_sum;
	cursk->Update(key.key, (val_tp)total_size);
	
	t2 = now_us();
	updatetime = (double)(t2-t1)/1000000000;
	opts->avrupdatetime += updatetime;

	//Update Improved TCM
	SDMatrix* cursk_t = (SDMatrix*)opts->heavychangersdm->GetCurSketch();
	t1 = now_us();
	
	//key_tp key;
	key.key[0] = (uint32_t)t.key.AppProto;
	key.key[1] = t.key.src_ip;
	//uint64_t total_size = t.bytes_sum;
	cursk_t->Update(key.key, (val_tp)total_size);
	
	t2 = now_us();
	updatetime_t = (double)(t2-t1)/1000000000;
	opts->avrupdatetime_t += updatetime_t;
}

static void Query(void *global){
	struct globalopts *opts = (struct globalopts *)global;
	
	double querytime9=0;
	double ia_TopkApp=0;  //intersection accuracy
	double js_TopkApp=0;  //jaccard similarity
	double ndcg_TopkApp=0;  //normalized discounted cumulative gain
	double reerTopkApp=0;  //relative error
	double querytime10=0;
	double ia_TopkUser=0;
	double js_TopkUser=0;
	double ndcg_TopkUser=0;
	double reerTopkUser=0;
	
	double querytime9_t=0;
	double ia_TopkApp_t=0;
	double js_TopkApp_t=0;
	double ndcg_TopkApp_t=0;
	double reerTopkApp_t=0;
	double querytime10_t=0;
	double ia_TopkUser_t=0;
	double js_TopkUser_t=0;
	double ndcg_TopkUser_t=0;
	double reerTopkUser_t=0;

	double reerTopkApp2=0;
	double reerTopkApp_t2=0;
	double reerTopkUser2=0;
	double reerTopkUser_t2=0;

	uint64_t t1, t2;
	
	//Top-k Ground Truth
	std::pair<key_tp, val_tp> GT_topk_apps[opts->K];
	std::pair<key_tp, val_tp> GT_topk_users[opts->K2];
	for (auto it = opts->groundnodetmp.begin(); it != opts->groundnodetmp.end(); it++) {
		int gt_idx = opts->K-1;
		if (it->second[0] > GT_topk_apps[gt_idx].second) {
		while (it->second[0] > GT_topk_apps[gt_idx-1].second) {
			GT_topk_apps[gt_idx] = GT_topk_apps[gt_idx-1];
			gt_idx--;
			if (gt_idx == 0) {
				break;
			}
		}
		std::pair<key_tp, val_tp> temp;
		temp = std::make_pair(it->first, it->second[0]);
		GT_topk_apps[gt_idx] = temp;
		}
	}
	for (auto it = opts->groundnodetmp2.begin(); it != opts->groundnodetmp2.end(); it++) {
		int gt_idx = opts->K2-1;
		if (it->second[0] > GT_topk_users[gt_idx].second) {
		while (it->second[0] > GT_topk_users[gt_idx-1].second) {
			GT_topk_users[gt_idx] = GT_topk_users[gt_idx-1];
			gt_idx--;
			if (gt_idx == 0) {
				break;
			}
		}
		std::pair<key_tp, val_tp> temp;
		temp = std::make_pair(it->first, it->second[0]);
		GT_topk_users[gt_idx] = temp;
		}
	}
	std::cout << "Top-k Apps: " << std::endl;
	for(int i = 0; i < opts->K; i++) {
		std::cout << "(" << lpi_print((lpi_protocol_t)GT_topk_apps[i].first.key[0]) << ", " << GT_topk_apps[i].first.key[1];
		std::cout << ")->" << GT_topk_apps[i].second << std::endl;
	}
	std::cout << "Top-k Users: " << std::endl;
	for(int i = 0; i < opts->K2; i++) {
		std::cout << "(" << GT_topk_users[i].first.key[0] << ", " << ValueToIP(GT_topk_users[i].first.key[1]);
		std::cout << ")->" << GT_topk_users[i].second << std::endl;
	}
	
	//Query for Top-k Apps
	DMatrix* cursk = (DMatrix*)opts->heavychangerdm->GetCurSketch();
	SDMatrix* cursk_t = (SDMatrix*)opts->heavychangersdm->GetCurSketch();
	
	std::pair<key_tp, val_tp> *es_topk_apps = new std::pair<key_tp, val_tp>[opts->K];
	memset(es_topk_apps, -1, sizeof(std::pair<key_tp, val_tp>)*opts->K);
	t1 = now_us();
	cursk->QueryTopkApps(opts->K, es_topk_apps);
	t2 = now_us();
	querytime9 = (double)(t2-t1)/1000000000;
	opts->avrquerytime9 += querytime9;

	std::pair<key_tp, val_tp> *es_topk_apps_t = new std::pair<key_tp, val_tp>[opts->K];
	memset(es_topk_apps_t, -1, sizeof(std::pair<key_tp, val_tp>)*opts->K);
	t1 = now_us();
	cursk_t->QueryTopkApps(opts->K, es_topk_apps_t);
	t2 = now_us();
	querytime9_t = (double)(t2-t1)/1000000000;
	opts->avrquerytime9_t += querytime9_t;

	int tp_app = 0;
	reerTopkApp = 0;
	int tp_app_t = 0;
	reerTopkApp_t = 0;
	double dcg_app = 0;
	double dcg_app_t = 0;
	double idcg_app = 0;
		
	for(int i = 0; i < opts->K; i++) {
		val_tp value1 = 0;
		val_tp value2 = 0;
		for(int j = 0; j < opts->K; j++) {
			if(GT_topk_apps[i].first.key[0] == es_topk_apps[j].first.key[0]) {
				tp_app++;
				value1 = es_topk_apps[j].second;
			}
			if(GT_topk_apps[i].first.key[0] == es_topk_apps_t[j].first.key[0]) {
				tp_app_t++;
				value2 = es_topk_apps_t[j].second;
			}
		}
		if (value1 != 0 && value2 != 0) {
			reerTopkApp += abs(long(GT_topk_apps[i].second - value1))*1.0/GT_topk_apps[i].second;
			reerTopkApp_t += abs(long(GT_topk_apps[i].second - value2))*1.0/GT_topk_apps[i].second;
		}
		if (value1 != 0 || value2 != 0) {
			reerTopkApp2 += abs(long(GT_topk_apps[i].second - value1))*1.0/GT_topk_apps[i].second;
			reerTopkApp_t2 += abs(long(GT_topk_apps[i].second - value2))*1.0/GT_topk_apps[i].second;
		}
	}
	if (tp_app == 0) {
		reerTopkApp2 = 0;
	}
	else {
		reerTopkApp2 = reerTopkApp2/tp_app;
	}
	opts->avrreerTopkApp2 += reerTopkApp2;
	if (tp_app_t == 0) {
		reerTopkApp_t2 = 0;
	}
	else {
		reerTopkApp_t2 = reerTopkApp_t2/tp_app_t;
	}
	opts->avrreerTopkApp_t2 += reerTopkApp_t2;

	for(int i = 0; i < opts->K; i++) {
		for(int j = 0; j < opts->K; j++) {
			if(es_topk_apps[i].first.key[0] == GT_topk_apps[j].first.key[0]) {
				dcg_app += (opts->K-abs(i-j))/(log(i+2)/log(2));
				//dcg_app += (pow(2,(opts->K-abs(i-j)))-1)/(log(i+2)/log(2));
			}
			if(es_topk_apps_t[i].first.key[0] == GT_topk_apps[j].first.key[0]) {
				dcg_app_t += (opts->K-abs(i-j))/(log(i+2)/log(2));
				//dcg_app_t += (pow(2,(opts->K-abs(i-j)))-1)/(log(i+2)/log(2));
			}
		}
		idcg_app += opts->K/(log(i+2)/log(2));
		//idcg_app += (pow(2,opts->K)-1)/(log(i+2)/log(2));
	}

	ia_TopkApp = tp_app*1.0/opts->K;
	js_TopkApp = tp_app*1.0/(2*opts->K-tp_app);
	ndcg_TopkApp = dcg_app*1.0/idcg_app;
	if (tp_app == 0) {
		reerTopkApp = 0;
	}
	else {
		reerTopkApp = reerTopkApp/tp_app;
	}
	opts->avria_TopkApp += ia_TopkApp;
	opts->avrjs_TopkApp += js_TopkApp;
	opts->avrndcg_TopkApp += ndcg_TopkApp;
	opts->avrreerTopkApp += reerTopkApp;

	ia_TopkApp_t = tp_app_t*1.0/opts->K;
	js_TopkApp_t = tp_app_t*1.0/(2*opts->K-tp_app_t);
	ndcg_TopkApp_t = dcg_app_t*1.0/idcg_app;
	if (tp_app_t == 0) {
		reerTopkApp_t = 0;
	}
	else {
		reerTopkApp_t = reerTopkApp_t/tp_app_t;
	}
	opts->avria_TopkApp_t += ia_TopkApp_t;
	opts->avrjs_TopkApp_t += js_TopkApp_t;
	opts->avrndcg_TopkApp_t += ndcg_TopkApp_t;
	opts->avrreerTopkApp_t += reerTopkApp_t;


	//Query for Top-k Users
	std::pair<key_tp, val_tp> *es_topk_users = new std::pair<key_tp, val_tp>[opts->K2];
	memset(es_topk_users, -1, sizeof(std::pair<key_tp, val_tp>)*opts->K2);
	t1 = now_us();
	cursk->QueryTopkUsers(opts->K2, es_topk_users);
	t2 = now_us();
	querytime10 = (double)(t2-t1)/1000000000;
	opts->avrquerytime10 += querytime10;

	std::pair<key_tp, val_tp> *es_topk_users_t = new std::pair<key_tp, val_tp>[opts->K2];
	memset(es_topk_users_t, -1, sizeof(std::pair<key_tp, val_tp>)*opts->K2);
	t1 = now_us();
	cursk_t->QueryTopkUsers(opts->K2, es_topk_users_t);
	t2 = now_us();
	querytime10_t = (double)(t2-t1)/1000000000;
	opts->avrquerytime10_t += querytime10_t;

	int tp_user = 0;
	reerTopkUser = 0;
	int tp_user_t = 0;
	reerTopkUser_t = 0;
	double dcg_user = 0;
	double dcg_user_t = 0;
	double idcg_user = 0;
	
	for(int i = 0; i < opts->K2; i++) {
		val_tp value1 = 0;
		val_tp value2 = 0;
		for(int j = 0; j < opts->K2; j++) {
			if(GT_topk_users[i].first.key[1] == es_topk_users[j].first.key[1]) {
				tp_user++;
				value1 = es_topk_users[j].second;
			}
			if(GT_topk_users[i].first.key[1] == es_topk_users_t[j].first.key[1]) {
				tp_user_t++;
				value2 = es_topk_users_t[j].second;
			}
			if((GT_topk_users[i].first.key[1] == es_topk_users[j].first.key[1]) && (GT_topk_users[i].first.key[1] == es_topk_users_t[j].first.key[1])) {
				reerTopkUser += abs(long(GT_topk_users[i].second - es_topk_users[j].second))*1.0/GT_topk_users[i].second;
				reerTopkUser_t += abs(long(GT_topk_users[i].second - es_topk_users_t[j].second))*1.0/GT_topk_users[i].second;
			}
		}
		if (value1 != 0 && value2 != 0) {
			reerTopkUser += abs(long(GT_topk_users[i].second - value1))*1.0/GT_topk_users[i].second;
			reerTopkUser_t += abs(long(GT_topk_users[i].second - value2))*1.0/GT_topk_users[i].second;
		}
		if (value1 != 0 || value2 != 0) {
			reerTopkUser2 += abs(long(GT_topk_users[i].second - value1))*1.0/GT_topk_users[i].second;
			reerTopkUser_t2 += abs(long(GT_topk_users[i].second - value2))*1.0/GT_topk_users[i].second;
		}
	}
	if (tp_user == 0) {
		reerTopkUser2 = 0;
	}
	else {
		reerTopkUser2 = reerTopkUser2/tp_user;
	}
	opts->avrreerTopkUser2 += reerTopkUser2;
	if (tp_user_t == 0) {
		reerTopkUser_t2 = 0;
	}
	else {
		reerTopkUser_t2 = reerTopkUser_t2/tp_user_t;
	}
	opts->avrreerTopkUser_t2 += reerTopkUser_t2;

	for(int i = 0; i < opts->K2; i++) {
		for(int j = 0; j < opts->K2; j++) {
			if(es_topk_users[i].first.key[1] == GT_topk_users[j].first.key[1]) {
				dcg_user += (opts->K2-abs(i-j))/(log(i+2)/log(2));
				//dcg_user += (pow(2,(opts->K2-abs(i-j)))-1)/(log(i+2)/log(2));
			}
			if(es_topk_users_t[i].first.key[1] == GT_topk_users[j].first.key[1]) {
				dcg_user_t += (opts->K2-abs(i-j))/(log(i+2)/log(2));
				//dcg_user_t += (pow(2,(opts->K2-abs(i-j)))-1)/(log(i+2)/log(2));
			}
		}
		idcg_user += opts->K2/(log(i+2)/log(2));
		//idcg_user += (pow(2,opts->K2)-1)/(log(i+2)/log(2));
	}

	ia_TopkUser = tp_user*1.0/opts->K2;
	js_TopkUser = tp_user*1.0/(2*opts->K2-tp_user);
	ndcg_TopkUser = dcg_user*1.0/idcg_user;
	if (tp_user == 0) {
		reerTopkUser = 0;
	}
	else {
		reerTopkUser = reerTopkUser/tp_user;
	}
	opts->avria_TopkUser += ia_TopkUser;
	opts->avrjs_TopkUser += js_TopkUser;
	opts->avrndcg_TopkUser += ndcg_TopkUser;
	opts->avrreerTopkUser += reerTopkUser;

	ia_TopkUser_t = tp_user_t*1.0/opts->K2;
	js_TopkUser_t = tp_user_t*1.0/(2*opts->K2-tp_user_t);
	ndcg_TopkUser_t = dcg_user_t*1.0/idcg_user;
	if (tp_user_t == 0) {
		reerTopkUser_t = 0;
	}
	else {
		reerTopkUser_t = reerTopkUser_t/tp_user_t;
	}
	opts->avria_TopkUser_t += ia_TopkUser_t;
	opts->avrjs_TopkUser_t += js_TopkUser_t;
	opts->avrndcg_TopkUser_t += ndcg_TopkUser_t;
	opts->avrreerTopkUser_t += reerTopkUser_t;

	std::cout << "AppSketch ES Top-k Apps: " << std::endl;
	for(int i = 0; i < opts->K; i++) {
		std::cout << "(" << lpi_print((lpi_protocol_t)es_topk_apps[i].first.key[0]) << ", " << ValueToIP(es_topk_apps[i].first.key[1]);
		std::cout << ")->" << es_topk_apps[i].second << std::endl;
	}
	std::cout << "AppSketch ES Top-k Users: " << std::endl;
	for(int i = 0; i < opts->K2; i++) {
		std::cout << "(" << es_topk_users[i].first.key[0] << ", " << ValueToIP(es_topk_users[i].first.key[1]);
		std::cout << ")->" << es_topk_users[i].second << std::endl;
	}

	std::cout << "Improved TCM ES Top-k Apps: " << std::endl;
	for(int i = 0; i < opts->K; i++) {
		std::cout << "(" << lpi_print((lpi_protocol_t)es_topk_apps_t[i].first.key[0]) << ", " << ValueToIP(es_topk_apps_t[i].first.key[1]);
		std::cout << ")->" << es_topk_apps_t[i].second << std::endl;
	}
	std::cout << "Improved TCM ES Top-k Users: " << std::endl;
	for(int i = 0; i < opts->K2; i++) {
		std::cout << "(" << es_topk_users_t[i].first.key[0] << ", " << ValueToIP(es_topk_users_t[i].first.key[1]);
		std::cout << ")->" << es_topk_users_t[i].second << std::endl;
	}

	//std::cout << std::setfill(' ');

	std::cout << "AppSketch: " << std::endl;
	std::cout << std::setw(30)<< std::left << "IA. Top-k Apps" << std::setw(30) << std::left << "JS. Top-k Apps" << std::endl;
	std::cout << std::setw(30)<< std::left << ia_TopkApp << std::setw(30) << std::left << js_TopkApp << std::endl;
	std::cout << std::setw(30)<< std::left << "NDCG. Top-k Users" << std::setw(30) << std::left << "RError. Top-k Apps" << std::endl;
	//std::cout << std::setw(30)<< std::left << ndcg_TopkApp << std::setw(30) << std::left << reerTopkApp << std::setw(30) << std::left << reerTopkApp2 << std::endl;
	std::cout << std::setw(30)<< std::left << ndcg_TopkApp << std::setw(30) << std::left << reerTopkApp2 << std::endl;
	std::cout << std::setw(30)<< std::left << "IA. Top-k Users" << std::setw(30) << std::left << "JS. Top-k Users" << std::endl;
	std::cout << std::setw(30)<< std::left << ia_TopkUser << std::setw(30) << std::left << js_TopkUser << std::endl;
	std::cout << std::setw(30)<< std::left << "NDCG. Top-k Users" << std::setw(30) << std::left << "RError. Top-k Users" << std::endl;
	//std::cout << std::setw(30)<< std::left << ndcg_TopkUser << std::setw(30) << std::left << reerTopkUser << std::setw(30) << std::left << reerTopkUser2 << std::endl;
	std::cout << std::setw(30)<< std::left << ndcg_TopkUser << std::setw(30) << std::left << reerTopkUser2 << std::endl;

	std::cout << "Improved TCM: " << std::endl;
	std::cout << std::setw(30)<< std::left << "IA. Top-k Apps" << std::setw(30) << std::left << "JS. Top-k Apps" << std::endl;
	std::cout << std::setw(30)<< std::left << ia_TopkApp_t << std::setw(30) << std::left << js_TopkApp_t << std::endl;
	std::cout << std::setw(30)<< std::left << "NDCG. Top-k Users" << std::setw(30) << std::left << "RError. Top-k Apps" << std::endl;
	//std::cout << std::setw(30)<< std::left << ndcg_TopkApp_t << std::setw(30) << std::left << reerTopkApp_t << std::setw(30) << std::left << reerTopkApp_t2 << std::endl;
	std::cout << std::setw(30)<< std::left << ndcg_TopkApp_t << std::setw(30) << std::left << reerTopkApp_t2 << std::endl;
	std::cout << std::setw(30)<< std::left << "IA. Top-k Users" << std::setw(30) << std::left << "JS. Top-k Users" << std::endl;
	std::cout << std::setw(30)<< std::left << ia_TopkUser_t << std::setw(30) << std::left << js_TopkUser_t << std::endl;
	std::cout << std::setw(30)<< std::left << "NDCG. Top-k Users" << std::setw(30) << std::left << "RError. Top-k Users" << std::endl;
	//std::cout << std::setw(30)<< std::left << ndcg_TopkUser_t << std::setw(30) << std::left << reerTopkUser_t << std::setw(30) << std::left << reerTopkUser_t2 << std::endl;
	std::cout << std::setw(30)<< std::left << ndcg_TopkUser_t << std::setw(30) << std::left << reerTopkUser_t2 << std::endl;

	opts->numfile++;
	
	opts->heavychangerdm->Reset();
	opts->heavychangersdm->Reset();
	
	//Reset gounrdtmp;
	for (auto it = opts->groundedgetmp.begin(); it != opts->groundedgetmp.end(); it++) {
		it->second[1] = it->second[0];
		it->second[0] = 0;
	}

	for (auto it = opts->groundnodetmp.begin(); it != opts->groundnodetmp.end(); it++) {
		it->second[1] = it->second[0];
		it->second[0] = 0;
	}

	for (auto it = opts->groundnodetmp2.begin(); it != opts->groundnodetmp2.end(); it++) {
		it->second[1] = it->second[0];
		it->second[0] = 0;
	}

	opts->sum = 0;
}

static void stop_processing(libtrace_t *trace, libtrace_thread_t *thread, 
    void *global, void *tls) {

    struct globalopts *opts = (struct globalopts *)global;
    struct threadlocal *tl = (struct threadlocal *)tls;

    expire_ident_flows(trace, thread, opts, tl->flowmanager, 0, true);

    if(EndTime>StartTime){
	std::cout << "Epoch " << opts->numfile+1 << " (";
	std::cout << std::setw(2) << std::right << std::setfill('0') << (int)StartTime%86400/3600+8 << ":" << std::setw(2) << std::right << std::setfill('0') << (int)StartTime%3600/60 << ":";
	std::cout << std::setw(2) << std::right << std::setfill('0') << (int)StartTime%60 << " - ";
	std::cout << std::setw(2) << std::right << std::setfill('0') << (int)EndTime%86400/3600+8 << ":" << std::setw(2) << std::right << std::setfill('0') << (int)EndTime%3600/60 << ":";
	std::cout << std::setw(2) << std::right << std::setfill('0') << (int)EndTime%60 << "):" << std::left << std::setfill(' ') << std::endl;
	Query(opts);
	StartTime = EndTime;
    }
    delete(tl->flowmanager);
    free(tl);
}

static void per_result(libtrace_t *trace, libtrace_thread_t *sender, 
    void *global, void *tls, libtrace_result_t *result) {

	struct globalopts *opts = (struct globalopts *)global;
	char *resultstr;

	if (result->type != RESULT_USER)
		return;

	resultstr = (char *)result->value.ptr;
	//printf("%s", resultstr);

	std::string srcdst = resultstr;
	uint16_t srcport, dstport;
	int srcip, dstip;
	uint8_t protocol;
	uint64_t bytes_sum;
	
	int pos1 = srcdst.find(" ");
	int pos2 = srcdst.find(" ",pos1+1);
	int pos3 = srcdst.find(" ",pos2+1);
	int pos4 = srcdst.find(" ",pos3+1);
	int pos5 = srcdst.find(" ",pos4+1);
	int pos6 = srcdst.find(" ",pos5+1);
	int pos7 = srcdst.find("\n",pos6+1);

	std::string name_tmp = srcdst.substr(0,pos1);
	char *name = const_cast<char*>(name_tmp.c_str());
	std::string dst = srcdst.substr(pos1+1,pos2-pos1-1);
        std::string src = srcdst.substr(pos2+1,pos3-pos2-1);
	std::string dstp = srcdst.substr(pos3+1,pos4-pos3-1);
        std::string srcp = srcdst.substr(pos4+1,pos5-pos4-1);
	std::string proto = srcdst.substr(pos5+1,pos6-pos5-1);
	std::string bytessum = srcdst.substr(pos6+1,pos7-pos6-1);
	
	lpi_protocol_t appproto = lpi_get_protocol_by_name(name);
        dstip = IPToValue(dst.c_str());
        srcip = IPToValue(src.c_str());
	dstport = atoi(dstp.c_str());
	srcport = atoi(srcp.c_str());
	protocol = atoi(proto.c_str());
	bytes_sum = atoi(bytessum.c_str());

	tuple_t t;
	t.key.AppProto = appproto;
	t.key.src_ip = srcip;
	t.key.dst_ip = dstip;
	t.key.src_port = srcport;
	t.key.dst_port = dstport;
	t.key.protocol = protocol;
	t.bytes_sum = bytes_sum;
	
	Updating_Sketch(opts,t);
	
	free(resultstr);

	if((EndTime-StartTime) >= 360){
		std::cout << "Epoch " << opts->numfile+1 << " (";
		std::cout << std::setw(2) << std::right << std::setfill('0') << (int)StartTime%86400/3600+8 << ":" << std::setw(2) << std::right << std::setfill('0') << (int)StartTime%3600/60 << ":";
		std::cout << std::setw(2) << std::right << std::setfill('0') << (int)StartTime%60 << "-";
		std::cout << std::setw(2) << std::right << std::setfill('0') << (int)EndTime%86400/3600+8 << ":" << std::setw(2) << std::right << std::setfill('0') << (int)EndTime%3600/60 << ":";
		std::cout << std::setw(2) << std::right << std::setfill('0') << (int)EndTime%60 << "):" << std::left << std::setfill(' ') << std::endl;
		Query(opts);
		StartTime = EndTime;
	}
}

static libtrace_packet_t *per_packet(libtrace_t *trace, 
    libtrace_thread_t *thread, void *global, void *tls, 
    libtrace_packet_t *packet) {

    Flow *f;
    IdentFlow *ident = NULL;
    uint8_t dir = 255;
    bool is_new = false;

    libtrace_tcp_t *tcp = NULL;
    void *l3;
    double ts;

    uint16_t l3_type = 0;
    struct globalopts *opts = (struct globalopts *)global;
    struct threadlocal *tl = (struct threadlocal *)tls;

    /* Libflowmanager only deals with IP traffic, so ignore anything 
     * that does not have an IP header */
    l3 = trace_get_layer3(packet, &l3_type, NULL);
    if (l3_type != TRACE_ETHERTYPE_IP && l3_type != TRACE_ETHERTYPE_IPV6) 
        return packet;
    if (l3 == NULL)
        return packet;

    /* Expire all suitably idle flows */
    ts = trace_get_seconds(packet);
    //epoch
    if(EndTime - StartTime > 3600){
	StartTime = ts;
    }
    EndTime = ts;
    expire_ident_flows(trace, thread, opts, tl->flowmanager, ts, false);

    /* Determine packet direction */
    if (opts->dir_method == DIR_METHOD_TRACE) {
        dir = trace_get_direction(packet);
    }
    if (opts->dir_method == DIR_METHOD_MAC) {
	dir = mac_get_direction(packet, opts->mac_bytes);
    }
    if (opts->dir_method == DIR_METHOD_PORT) {
	dir = port_get_direction(packet);
    }

    if (dir != 0 && dir != 1)
        return packet;

    /* Match the packet to a Flow - this will create a new flow if
     * there is no matching flow already in the Flow map and set the
     * is_new flag to true. */
    f = tl->flowmanager->matchPacketToFlow(packet, dir, &is_new);

    /* Libflowmanager did not like something about that packet - best to
     * just ignore it and carry on */
    if (f == NULL) {
        return packet;
    }

    tcp = trace_get_tcp(packet);
    /* If the returned flow is new, you will probably want to allocate and
     * initialise any custom data that you intend to track for the flow */
    if (is_new) {
        init_ident_flow(f, dir, ts);
        ident = (IdentFlow *)f->extension;
    } else {
        ident = (IdentFlow *)f->extension;
	if (tcp && tcp->syn && !tcp->ack)
            ident->init_dir = dir;
        if (ident->end_ts < ts)
            ident->end_ts = ts;
    }

    uint32_t psize = trace_get_payload_length(packet);
    /* Update our own byte and packet counters for reporting purposes */
    if (dir == 0) {
	ident->out_pkts += 1;
	//ident->out_bytes += trace_get_payload_length(packet);
	ident->out_bytes += psize;
    }
    else {
	//ident->in_bytes += trace_get_payload_length(packet);
	ident->in_bytes += psize;
	ident->in_pkts += 1;
    }


    /* Pass the packet into libprotoident so it can extract any info
     * it needs from this packet */
    lpi_update_data(packet, &ident->lpi, dir);

    //=======================================================================
    if(ident->classified) {
	char *result = NULL;
	libtrace_generic_t gen;
	uint64_t bytes_sum = psize;
	result = display_packet(f, ident, opts, bytes_sum);
	if (result) {
	    gen.ptr = result;
	    trace_publish_result(trace, thread, ident->end_ts, gen, RESULT_USER);
	}
    }
    else if(ident->out_pkts + ident->in_pkts > 11) {
	lpi_module_t *proto;
	proto = lpi_guess_protocol(&ident->lpi);
	if(proto == NULL || proto->protocol == LPI_PROTO_INVALID || proto->protocol == LPI_PROTO_NO_PAYLOAD || proto->protocol == LPI_PROTO_NO_FIRSTPKT || proto->protocol == LPI_PROTO_ICMP || proto->protocol == LPI_PROTO_UNKNOWN || proto->protocol == LPI_PROTO_UDP || proto->protocol == LPI_PROTO_UNSUPPORTED) {

	}
	else {
	    strcpy(ident->proto_name, proto->name);
	    ident->classified = true;
	    char *result = NULL;
	    libtrace_generic_t gen;
	    uint64_t bytes_sum = ident->out_bytes + ident->in_bytes;
	    result = display_packet(f, ident, opts, bytes_sum);
	    if (result) {
		gen.ptr = result;
		trace_publish_result(trace, thread, ident->end_ts, gen, RESULT_USER);
	    }
	}
    }
    //=======================================================================

    assert(f);
    /* Tell libflowmanager to update the expiry time for this flow */
    tl->flowmanager->updateFlowExpiry(f, packet, dir, ts);

    return packet;
}

static void cleanup_signal(int sig) {
	(void)sig;
    if (!done) {
        trace_pstop(currenttrace);
        done = 1;
    }
}

static void usage(char *prog) {
	printf("Usage details for %s\n\n", prog);
	printf("%s [-l <mac>] [-T] [-b] [-d <dir>] [-f <filter>] [-R] [-H] [-t <threads>] [-B buflen] inputURI [inputURI ...]\n\n", prog);
	printf("Options:\n");
	printf("  -l <mac>	Determine direction based on <mac> representing the 'inside' \n			portion of the network\n");
	printf("  -T		Use trace direction tags to determine direction\n");
	printf("  -b		Ignore flows that do not send data in both directions \n");
	printf("  -d <dir>	Ignore flows where the initial packet does not match the given \n   		direction\n");
	printf("  -f <filter>	Ignore flows that do not match the given BPF filter\n");
	printf("  -R 		Ignore flows involving private RFC 1918 address space\n");
	printf("  -H		Ignore flows that do not meet the criteria for an SPNAT hole\n");
	printf("  -t <threads>  Share the workload over the given number of threads\n");
	printf("  -B <buflen>   Buffer results until there are <buflen> results waiting\n");
	exit(0);
}

int main(int argc, char* argv[]) {
	lpi_init_library();

	struct globalopts opts;
	int opt;
	char *filterstring = NULL;
	int dir;
	int threads = 1;
	int bufferresults = 10;
	opts.dir_method = DIR_METHOD_PORT;
	opts.only_dir0 = false;
	opts.only_dir1 = false;
	opts.require_both = false;
	opts.nat_hole = false;
	opts.ignore_rfc1918 = false;
	opts.local_mac = NULL;
	while ((opt = getopt(argc, argv, "l:bB:Hd:f:RhTt:")) != EOF) {
		switch (opt) {
			case 'l':
				opts.local_mac = optarg;
				opts.dir_method = DIR_METHOD_MAC;
				break;
			case 'b':
				opts.require_both = true;
				break;
			case 'B':
				bufferresults = atoi(optarg);
				if (bufferresults <= 0)
					bufferresults = 1;
				break;
			case 'd':
				dir = atoi(optarg);
				if (dir == 0)
					opts.only_dir0 = true;
				if (dir == 1)
					opts.only_dir1 = true;
				break;
			case 'f':
				filterstring = optarg;
				break;
			case 'R':
				opts.ignore_rfc1918 = true;
				break;
			case 'H':
				opts.nat_hole = true;
				break;
			case 'T':
				opts.dir_method = DIR_METHOD_TRACE;
				break;
			case 't':
				threads = atoi(optarg);
				if (threads <= 0)
					threads = 1;
				break;
			case 'h':
			default:
				usage(argv[0]);
		}
	}
	
	
	//Sketch Parameters
	//parameters
	int mv_length = 30;
        int mv_width = 5;
	int mv_depth = 3;
	std::cout << "AppSketch: " << std::endl;
	std::cout << "length * width * depth: " << mv_length << " * " << mv_width << " * " << mv_depth << std::endl;

	int mv_length_t = 52;
        int mv_width_t = 9;
	int mv_depth_t = 3;
	std::cout << "Improved TCM: " << std::endl;
	std::cout << "length * width * depth: " << mv_length_t << " * " << mv_width_t << " * " << mv_depth_t << std::endl;

	opts.numfile = 0;

	// Time cost
	opts.avrupdatetime=0;
	opts.avrquerytime9=0;
	opts.avria_TopkApp=0;  //intersection accuracy
	opts.avrjs_TopkApp=0;  //jaccard similarity
	opts.avrndcg_TopkApp=0;  //normalized discounted cumulative gain
	opts.avrreerTopkApp=0;  //relative error
	opts.avrquerytime10=0;
	opts.avria_TopkUser=0;
	opts.avrjs_TopkUser=0;
	opts.avrndcg_TopkUser=0;
	opts.avrreerTopkUser=0;

	opts.avrupdatetime_t=0;
	opts.avrquerytime9_t=0;
	opts.avria_TopkApp_t=0;
	opts.avrjs_TopkApp_t=0;
	opts.avrndcg_TopkApp_t=0;
	opts.avrreerTopkApp_t=0;
	opts.avrquerytime10_t=0;
	opts.avria_TopkUser_t=0;
	opts.avrjs_TopkUser_t=0;
	opts.avrndcg_TopkUser_t=0;
	opts.avrreerTopkUser_t=0;
	
	opts.avrreerTopkApp2=0;
	opts.avrreerTopkApp_t2=0;
	opts.avrreerTopkUser2=0;
	opts.avrreerTopkUser_t2=0;

	//Reset gounrdtmp;
	for (auto it = opts.groundedgetmp.begin(); it != opts.groundedgetmp.end(); it++) {
		it->second[1] = it->second[0];
		it->second[0] = 0;
	}

	for (auto it = opts.groundnodetmp.begin(); it != opts.groundnodetmp.end(); it++) {
		it->second[1] = it->second[0];
		it->second[0] = 0;
	}

	for (auto it = opts.groundnodetmp2.begin(); it != opts.groundnodetmp2.end(); it++) {
		it->second[1] = it->second[0];
		it->second[0] = 0;
	}

	opts.K = 10;
	opts.K2 = 10;

	//AppSketch and Improved TCM
	opts.heavychangerdm = new HeavyChanger<DMatrix>(mv_depth, mv_length, mv_width, LGN, opts.K, opts.K2);
	opts.heavychangersdm = new HeavyChanger<SDMatrix>(mv_depth_t, mv_length_t, mv_width_t, LGN, opts.K, opts.K2);

	opts.sum = 0;

	//LPI start
	char input_file[100];
	if(argc>1){
		strcpy(input_file, argv[1]);
	}
	else{
		std::string file;
		std::ifstream tracefiles("iptrace_URI.txt");
		if(!tracefiles.is_open()) {
			std::cout << "Error opening file" << std::endl;
			return -1;
		}
		if(getline(tracefiles, file)){
			strcpy(input_file, file.c_str());
		}
		else{
			strcpy(input_file, "./traces/ipv4.202011262000.pcap");
		}
	}
	std::cout << "[Dataset]: " << input_file << std::endl;
	
	libtrace_filter_t *filter = NULL;

	libtrace_callback_set_t *processing, *reporter;

	processing = trace_create_callback_set();
	trace_set_starting_cb(processing, start_processing);
	trace_set_stopping_cb(processing, stop_processing);
	trace_set_packet_cb(processing, per_packet);

	reporter = trace_create_callback_set();
	trace_set_starting_cb(reporter, start_reporter);
	trace_set_stopping_cb(reporter, stop_reporter);
	trace_set_result_cb(reporter, per_result);

	if (filterstring != NULL) {
		filter = trace_create_filter(filterstring);
	}

	if (opts.local_mac != NULL) {
		if (convert_mac_string(opts.local_mac, opts.mac_bytes) < 0) {
			fprintf(stderr, "Invalid MAC: %s\n", opts.local_mac);
			return 1;
		}
	}

	struct sigaction sigact;

	sigact.sa_handler = cleanup_signal;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = SA_RESTART;

	sigaction(SIGINT, &sigact, NULL);
	sigaction(SIGTERM, &sigact, NULL);

	signal(SIGINT,&cleanup_signal);
	signal(SIGTERM,&cleanup_signal);

	if (done) {
		//break;
		fprintf(stderr, "%s\n", input_file);
	}

	/* Bog-standard libtrace stuff for reading trace files */
	currenttrace = trace_create(input_file);

	if (!currenttrace) {
		perror("Creating libtrace trace");
		return -1;
	}

	if (trace_is_err(currenttrace)) {
		trace_perror(currenttrace, "Opening trace file");
		trace_destroy(currenttrace);
		//continue;
	}

	if (filter && trace_config(currenttrace, TRACE_OPTION_FILTER, filter) == -1) {
		trace_perror(currenttrace, "Configuring filter");
		trace_destroy(currenttrace);
		return -1;
	}

	trace_set_perpkt_threads(currenttrace, threads);
	trace_set_reporter_thold(currenttrace, bufferresults);

	trace_set_combiner(currenttrace, &combiner_unordered, (libtrace_generic_t){0});  //combiner_ordered

	trace_set_hasher(currenttrace, HASHER_BIDIRECTIONAL, NULL, NULL);

	if (trace_pstart(currenttrace, &opts, processing, reporter) == -1) {
		trace_perror(currenttrace, "Starting trace");
		trace_destroy(currenttrace);
		//continue;
	}

	trace_join(currenttrace);
	trace_destroy(currenttrace);

	trace_destroy_callback_set(processing);
	trace_destroy_callback_set(reporter);
	
	//Delete
	for (auto it = opts.groundedgetmp.begin(); it != opts.groundedgetmp.end(); it++) {
		delete [] it->second;
	}
	delete opts.heavychangerdm;
	delete opts.heavychangersdm;

	std::cout << "-----------------------------------------------   Summary    -------------------------------------------------------" << std::endl;

	std::cout << "AppSketch: " << std::endl;
	std::cout << std::setw(30)<< std::left << "IA. Top-k Apps" << std::setw(30) << std::left << "JS. Top-k Apps" << std::endl;
	std::cout << std::setw(30)<< std::left << opts.avria_TopkApp/opts.numfile << std::setw(30) << std::left << opts.avrjs_TopkApp/opts.numfile << std::endl;
	std::cout << std::setw(30)<< std::left << "NDCG. Top-k Apps" << std::setw(30) << std::left << "RError. Top-k Apps" << std::endl;
	//std::cout << std::setw(30)<< std::left << opts.avrndcg_TopkApp/opts.numfile << std::setw(30) << std::left << opts.avrreerTopkApp/opts.numfile << std::setw(30) << std::left << opts.avrreerTopkApp2/opts.numfile << std::endl;
	std::cout << std::setw(30)<< std::left << opts.avrndcg_TopkApp/opts.numfile << std::setw(30) << std::left << opts.avrreerTopkApp2/opts.numfile << std::endl;
	std::cout << std::setw(30)<< std::left << "IA. Top-k Users" << std::setw(30) << std::left << "JS. Top-k Users" << std::endl;
	std::cout << std::setw(30)<< std::left << opts.avria_TopkUser/opts.numfile << std::setw(30) << std::left << opts.avrjs_TopkUser/opts.numfile << std::endl;
	std::cout << std::setw(30)<< std::left << "NDCG. Top-k Users" << std::setw(30) << std::left << "RError. Top-k Users" << std::endl;
	//std::cout << std::setw(30)<< std::left << opts.avrndcg_TopkUser/opts.numfile << std::setw(30) << std::left << opts.avrreerTopkUser/opts.numfile << std::setw(30) << std::left << opts.avrreerTopkUser2/opts.numfile << std::endl;
	std::cout << std::setw(30)<< std::left << opts.avrndcg_TopkUser/opts.numfile << std::setw(30) << std::left << opts.avrreerTopkUser2/opts.numfile << std::endl;

	std::cout << std::setw(30) << std::left << "AvrT. Update" << std::setw(30) << std::left << "AvrT. Top-k Apps"
	<< std::setw(30) << std::left << "AvrT. Top-k Users" << std::endl;
	std::cout << std::setw(30) << std::left << opts.avrupdatetime/opts.numfile << std::setw(30) << std::left << opts.avrquerytime9/opts.numfile
	<< std::setw(30) << std::left << opts.avrquerytime10/opts.numfile << std::endl;

	std::cout << "Improved TCM: " << std::endl;
	std::cout << std::setw(30)<< std::left << "IA. Top-k Apps" << std::setw(30) << std::left << "JS. Top-k Apps" << std::endl;
	std::cout << std::setw(30)<< std::left << opts.avria_TopkApp_t/opts.numfile << std::setw(30) << std::left << opts.avrjs_TopkApp_t/opts.numfile << std::endl;
	std::cout << std::setw(30)<< std::left << "NDCG. Top-k Apps" << std::setw(30) << std::left << "RError. Top-k Apps" << std::endl;
	//std::cout << std::setw(30)<< std::left << opts.avrndcg_TopkApp_t/opts.numfile << std::setw(30) << std::left << opts.avrreerTopkApp_t/opts.numfile << std::setw(30) << std::left << opts.avrreerTopkApp_t2/opts.numfile << std::endl;
	std::cout << std::setw(30)<< std::left << opts.avrndcg_TopkApp_t/opts.numfile << std::setw(30) << std::left << opts.avrreerTopkApp_t2/opts.numfile << std::endl;
	std::cout << std::setw(30)<< std::left << "IA. Top-k Users" << std::setw(30) << std::left << "JS. Top-k Users" << std::endl;
	std::cout << std::setw(30)<< std::left << opts.avria_TopkUser_t/opts.numfile << std::setw(30) << std::left << opts.avrjs_TopkUser_t/opts.numfile << std::endl;
	std::cout << std::setw(30)<< std::left << "NDCG. Top-k Users" << std::setw(30) << std::left << "RError. Top-k Users" << std::endl;
	//std::cout << std::setw(30)<< std::left << opts.avrndcg_TopkUser_t/opts.numfile << std::setw(30) << std::left << opts.avrreerTopkUser_t/opts.numfile << std::setw(30) << std::left << opts.avrreerTopkUser_t2/opts.numfile << std::endl;
	std::cout << std::setw(30)<< std::left << opts.avrndcg_TopkUser_t/opts.numfile << std::setw(30) << std::left << opts.avrreerTopkUser_t2/opts.numfile << std::endl;

	std::cout << std::setw(30) << std::left << "AvrT. Update" << std::setw(30) << std::left << "AvrT. Top-k Apps"
	<< std::setw(30) << std::left << "AvrT. Top-k Users" << std::endl;
	std::cout << std::setw(30) << std::left << opts.avrupdatetime_t/opts.numfile << std::setw(30) << std::left << opts.avrquerytime9_t/opts.numfile
	<< std::setw(30) << std::left << opts.avrquerytime10_t/opts.numfile << std::endl;

	lpi_free_library();
}
