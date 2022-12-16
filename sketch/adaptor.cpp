#include "adaptor.h"

Adaptor::Adaptor(std::string filename, uint64_t buffersize) {
    data = (adaptor_t*)calloc(1, sizeof(adaptor_t));
    data->databuffer = (unsigned char*)calloc(buffersize, sizeof(unsigned char));
    data->ptr = data->databuffer;
    data->cnt = 0;
    data->cur = 0;
    //Read lpi_out file
    std::ifstream infile;
    infile.open(filename);
    std::string srcdst;
    unsigned char* p = data->databuffer;
    while (getline(infile, srcdst))
    {
	uint16_t srcport, dstport;
	int srcip, dstip;
	uint8_t protocol;
	COUNT_TYPE bytes_sum;
	
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
	
	memcpy(p, &appproto, sizeof(lpi_protocol_t));
        memcpy(p+sizeof(lpi_protocol_t), &srcip, sizeof(uint32_t));
	memcpy(p+sizeof(lpi_protocol_t)+sizeof(uint32_t), &dstip, sizeof(uint32_t));
        memcpy(p+sizeof(lpi_protocol_t)+sizeof(uint32_t)*2, &srcport, sizeof(uint16_t));
	memcpy(p+sizeof(lpi_protocol_t)+sizeof(uint16_t)*5, &dstport, sizeof(uint16_t));
	memcpy(p+sizeof(lpi_protocol_t)+sizeof(uint32_t)*3, &protocol, sizeof(uint8_t));
	memcpy(p+sizeof(lpi_protocol_t)+sizeof(uint8_t)*13, &bytes_sum, sizeof(COUNT_TYPE));
	p += sizeof(lpi_protocol_t)+sizeof(uint8_t)*21;
	data->cnt++;
    }

    infile.close();
}

Adaptor::~Adaptor() {
    free(data->databuffer);
    free(data);
}

int Adaptor::GetNext(tuple_t* t) {
    if (data->cur >= data->cnt) {
        return -1;
    }
    t->key.AppProto = *((lpi_protocol_t*)data->ptr);
    t->key.src_ip = *((uint32_t*)(data->ptr+sizeof(lpi_protocol_t)));
    t->key.dst_ip = *((uint32_t*)(data->ptr+sizeof(lpi_protocol_t)+4));
    t->key.src_port = *((uint16_t*)(data->ptr+sizeof(lpi_protocol_t)+8));
    t->key.dst_port = *((uint16_t*)(data->ptr+sizeof(lpi_protocol_t)+10));
    t->key.protocol = *((uint8_t*)(data->ptr+sizeof(lpi_protocol_t)+12));
    t->bytes_sum = *((COUNT_TYPE*)(data->ptr+sizeof(lpi_protocol_t)+13));
    data->cur++;
    data->ptr += sizeof(lpi_protocol_t)+21;
    return 1;
}

void Adaptor::Reset() {
    data->cur = 0;
    data->ptr = data->databuffer;
}

uint64_t Adaptor::GetDataSize() {
    return data->cnt;
}
