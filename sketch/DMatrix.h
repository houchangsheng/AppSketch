#ifndef DMATRIX_H
#define DMATRIX_H
#include <vector>
#include <unordered_set>
#include <utility>
#include <cstring>
#include <cmath>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <arpa/inet.h>

#include "MultiAbstract.h"

template<typename DATA_TYPE>
class DMatrix : public MultiAbstract<DATA_TYPE>{

    typedef struct SBUCKET_type {
        DATA_TYPE key;
        COUNT_TYPE sum;
        COUNT_TYPE count;
    } SBucket;

    COUNT_TYPE sum;
    SBucket **counts;

    int depth;
    int dim_1;
    int dim_2;

public:
    typedef std::unordered_map<DATA_TYPE, COUNT_TYPE> HashMap;

    DMatrix(uint32_t _MEMORY, double _prop, uint32_t _HASH_NUM = 2, std::string _name = "DMatrix") {
        this->name = _name;
        std::cout << this->name << std::endl;

        uint32_t Memory = _MEMORY / _HASH_NUM;

        uint32_t bucket_num = Memory / sizeof(SBucket);

        dim_1 = sqrt(uint32_t(bucket_num * _prop));
        dim_2 = dim_1 / _prop;
        depth = _HASH_NUM;
        sum = 0;

        counts = new SBucket *[depth];
        for (int i = 0; i < depth; i++) {
            counts[i] = new SBucket [dim_1*dim_2];
            memset(counts[i], 0, sizeof(SBucket)*dim_1*dim_2);
        }
    }

    ~DMatrix() {
        for (int i = 0; i < depth; i++) {
            delete [] counts[i];
        }
        delete [] counts;
    }

    void Insert(const DATA_TYPE& item, const COUNT_TYPE& p_s) {
        sum += p_s;
        int idx_1 = 0;
        int idx_2 = 0;

        DATA_TYPE item_1 = item;
        DATA_TYPE item_2 = item;

        item_1.key[1] = 0;
        item_2.key[0] = 0;

        for (int i = 0; i < depth; i++) {
            idx_1 = hash(item_1, i) % dim_1;
            idx_2 = hash(item_2, i) % dim_2;
            SBucket* sbucket = &counts[i][idx_1*dim_2 + idx_2];
            sbucket->sum += p_s;
            if (sbucket->key.key[0] == 0 && sbucket->key.key[1] == 0) {
                sbucket->key = item;
                sbucket->count = p_s;
            } else if(sbucket->key == item) {
                sbucket->count += p_s;
            } else {
                sbucket->count -= p_s;
                if (sbucket->count < 0) {
                    sbucket->key = item;
                    sbucket->count = -sbucket->count;
                }
            }
        }
    }

    COUNT_TYPE Query(const DATA_TYPE& item) {

        COUNT_TYPE ret = 0;

        int idx_1 = 0;
        int idx_2 = 0;

        DATA_TYPE item_1 = item;
        DATA_TYPE item_2 = item;

        item_1.key[1] = 0;
        item_2.key[0] = 0;

        for (int i = 0; i < depth; i++) {
            idx_1 = hash(item_1, i) % dim_1;
            idx_2 = hash(item_2, i) % dim_2;
            SBucket* sbucket = &counts[i][idx_1*dim_2 + idx_2];
            COUNT_TYPE upest = 0;

            if (sbucket->key == item) {
                upest = (sbucket->sum + sbucket->count)/2;
            }
            else {
                upest = (sbucket->sum - sbucket->count)/2;
            }
            if (i == 0) ret = upest;
            else ret = std::min(ret, upest);
        }
        return ret;
    }

    COUNT_TYPE Query_node(const DATA_TYPE& item) {

        COUNT_TYPE ret = 0;

        int idx_1 = 0;
        DATA_TYPE item_1 = item;
        item_1.key[1] = 0;

        for (int i = 0; i < depth; i++) {
            idx_1 = hash(item_1, i) % dim_1;
            COUNT_TYPE value = 0;
            SBucket* sbucket;
            for (int j = 0; j < dim_2; j++){
                sbucket = &counts[i][idx_1*dim_2 + j];

                DATA_TYPE item_temp = sbucket->key;
                item_temp.key[1] = 0;
                 
                if(item_temp == item_1) {
                    value += (sbucket->sum + sbucket->count)/2;
                }
                else {
                    value += (sbucket->sum - sbucket->count)/2;
                }
            }
            if (i == 0) ret = value;
            else ret = std::min(ret, value);
        }
        return ret;
    }

    COUNT_TYPE Query_node_t(const DATA_TYPE& item) {

        COUNT_TYPE ret = 0;

        int idx_2 = 0;
        DATA_TYPE item_2 = item;
        item_2.key[0] = 0;

        for (int i = 0; i < depth; i++) {
            idx_2 = hash(item_2, i) % dim_2;
            COUNT_TYPE value = 0;
            SBucket* sbucket;
            for (int j = 0; j < dim_1; j++){
                sbucket = &counts[i][j*dim_2 + idx_2];

                DATA_TYPE item_temp = sbucket->key;
                item_temp.key[0] = 0;
                 
                if(item_temp == item_2) {
                    value += (sbucket->sum + sbucket->count)/2;
                }
                else {
                    value += (sbucket->sum - sbucket->count)/2;
                }
            }
            if (i == 0) ret = value;
            else ret = std::min(ret, value);
        }
        return ret;
    }

    HashMap AllQuery(){
        HashMap ret;
        SBucket* sbucket;

        for (int i = 0; i < depth; i++) {
            for (int j = 0; j < dim_1 * dim_2; j++)
            {
                sbucket = &counts[i][j];

                if(ret.find(sbucket->key) == ret.end()) {
                    ret[sbucket->key] = (sbucket->sum + sbucket->count)/2;
                }
                else {
                    ret[sbucket->key] = std::min(ret[sbucket->key], (sbucket->sum + sbucket->count)/2);
                }
            }
        }

        return ret;
    }

    HashMap QueryTopk() {

        HashMap ret;
        SBucket* sbucket;

        for (int i = 0; i < depth; i++) {
            for (int j = 0; j < dim_1 * dim_2; j++)
            {
                sbucket = &counts[i][j];

                DATA_TYPE item_1 = sbucket->key;
                item_1.key[1] = 0;
                ret[item_1] = 0;
            }
        }

        for (auto it = ret.begin(); it != ret.end(); ++it) {
            ret[it->first] = Query_node(it->first);
        }

        return ret;
    }

    HashMap QueryTopk_t() {

        HashMap ret;
        SBucket* sbucket;

        for (int i = 0; i < depth; i++) {
            for (int j = 0; j < dim_1 * dim_2; j++)
            {
                sbucket = &counts[i][j];

                DATA_TYPE item_2 = sbucket->key;
                item_2.key[0] = 0;

                ret[item_2] = 0;
            }
        }

        for (auto it = ret.begin(); it != ret.end(); ++it) {
            ret[it->first] = Query_node_t(it->first);
        }

        return ret;
    }

};

#endif
