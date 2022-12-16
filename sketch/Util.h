#ifndef UTIL_H
#define UTIL_H

#include <x86intrin.h>

#include <vector>
#include <chrono>
#include <algorithm>
#include <functional>

#include "hash.h"
#include "libprotoident.h"
#include "lpi_protoident.h"

typedef int64_t COUNT_TYPE;

typedef struct __attribute__ ((__packed__)) flow_key {
    lpi_protocol_t AppProto;
    uint32_t src_ip;
    uint32_t dst_ip;
    uint16_t src_port;
    uint16_t dst_port;
    uint8_t protocol;
} flow_key_t;

typedef struct __attribute__ ((__packed__)) Tuple {
    flow_key_t key;
    COUNT_TYPE bytes_sum;
} tuple_t;

typedef struct full_key_tuple {
    uint32_t key[2];
} FullKey;

inline bool operator == (const FullKey& a, const FullKey& b){
    return memcmp(a.key, b.key, sizeof(FullKey)) == 0;
}

namespace std{
    template<>
    struct hash<FullKey>{
        size_t operator()(const FullKey& item) const noexcept
        {
            return Hash::BOBHash32((uint8_t*)&item, sizeof(FullKey), 0);
        }
    };
}

typedef std::chrono::high_resolution_clock::time_point TP;

inline TP now(){
    return std::chrono::high_resolution_clock::now();
}

inline double durationms(TP finish, TP start){
    return std::chrono::duration_cast<std::chrono::duration<double,std::ratio<1,1000000>>>(finish - start).count();
}

template<typename T>
T Median(std::vector<T> vec, uint32_t len){
    std::sort(vec.begin(), vec.end());
    return (len & 1) ? vec[len >> 1] : (vec[len >> 1] + vec[(len >> 1) - 1]) / 2.0;
}

#endif
