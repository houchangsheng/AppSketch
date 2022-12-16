#define __STDC_FORMAT_MACROS

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <inttypes.h>
#include <signal.h>

#include <libtrace.h>
#include <libtrace_parallel.h>
#include <libflowmanager.h>
#include <libprotoident.h>

#include "../tools_common.h"

#include <iostream>
#include <fstream>

enum {
	DIR_METHOD_TRACE,
	DIR_METHOD_MAC,
	DIR_METHOD_PORT
};

struct globalopts {

    int dir_method;
    bool only_dir0;
    bool only_dir1;
    bool require_both;
    bool nat_hole;
    bool ignore_rfc1918;
    char* local_mac;
    uint8_t mac_bytes[6];

    char* file_url;
    char* trace_file;
    char* suffix;
};

int lpi(struct globalopts opts, char* filterstring, int threads, int bufferresults);
