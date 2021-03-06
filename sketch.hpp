#ifndef MVKETCH_H
#define MVKETCH_H
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
extern "C"
{
#include "hash.h"
#include "util.h"
}



class DMatrix {

    typedef struct SBUCKET_type {
        //Total sum V(i, j)
        val_tp sum;

        long count;

        //unsigned char key[LGN];
        uint32_t key[2];

    } SBucket;

    struct MV_type {

        //Counter to count total degree
        val_tp sum;
        //Counter table
        SBucket **counts;

        //Outer sketch depth, length and width
        int depth;
        int length;
        int width;

        //# key word bits
        int lgn;

        unsigned long *hash, *scale, *hardner;
    };


    public:
    //DMatrix(int depth, int length, int width, int lgn);
    DMatrix(int depth, int length, int width, int lgn, int K, int K2);

    ~DMatrix();

    void Update(uint32_t * key, val_tp value);

    val_tp Queryedgeweight(uint32_t* key);

    val_tp Querynodeweight(uint32_t* key);

    val_tp Querynodeweight_d(uint32_t* key);

    void QueryTopkApps(int K, std::pair<key_tp, val_tp> *es_topk_apps);
    void QueryTopkUsers(int K2, std::pair<key_tp, val_tp> *es_topk_users);

    val_tp Querynodeweight2(uint32_t* key);

    void Queryheavyhitteredge(val_tp thresh, myvector& results);

    void Queryheavyhitternode(val_tp thresh, myvector& resultsnode);

    bool BFS(uint32_t* key, myvector& path);

    val_tp Change(DMatrix** mv_arr);

    void Queryheavychangeredge(val_tp thresh, myvector& results);

    void Queryheavychangernode(val_tp thresh, myvector& results);

    val_tp Low_estimate(uint32_t* key);

    val_tp Up_estimate(uint32_t* key);

    val_tp Up_estimate2(uint32_t* key);

    val_tp Low_estimatenode(uint32_t* key);

    val_tp GetCount();

    void Reset();

    void Print();

    void FindSuper(mymap& super);

    MV_type mv_;

    private:

    void SetBucket(int row, int column, val_tp sum, long count, uint32_t * key);

    DMatrix::SBucket** GetTable();

    //MV_type mv_;
};

#endif
