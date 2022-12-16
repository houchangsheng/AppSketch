#ifndef _SC_H
#define _SC_H

#include "MultiAbstract.h"
#include <cstring>
#include <algorithm>
#include <immintrin.h>
#include <stdexcept>
/*#ifdef UNIX
#include <x86intrin.h>
#else
#include <intrin.h>
#endif*/

#define MAX_HASH_NUM 4

template<typename DATA_TYPE>
class ColdFilter : public MultiAbstract<DATA_TYPE>
{
    int d1;
    int m1_in_bytes;
    int d2;
    int m2_in_bytes;

    int w1;
    int w_word;
    int w2;

    uint64_t* L1;
    uint16_t* L2;

    MultiAbstract<DATA_TYPE> * multi_a;
    COUNT_TYPE threshold;
public:

    typedef std::unordered_map<DATA_TYPE, COUNT_TYPE> HashMap;

    ColdFilter(uint32_t _MEMORY, MultiAbstract<DATA_TYPE> * _multi_a = NULL, COUNT_TYPE _threshold = 240, int l1_ratio = 65, uint32_t _HASH_NUM = 3, std::string _name = "ColdFilter")
    {
        this->name = _name;
        std::cout << this->name << std::endl;

        //d1 = 3;
        d1 = _HASH_NUM;
        m1_in_bytes = int(_MEMORY * l1_ratio / 100.0);
        //d2 = 3;
        d2 = _HASH_NUM;
        m2_in_bytes = int(_MEMORY * (100 - l1_ratio) / 100.0);

        w1 = m1_in_bytes * 8 / 4;
        w_word = m1_in_bytes * 8 / 4 / 16;
        w2 = m2_in_bytes * 8 / 16;

        L1 = new uint64_t[m1_in_bytes * 8 / 4 / 16]; // Layer 1 is organized as word, one word contains 16 counter, one counter consist of 4 bit
        L2 = new uint16_t[m2_in_bytes * 8 / 16]; // Layer 2 is organized as counter, one counter consist of 16 bit

        memset(L1, 0, sizeof(uint64_t) * w_word);
        memset(L2, 0, sizeof(short int) * w2);

        multi_a = _multi_a;
        threshold = _threshold;
    }

    ~ColdFilter()
    {
    }

    void print_basic_info()
    {
        printf("Cold Filter\n");
        printf("\tL1: %d counters, %.4lf MB occupies\n", w1, w1 * 0.5 / 1024 / 1024);
        printf("\tL2: %d counters, %.4lf MB occupies\n", w2, w2 * 2.0 / 1024 / 1024);
    }

    //periodical refreshing for continuous top-k;
    void init_array_period()
    {
        for (int i = 0; i < w_word; i++) {
            uint64_t temp = L1[i];

            temp = (temp & (0xF)) == 0xF ? temp : (temp & 0xFFFFFFFFFFFFFFF0);
            temp = (temp & (0xF0)) == 0xF0 ? temp : (temp & 0xFFFFFFFFFFFFFF0F);
            temp = (temp & (0xF00)) == 0xF00 ? temp : (temp & 0xFFFFFFFFFFFFF0FF);
            temp = (temp & (0xF000)) == 0xF000 ? temp : (temp & 0xFFFFFFFFFFFF0FFF);

            temp = (temp & (0xF0000)) == 0xF0000 ? temp : (temp & 0xFFFFFFFFFFF0FFFF);
            temp = (temp & (0xF00000)) == 0xF00000 ? temp : (temp & 0xFFFFFFFFFF0FFFFF);
            temp = (temp & (0xF000000)) == 0xF000000 ? temp : (temp & 0xFFFFFFFFF0FFFFFF);
            temp = (temp & (0xF0000000)) == 0xF0000000 ? temp : (temp & 0xFFFFFFFF0FFFFFFF);

            temp = (temp & (0xF00000000)) == 0xF00000000 ? temp : (temp & 0xFFFFFFF0FFFFFFFF);
            temp = (temp & (0xF000000000)) == 0xF000000000 ? temp : (temp & 0xFFFFFF0FFFFFFFFF);
            temp = (temp & (0xF0000000000)) == 0xF0000000000 ? temp : (temp & 0xFFFFF0FFFFFFFFFF);
            temp = (temp & (0xF00000000000)) == 0xF00000000000 ? temp : (temp & 0xFFFF0FFFFFFFFFFF);

            temp = (temp & (0xF000000000000)) == 0xF000000000000 ? temp : (temp & 0xFFF0FFFFFFFFFFFF);
            temp = (temp & (0xF0000000000000)) == 0xF0000000000000 ? temp : (temp & 0xFF0FFFFFFFFFFFFF);
            temp = (temp & (0xF00000000000000)) == 0xF00000000000000 ? temp : (temp & 0xF0FFFFFFFFFFFFFF);
            temp = (temp & (0xF000000000000000)) == 0xF000000000000000 ? temp : (temp & 0x0FFFFFFFFFFFFFFF);

            L1[i] = temp;
        }

        for (int i = 0; i < w2; i++) {
            short int temp = L2[i];
            L2[i] = (temp == threshold) ? temp : 0;
        }
    }

    void init_multi_a(MultiAbstract<DATA_TYPE> * _multi_a)
    {
        multi_a = _multi_a;
    }

    void Insert(const DATA_TYPE& item, const COUNT_TYPE& p_s)
    {
        COUNT_TYPE p_s_temp = p_s;
        int v1 = INT_MAX;

        int value[MAX_HASH_NUM];
        int index[MAX_HASH_NUM];
        int offset[MAX_HASH_NUM];

        memset(value, 0, sizeof(int) * MAX_HASH_NUM);
        memset(index, 0, sizeof(int) * MAX_HASH_NUM);
        memset(offset, 0, sizeof(int) * MAX_HASH_NUM);

        //uint64_t hash_value = hash(item, 4);
        uint32_t hash_value = hash(item, 4);
        int word_index = hash_value % w_word;
        hash_value >>= 16;

        uint64_t temp = L1[word_index];
        for (int i = 0; i < d1; i++) {
            offset[i] = (hash_value & 0xF);
            value[i] = (temp >> (offset[i] << 2)) & 0xF;
            v1 = std::min(v1, value[i]);

            hash_value >>= 4;
        }

        int temp2 = v1 + p_s_temp;
        if (temp2 <= 15) { // maybe optimized use SIMD?
            for (int i = 0; i < d1; i++) {
                int temp3 = ((temp >> (offset[i] << 2)) & 0xF);
                if (temp3 < temp2) {
                    temp += ((uint64_t)(temp2 - temp3) << (offset[i] << 2));
                }
            }
            L1[word_index] = temp;
            return;
        }

        for (int i = 0; i < d1; i++) {
            temp |= ((uint64_t)0xF << (offset[i] << 2));
        }
        L1[word_index] = temp;

        int delta1 = 15 - v1;
        p_s_temp -= delta1;

        int v2 = INT_MAX;
        for (int i = 0; i < d2; i++) {
            index[i] = hash(item, i) % w2;
            value[i] = L2[index[i]];
            v2 = std::min(value[i], v2);
        }

        temp2 = v2 + p_s_temp;
        if (temp2 <= threshold) {
            for (int i = 0; i < d2; i++) {
                L2[index[i]] = (L2[index[i]] > temp2) ? L2[index[i]] : temp2;
            }
            return;
        }

        for (int i = 0; i < d2; i++) {
            L2[index[i]] = threshold;
        }

        int delta2 = threshold - v2;
        p_s_temp -= delta2;

        multi_a->Insert(item, p_s_temp);
    }

    COUNT_TYPE Query(const DATA_TYPE& item)
    {
        COUNT_TYPE temp = multi_a->Query_t(item);
        if(temp == 0)
        {
            return query_t(item);
        }
        else
        {
            return temp;
        }
    }

    int query_t(const DATA_TYPE& item)
    {
        int v1 = INT_MAX;

        uint32_t hash_value = hash(item, 4);
        int word_index = hash_value % w_word;
        hash_value >>= 16;

        uint64_t temp = L1[word_index];
        for (int i = 0; i < d1; i++) {
            int of, val;
            of = (hash_value & 0xF);
            val = (temp >> (of << 2)) & 0xF;
            v1 = std::min(val, v1);
            hash_value >>= 4;
        }

        if (v1 != 15)
            return v1;

        int v2 = INT_MAX;
        for (int i = 0; i < d2; i++) {
            int index = hash(item, i) % w2;
            int value = L2[index];
            v2 = std::min(value, v2);
        }

        return v1 + v2;
    }

    HashMap AllQuery(){
        return multi_a->AllQuery();
    }
};

#endif//_SC_H
