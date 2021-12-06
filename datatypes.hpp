#ifndef DATATYPE_H
#define DATATYPE_H
#include "hash.h"
#include "string.h"
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include "libprotoident.h"
#define LGN 8

typedef struct key_t_s {
    uint32_t key[2];
    key_t_s()
    {
	this->key[0] = 0;
	this->key[1] = 0;
	//memset(this,0,sizeof(key_t_s));
    }
} key_tp;

typedef uint64_t val_tp;

/**
 * Object for hash
 */
typedef struct {
    /// overloaded operation
    long operator() (const key_tp &k) const { return MurmurHash64A(k.key, LGN, 388650253); }
} key_tp_hash;

/**
 * Object for equality
 */
typedef struct {
    /// overloaded operation
    bool operator() (const key_tp &x, const key_tp &y) const {
        return memcmp(x.key, y.key, LGN)==0;
    }
} key_tp_eq;


typedef std::unordered_set<key_tp, key_tp_hash, key_tp_eq> myset;

typedef std::unordered_map<key_tp, val_tp, key_tp_hash, key_tp_eq> mymap;

typedef std::vector<std::pair<key_tp, val_tp> > myvector;


/**
 * IP flow key
 * */
typedef struct __attribute__ ((__packed__)) flow_key {
    lpi_protocol_t AppProto;
    uint32_t src_ip;
    uint32_t dst_ip;
    uint16_t src_port;
    uint16_t dst_port;
    uint8_t protocol;
} flow_key_t;

/**
 *input data
 * */
typedef struct __attribute__ ((__packed__)) Tuple {
    flow_key_t key;
    uint64_t bytes_sum;
} tuple_t;

#endif
