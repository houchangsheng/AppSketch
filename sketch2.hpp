#ifndef SDMatrix_H
#define SDMatrix_H
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
#include "datatypes.hpp"
#include "BitMap.hpp"
extern "C"
{
#include "hash.h"
#include "util.h"
}



class SDMatrix {

    typedef struct SBUCKET_type {
        val_tp sum;
    } SBucket;

    typedef struct Topk_List_type {
	val_tp sum;
        uint32_t key[2];
	//long count;
    } TkList;

    /*typedef struct Topk_List_type {
	BitMap* node_bit;
	val_tp sum;
        uint32_t key[2];
	long count;
    } TkList;*/

    struct SDMatrix_type {

        //Counter to count total degree
        val_tp total_sum;
        //Counter table
        SBucket **counts;

        //Outer sketch depth, length and width
        int depth;
        int length;
        int width;

	TkList *tka;
	TkList *tku;

	BitMap *bm_a;
	BitMap *bm_u;

	val_tp topk_threshold;

	int K_App;
        int K_User;

        //key word bits
        int lgn;

        unsigned long *hash, *scale, *hardner;
    };


    public:
    SDMatrix(int depth, int length, int width, int lgn, int K, int K2);

    ~SDMatrix();

    void Update(uint32_t * key, val_tp value);

    val_tp Queryedgeweight(uint32_t* key);

    val_tp Querynodeweight(uint32_t* key);

    val_tp Querynodeweight_d(uint32_t* key);

    void QueryTopkApps(int K, std::pair<key_tp, val_tp> *es_topk_apps);

    void QueryTopkUsers(int K2, std::pair<key_tp, val_tp> *es_topk_users);

    val_tp Change(SDMatrix** sdm_arr);

    val_tp GetCount();

    void Reset();

    private:

    SDMatrix::SBucket** GetTable();

    SDMatrix_type sdm_;
};

#endif
