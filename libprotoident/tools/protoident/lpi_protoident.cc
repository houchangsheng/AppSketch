#include "lpi_protoident.h"

libtrace_t* currenttrace;
static volatile int done = 0;

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

static void stop_processing(libtrace_t *trace, libtrace_thread_t *thread, 
    void *global, void *tls) {

    struct globalopts *opts = (struct globalopts *)global;
    struct threadlocal *tl = (struct threadlocal *)tls;

    expire_ident_flows(trace, thread, opts, tl->flowmanager, 0, true);
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

    char output_file[100];
    strcpy(output_file, opts->file_url);
    strcat(output_file, "lpiout_");
    strcat(output_file, opts->trace_file);

    std::ofstream lpiout;
    lpiout.open(output_file, std::ios::app);
    lpiout << resultstr;
    lpiout.close();
    free(resultstr);
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

int lpi(struct globalopts opts, char* filterstring, int threads, int bufferresults) {
    
    char input_file[100];
    strcpy(input_file, opts.file_url);
    strcat(input_file, opts.trace_file);
    strcat(input_file, opts.suffix);
    char output_file[100];
    strcpy(output_file, opts.file_url);
    strcat(output_file, "lpiout_");
    strcat(output_file, opts.trace_file);

    std::ofstream lpiout;
    lpiout.open(output_file, std::ios::trunc);
    lpiout.close();
    
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

	//if (lpi_init_library() == -1)
	//	return -1;
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
    //lpi_free_library();

    return 0;
}

