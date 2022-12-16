#ifndef HHBENCH_H
#define HHBENCH_H

#include <vector>
#include <fstream>

#include "AppSketch.h"
#include "CocoWaving.h"
#include "USS.h"
#include "WavingSketch.h"
#include "HeavyGuardian.h"
#include "SpaceSaving.h"
#include "ColdFilter.h"
#include "DMatrix.h"
#include "adaptor.h"

std::string alg_name[10] = {"AppSketch", "USS", "WavingSketch", "WavingSketch_Single", "HeavyGuardian", "HeavyGuardian_Single", "ColdFilter", "ColdFilter_Single", "DMatrix", "CocoWaving"};

uint32_t epoch;

uint32_t KKK[3];
double ia_K[10][3];
double js_K[10][3];
double ndcg_K[10][3];
double aae_K[10][3];
double are_K[10][3];

COUNT_TYPE TH;
double Recall_TH[10][3];
double Precision_TH[10][3];
double aae_TH[10][3];
double are_TH[10][3];

double insert_t[10];
double query_t[10];

void add_epoch() {
    epoch += 1;
}

void print_KKK() {
    for(int j = 0; j < 3; j++) {
        std::cout << std::setw(10) << std::left << "key-type: " << std::setw(1) << std::left << j+1 << std::setw(5) << std::left << "  K: " << std::setw(5) << std::left << KKK[j] << std::endl;
        for(int i = 0; i < 10; i++) {
            std::cout << std::setw(21) << std::left << alg_name[i] << ":" << std::setw(6) << std::left << "  ia: " << std::setw(10) << std::left << ia_K[i][j] / epoch << std::setw(6) << std::left << "  js: " << std::setw(10) << std::left << js_K[i][j] / epoch << std::setw(8) << std::left << "  ndcg: " << std::setw(10) << std::left << ndcg_K[i][j] / epoch << std::setw(7) << std::left << "  aae: " << std::setw(12) << std::left << aae_K[i][j] / epoch << std::setw(7) << std::left << "  are: " << std::setw(12) << std::left << are_K[i][j] / epoch << std::endl;
        }
        std::cout << std::endl;
    }
}

void print_TH() {
    for(int j = 0; j < 3; j++) {
        std::cout << std::setw(10) << std::left << "key-type: " << std::setw(1) << std::left << j+1 << std::setw(6) << std::left << "  TH: " << std::setw(5) << std::left << TH << std::endl;
        for(int i = 0; i < 10; i++) {
            std::cout << std::setw(21) << std::left << alg_name[i] << ":" << std::setw(10) << std::left << "  Recall: " << std::setw(10) << std::left << Recall_TH[i][j] / epoch << std::setw(13) << std::left << "  Precision: " << std::setw(10) << std::left << Precision_TH[i][j] / epoch << std::setw(7) << std::left << "  aae: " << std::setw(12) << std::left << aae_TH[i][j] / epoch << std::setw(7) << std::left << "  are: " << std::setw(12) << std::left << are_TH[i][j] / epoch << std::endl;
        }
        std::cout << std::endl;
    }
}

void print_Time() {
    for(int i = 0; i < 10; i++) {
        std::cout << std::setw(21) << std::left << alg_name[i] << ":" << std::setw(15) << std::left << "  insert time: " << std::setw(12) << std::left << insert_t[i] / epoch << std::setw(14) << std::left << "  query time: " << std::setw(12) << std::left << query_t[i] / epoch << std::endl;
    }
    std::cout << std::endl;
}

void epoch_clear() {
    epoch = 0;
    KKK[0] = 0;
    KKK[1] = 0;
    KKK[2] = 0;
    TH = 0;
    for(int i = 0; i < 10; i++) {
        for(int j = 0; j < 3; j++) {
            ia_K[i][j] = 0;
            js_K[i][j] = 0;
            ndcg_K[i][j] = 0;
            aae_K[i][j] = 0;
            are_K[i][j] = 0;

            Recall_TH[i][j] = 0;
            Precision_TH[i][j] = 0;
            aae_TH[i][j] = 0;
            are_TH[i][j] = 0;
        }
    }
}



template<typename T,typename T2>
inline bool comp(const std::pair<T,T2>& a, const std::pair<T,T2>& b){
    return a.second > b.second;
}

inline void print_top_k(std::vector< std::pair<FullKey, COUNT_TYPE> > _mp, std::vector< std::pair<FullKey, COUNT_TYPE> > _record, uint32_t _K, uint32_t _key_type) {
    std::cout << "Top-k Flows: " << std::endl;
    for(uint32_t i = 0; i < _K; i++) {
        std::cout << "( " << std::setw(20) << std::left << lpi_print((lpi_protocol_t)_record[i].first.key[0]) << ",  " << std::setw(15) << std::left << ValueToIP(_record[i].first.key[1]);
        std::cout << " )  ->  " << std::setw(12) << std::left << _record[i].second << std::setw(8) << std::left << "        ";
        std::cout << "( " << std::setw(20) << std::left << lpi_print((lpi_protocol_t)_mp[i].first.key[0]) << ",  " << std::setw(15) << std::left << ValueToIP(_mp[i].first.key[1]);
        std::cout << " )  ->  " << std::setw(12) << std::left << _mp[i].second << std::endl;
    }
}

inline void print_top_k(std::vector< std::pair<uint32_t, COUNT_TYPE> > _mp, std::vector< std::pair<uint32_t, COUNT_TYPE> > _record, uint32_t _K, uint32_t _key_type) {
    if(_key_type == 2) {
        std::cout << "Top-k Apps: " << std::endl;
        for(uint32_t i = 0; i < _K; i++) {
            std::cout << std::setw(20) << std::left << lpi_print((lpi_protocol_t)_record[i].first) << "  ->  " << std::setw(12) << std::left << _record[i].second << std::setw(8) << std::left << "        ";
            std::cout << std::setw(20) << std::left << lpi_print((lpi_protocol_t)_mp[i].first) << "  ->  " << std::setw(12) << std::left << _mp[i].second << std::endl;
        }
    }
    if(_key_type == 3) {
        std::cout << "Top-k Sources: " << std::endl;
        for(uint32_t i = 0; i < _K; i++) {
            std::cout << std::setw(15) << std::left << ValueToIP(_record[i].first) << "  ->  " << std::setw(12) << std::left << _record[i].second << std::setw(8) << std::left << "        ";
            std::cout << std::setw(15) << std::left << ValueToIP(_mp[i].first) << "  ->  " << std::setw(12) << std::left << _mp[i].second << std::endl;
        }
    }
}

class BenchMark{
public:

    BenchMark(std::string PATH, std::string name, uint64_t bufferSize){

        fileName = name;

        adaptor =  new Adaptor(PATH+name, bufferSize);
        std::cout << "[Dataset]: " << name << std::endl;
        adaptor->Reset();

        COUNT_TYPE sum = 0;

        tuple_t t;
        memset(&t, 0, sizeof(tuple_t));
        while(adaptor->GetNext(&t) == 1) {
            sum += t.bytes_sum;
            uint32_t appKey = (uint32_t)t.key.AppProto;
            uint32_t srcKey = t.key.src_ip;
            FullKey fKey;
            fKey.key[0] = appKey;
            fKey.key[1] = srcKey;
            if(t.bytes_sum!=0){
                fKey_mp[fKey] += t.bytes_sum;
                pKey_mp[0][appKey] += t.bytes_sum;
                pKey_mp[1][srcKey] += t.bytes_sum;
            }
        }
    }

    ~BenchMark(){
    }

    void HHMultiBench_AppSketch(uint32_t MEMORY, bool th_K_flag, double alpha, uint32_t* K, uint32_t hash_num) {
        TP t1,t2,t3;
        AppSketch<FullKey>* sketch = new AppSketch<FullKey>(MEMORY,hash_num);

        adaptor->Reset();

        COUNT_TYPE sum = 0;
        tuple_t t;
        memset(&t, 0, sizeof(tuple_t));
        t1 = now();
        while(adaptor->GetNext(&t) == 1) {
            sum += t.bytes_sum;
            uint32_t appKey = (uint32_t)t.key.AppProto;
            uint32_t srcKey = t.key.src_ip;
            FullKey fKey;
            fKey.key[0] = appKey;
            fKey.key[1] = srcKey;
            if(t.bytes_sum!=0){
                sketch->Insert(fKey, t.bytes_sum);
            }
        }
        t2 = now();

        std::unordered_map<FullKey, COUNT_TYPE> estFKey = sketch->AllQuery();
        std::unordered_map<uint32_t, COUNT_TYPE> estPKey[2];

        for(auto it = estFKey.begin();it != estFKey.end();++it){
            estPKey[0][(it->first).key[0]] += it->second;
            estPKey[1][(it->first).key[1]] += it->second;
        }
        t3 = now();

        uint32_t prt_idx = 0;
        if(!th_K_flag) {
            COUNT_TYPE threshold = alpha * sum;

            CompareHH(estFKey, fKey_mp, threshold, 1, prt_idx);
            CompareHH(estPKey[0], pKey_mp[0], threshold, 2, prt_idx);
            CompareHH(estPKey[1], pKey_mp[1], threshold, 3, prt_idx);
        }
        else {
            CompareTopK(estFKey, fKey_mp, K[0], 1, prt_idx);
            CompareTopK(estPKey[0], pKey_mp[0], K[1], 2, prt_idx);
            CompareTopK(estPKey[1], pKey_mp[1], K[2], 3, prt_idx);
        }

        insert_t[prt_idx] += durationms(t2,t1);
        query_t[prt_idx] += durationms(t3,t2);

        delete sketch;
    }

    void HHMultiBench_USS(uint32_t MEMORY, bool th_K_flag, double alpha, uint32_t* K) {
        TP t1,t2,t3;
        USS<FullKey>* sketch = new USS<FullKey>(MEMORY);

        adaptor->Reset();

        COUNT_TYPE sum = 0;
        tuple_t t;
        memset(&t, 0, sizeof(tuple_t));
        t1 = now();
        while(adaptor->GetNext(&t) == 1) {
            sum += t.bytes_sum;
            uint32_t appKey = (uint32_t)t.key.AppProto;
            uint32_t srcKey = t.key.src_ip;
            FullKey fKey;
            fKey.key[0] = appKey;
            fKey.key[1] = srcKey;
            if(t.bytes_sum!=0){
                sketch->Insert(fKey, t.bytes_sum);
            }
        }
        t2 = now();

        std::unordered_map<FullKey, COUNT_TYPE> estFKey = sketch->AllQuery();
        std::unordered_map<uint32_t, COUNT_TYPE> estPKey[2];

        for(auto it = estFKey.begin();it != estFKey.end();++it){
            estPKey[0][(it->first).key[0]] += it->second;
            estPKey[1][(it->first).key[1]] += it->second;
        }
        t3 = now();

        uint32_t prt_idx = 1;
        if(!th_K_flag) {
            COUNT_TYPE threshold = alpha * sum;

            CompareHH(estFKey, fKey_mp, threshold, 1, prt_idx);
            CompareHH(estPKey[0], pKey_mp[0], threshold, 2, prt_idx);
            CompareHH(estPKey[1], pKey_mp[1], threshold, 3, prt_idx);
        }
        else {
            CompareTopK(estFKey, fKey_mp, K[0], 1, prt_idx);
            CompareTopK(estPKey[0], pKey_mp[0], K[1], 2, prt_idx);
            CompareTopK(estPKey[1], pKey_mp[1], K[2], 3, prt_idx);
        }

        insert_t[prt_idx] += durationms(t2,t1);
        query_t[prt_idx] += durationms(t3,t2);

        delete sketch;
    }

    void Bench_WavingSketch(uint32_t MEMORY, bool th_K_flag, double alpha, uint32_t* K) {
        TP t1,t2,t3;
        WavingSketch<FullKey,8,1>* sketch = new WavingSketch<FullKey,8,1>(MEMORY);

        adaptor->Reset();

        COUNT_TYPE sum = 0;
        tuple_t t;
        memset(&t, 0, sizeof(tuple_t));
        t1 = now();
        while(adaptor->GetNext(&t) == 1) {
            sum += t.bytes_sum;
            uint32_t appKey = (uint32_t)t.key.AppProto;
            uint32_t srcKey = t.key.src_ip;
            FullKey fKey;
            fKey.key[0] = appKey;
            fKey.key[1] = srcKey;
            if(t.bytes_sum!=0){
                sketch->Insert(fKey, t.bytes_sum);
            }
        }
        t2 = now();

        std::unordered_map<FullKey, COUNT_TYPE> estFKey = sketch->AllQuery();
        std::unordered_map<uint32_t, COUNT_TYPE> estPKey[2];

        for(auto it = estFKey.begin();it != estFKey.end();++it){
            estPKey[0][(it->first).key[0]] += it->second;
            estPKey[1][(it->first).key[1]] += it->second;
        }
        t3 = now();

        uint32_t prt_idx = 2;
        if(!th_K_flag) {
            COUNT_TYPE threshold = alpha * sum;

            CompareHH(estFKey, fKey_mp, threshold, 1, prt_idx);
            CompareHH(estPKey[0], pKey_mp[0], threshold, 2, prt_idx);
            CompareHH(estPKey[1], pKey_mp[1], threshold, 3, prt_idx);
        }
        else {
            CompareTopK(estFKey, fKey_mp, K[0], 1, prt_idx);
            CompareTopK(estPKey[0], pKey_mp[0], K[1], 2, prt_idx);
            CompareTopK(estPKey[1], pKey_mp[1], K[2], 3, prt_idx);
        }

        insert_t[prt_idx] += durationms(t2,t1);
        query_t[prt_idx] += durationms(t3,t2);

        delete sketch;
    }

    void SingleBench_WavingSketch(uint32_t MEMORY, bool th_K_flag, double alpha, uint32_t* K) {
        TP t1,t2,t3;

        //uint32_t mem = MEMORY / 3;

        uint32_t mem_total = fKey_mp.size() + pKey_mp[0].size() + pKey_mp[1].size();
        uint32_t mem = MEMORY * fKey_mp.size() / mem_total;
        uint32_t mem1 = MEMORY * pKey_mp[0].size() / mem_total;
        uint32_t mem2 = MEMORY * pKey_mp[1].size() / mem_total;

        if(mem1 < 104) {
            mem1 = 104;
            mem2 = mem2 - 104 + mem1;
        }

        WavingSketch<FullKey,8,1>* sketch = new WavingSketch<FullKey,8,1>(mem, "WavingSketch_Single");
        WavingSketch<uint32_t,8,1>* sketch_1 = new WavingSketch<uint32_t,8,1>(mem1, "WavingSketch_Single");
        WavingSketch<uint32_t,8,1>* sketch_2 = new WavingSketch<uint32_t,8,1>(mem2, "WavingSketch_Single");

        adaptor->Reset();

        COUNT_TYPE sum = 0;
        tuple_t t;
        memset(&t, 0, sizeof(tuple_t));
        t1 = now();
        while(adaptor->GetNext(&t) == 1) {
            sum += t.bytes_sum;
            uint32_t appKey = (uint32_t)t.key.AppProto;
            uint32_t srcKey = t.key.src_ip;
            FullKey fKey;
            fKey.key[0] = appKey;
            fKey.key[1] = srcKey;
            if(t.bytes_sum!=0){
                sketch->Insert(fKey, t.bytes_sum);
                sketch_1->Insert(appKey, t.bytes_sum);
                sketch_2->Insert(srcKey, t.bytes_sum);
            }
        }
        t2 = now();

        std::unordered_map<FullKey, COUNT_TYPE> estFKey = sketch->AllQuery();
        std::unordered_map<uint32_t, COUNT_TYPE> estPKey_1 = sketch_1->AllQuery();
        std::unordered_map<uint32_t, COUNT_TYPE> estPKey_2 = sketch_2->AllQuery();

        t3 = now();

        uint32_t prt_idx = 3;
        if(!th_K_flag) {
            COUNT_TYPE threshold = alpha * sum;

            CompareHH(estFKey, fKey_mp, threshold, 1, prt_idx);
            CompareHH(estPKey_1, pKey_mp[0], threshold, 2, prt_idx);
            CompareHH(estPKey_2, pKey_mp[1], threshold, 3, prt_idx);
        }
        else {
            CompareTopK(estFKey, fKey_mp, K[0], 1, prt_idx);
            CompareTopK(estPKey_1, pKey_mp[0], K[1], 2, prt_idx);
            CompareTopK(estPKey_2, pKey_mp[1], K[2], 3, prt_idx);
        }

        insert_t[prt_idx] += durationms(t2,t1);
        query_t[prt_idx] += durationms(t3,t2);

        delete sketch;
    }

    void Bench_HeavyGuardian(uint32_t MEMORY, bool th_K_flag, double alpha, uint32_t* K) {
        TP t1,t2,t3;
        HeavyGuardian<FullKey>* sketch = new HeavyGuardian<FullKey>(MEMORY);

        adaptor->Reset();

        COUNT_TYPE sum = 0;
        tuple_t t;
        memset(&t, 0, sizeof(tuple_t));
        t1 = now();
        while(adaptor->GetNext(&t) == 1) {
            sum += t.bytes_sum;
            uint32_t appKey = (uint32_t)t.key.AppProto;
            uint32_t srcKey = t.key.src_ip;
            FullKey fKey;
            fKey.key[0] = appKey;
            fKey.key[1] = srcKey;
            if(t.bytes_sum!=0){
                sketch->Insert(fKey, t.bytes_sum);
            }
        }
        t2 = now();

        std::unordered_map<FullKey, COUNT_TYPE> estFKey = sketch->AllQuery();
        std::unordered_map<uint32_t, COUNT_TYPE> estPKey[2];

        for(auto it = estFKey.begin();it != estFKey.end();++it){
            estPKey[0][(it->first).key[0]] += it->second;
            estPKey[1][(it->first).key[1]] += it->second;
        }

        t3 = now();

        uint32_t prt_idx = 4;
        if(!th_K_flag) {
            COUNT_TYPE threshold = alpha * sum;

            CompareHH(estFKey, fKey_mp, threshold, 1, prt_idx);
            CompareHH(estPKey[0], pKey_mp[0], threshold, 2, prt_idx);
            CompareHH(estPKey[1], pKey_mp[1], threshold, 3, prt_idx);
        }
        else {
            CompareTopK(estFKey, fKey_mp, K[0], 1, prt_idx);
            CompareTopK(estPKey[0], pKey_mp[0], K[1], 2, prt_idx);
            CompareTopK(estPKey[1], pKey_mp[1], K[2], 3, prt_idx);
        }

        insert_t[prt_idx] += durationms(t2,t1);
        query_t[prt_idx] += durationms(t3,t2);

        delete sketch;
    }

    void SingleBench_HeavyGuardian(uint32_t MEMORY, bool th_K_flag, double alpha, uint32_t* K) {
        TP t1,t2,t3;

        //uint32_t mem = MEMORY / 3;

        uint32_t mem_total = fKey_mp.size() + pKey_mp[0].size() + pKey_mp[1].size();
        uint32_t mem = MEMORY * fKey_mp.size() / mem_total;
        uint32_t mem1 = MEMORY * pKey_mp[0].size() / mem_total;
        uint32_t mem2 = MEMORY * pKey_mp[1].size() / mem_total;

        if(mem1 < 128) {
            mem1 = 128;
            mem2 = mem2 - 128 + mem1;
        }

        HeavyGuardian<FullKey>* sketch = new HeavyGuardian<FullKey>(mem, "HeavyGuardian_Single");
        HeavyGuardian<uint32_t>* sketch_1 = new HeavyGuardian<uint32_t>(mem1, "HeavyGuardian_Single");
        HeavyGuardian<uint32_t>* sketch_2 = new HeavyGuardian<uint32_t>(mem2, "HeavyGuardian_Single");

        adaptor->Reset();

        COUNT_TYPE sum = 0;
        tuple_t t;
        memset(&t, 0, sizeof(tuple_t));
        t1 = now();
        while(adaptor->GetNext(&t) == 1) {
            sum += t.bytes_sum;
            uint32_t appKey = (uint32_t)t.key.AppProto;
            uint32_t srcKey = t.key.src_ip;
            FullKey fKey;
            fKey.key[0] = appKey;
            fKey.key[1] = srcKey;
            if(t.bytes_sum!=0){
                sketch->Insert(fKey, t.bytes_sum);
                sketch_1->Insert(appKey, t.bytes_sum);
                sketch_2->Insert(srcKey, t.bytes_sum);
            }
        }
        t2 = now();

        std::unordered_map<FullKey, COUNT_TYPE> estFKey = sketch->AllQuery();
        std::unordered_map<uint32_t, COUNT_TYPE> estPKey_1 = sketch_1->AllQuery();
        std::unordered_map<uint32_t, COUNT_TYPE> estPKey_2 = sketch_2->AllQuery();
        t3 = now();

        uint32_t prt_idx = 5;
        if(!th_K_flag) {
            COUNT_TYPE threshold = alpha * sum;

            CompareHH(estFKey, fKey_mp, threshold, 1, prt_idx);
            CompareHH(estPKey_1, pKey_mp[0], threshold, 2, prt_idx);
            CompareHH(estPKey_2, pKey_mp[1], threshold, 3, prt_idx);
        }
        else {
            CompareTopK(estFKey, fKey_mp, K[0], 1, prt_idx);
            CompareTopK(estPKey_1, pKey_mp[0], K[1], 2, prt_idx);
            CompareTopK(estPKey_2, pKey_mp[1], K[2], 3, prt_idx);
        }

        insert_t[prt_idx] += durationms(t2,t1);
        query_t[prt_idx] += durationms(t3,t2);

        delete sketch;
    }

    void Bench_ColdFilter(uint32_t MEMORY, bool th_K_flag, double alpha, uint32_t* K, uint32_t hash_num, double r) {
        TP t1,t2,t3;
        COUNT_TYPE sum = 0;
        tuple_t t;
        memset(&t, 0, sizeof(tuple_t));
        while(adaptor->GetNext(&t) == 1) {
            sum += t.bytes_sum;
        }

        COUNT_TYPE threshold = alpha * sum;

        if(th_K_flag) {
            std::vector< std::pair<FullKey, COUNT_TYPE> > fKey_mp_v(fKey_mp.begin(),fKey_mp.end());

            bool (*cmp)(const std::pair<FullKey,COUNT_TYPE>& a, const std::pair<FullKey,COUNT_TYPE>& b) = comp<FullKey,COUNT_TYPE>;

            sort(fKey_mp_v.begin(),fKey_mp_v.end(),cmp);

            threshold = fKey_mp_v[(K[0]-1)<fKey_mp_v.size()?(K[0]-1):fKey_mp_v.size()].second;
        }

        threshold = threshold < 65550 ? threshold : 65550;

        //const double r = 0.99;

        SpaceSaving<FullKey>* SS = new SpaceSaving<FullKey>(r*MEMORY);

        ColdFilter<FullKey>* sketch = new ColdFilter<FullKey>((1-r)*MEMORY,SS,threshold, 65, hash_num);

        adaptor->Reset();

        memset(&t, 0, sizeof(tuple_t));
        t1 = now();
        while(adaptor->GetNext(&t) == 1) {
            uint32_t appKey = (uint32_t)t.key.AppProto;
            uint32_t srcKey = t.key.src_ip;
            FullKey fKey;
            fKey.key[0] = appKey;
            fKey.key[1] = srcKey;
            if(t.bytes_sum!=0){
                sketch->Insert(fKey, t.bytes_sum);
            }
        }
        t2 = now();

        std::unordered_map<FullKey, COUNT_TYPE> estFKey = sketch->AllQuery();
        std::unordered_map<uint32_t, COUNT_TYPE> estPKey[2];

        for(auto it = estFKey.begin();it != estFKey.end();++it){
            estPKey[0][(it->first).key[0]] += it->second;
            estPKey[1][(it->first).key[1]] += it->second;
        }
        t3 = now();

        uint32_t prt_idx = 6;
        if(!th_K_flag) {
            COUNT_TYPE threshold = alpha * sum;

            CompareHH(estFKey, fKey_mp, threshold, 1, prt_idx);
            CompareHH(estPKey[0], pKey_mp[0], threshold, 2, prt_idx);
            CompareHH(estPKey[1], pKey_mp[1], threshold, 3, prt_idx);
        }
        else {
            CompareTopK(estFKey, fKey_mp, K[0], 1, prt_idx);
            CompareTopK(estPKey[0], pKey_mp[0], K[1], 2, prt_idx);
            CompareTopK(estPKey[1], pKey_mp[1], K[2], 3, prt_idx);
        }

        insert_t[prt_idx] += durationms(t2,t1);
        query_t[prt_idx] += durationms(t3,t2);

        delete sketch;
    }

    void SingleBench_ColdFilter(uint32_t MEMORY, bool th_K_flag, double alpha, uint32_t* K, uint32_t hash_num, double r) {
        TP t1,t2,t3;

        //uint32_t mem = MEMORY / 3;

        uint32_t mem_total = fKey_mp.size() + pKey_mp[0].size() + pKey_mp[1].size();
        uint32_t mem = MEMORY * fKey_mp.size() / mem_total;
        uint32_t mem1 = MEMORY * pKey_mp[0].size() / mem_total;
        uint32_t mem2 = MEMORY * pKey_mp[1].size() / mem_total;

        if(mem1 < 1300) {
            mem1 = 1300;
            mem2 = mem2 - 1300 + mem1;
        }

        COUNT_TYPE sum = 0;
        tuple_t t;
        memset(&t, 0, sizeof(tuple_t));
        while(adaptor->GetNext(&t) == 1) {
            sum += t.bytes_sum;
        }

        COUNT_TYPE threshold = alpha * sum;
        COUNT_TYPE threshold_1 = alpha * sum;
        COUNT_TYPE threshold_2 = alpha * sum;

        if(th_K_flag) {
            std::vector< std::pair<FullKey, COUNT_TYPE> > fKey_mp_v(fKey_mp.begin(),fKey_mp.end());
            std::vector< std::pair<uint32_t, COUNT_TYPE> > pKey_mp_1_v(pKey_mp[0].begin(),pKey_mp[0].end());
            std::vector< std::pair<uint32_t, COUNT_TYPE> > pKey_mp_2_v(pKey_mp[1].begin(),pKey_mp[1].end());

            bool (*cmp)(const std::pair<FullKey,COUNT_TYPE>& a, const std::pair<FullKey,COUNT_TYPE>& b) = comp<FullKey,COUNT_TYPE>;
            bool (*cmp2)(const std::pair<uint32_t,COUNT_TYPE>& a, const std::pair<uint32_t,COUNT_TYPE>& b) = comp<uint32_t,COUNT_TYPE>;

            sort(fKey_mp_v.begin(),fKey_mp_v.end(),cmp);
            sort(pKey_mp_1_v.begin(),pKey_mp_1_v.end(),cmp2);
            sort(pKey_mp_2_v.begin(),pKey_mp_2_v.end(),cmp2);

            threshold = fKey_mp_v[(K[0]-1)<fKey_mp_v.size()?(K[0]-1):fKey_mp_v.size()].second;
            threshold_1 = pKey_mp_1_v[(K[1]-1)<pKey_mp_1_v.size()?(K[1]-1):pKey_mp_1_v.size()].second;
            threshold_2 = pKey_mp_2_v[(K[2]-1)<pKey_mp_2_v.size()?(K[2]-1):pKey_mp_2_v.size()].second;
        }

        threshold = threshold < 65550 ? threshold : 65550;
        threshold_1 = threshold_1 < 65550 ? threshold_1 : 65550;
        threshold_2 = threshold_2 < 65550 ? threshold_2 : 65550;

        //const double r = 0.99;

        SpaceSaving<FullKey>* SS = new SpaceSaving<FullKey>(r*mem);
        ColdFilter<FullKey>* sketch = new ColdFilter<FullKey>((1-r)*mem,SS,threshold,65,hash_num,"ColdFilter_Single");

        SpaceSaving<uint32_t>* SS_1 = new SpaceSaving<uint32_t>(r*mem1);
        ColdFilter<uint32_t>* sketch_1 = new ColdFilter<uint32_t>((1-r)*mem1,SS_1,threshold_1,65,hash_num,"ColdFilter_Single");

        SpaceSaving<uint32_t>* SS_2 = new SpaceSaving<uint32_t>(r*mem2);
        ColdFilter<uint32_t>* sketch_2 = new ColdFilter<uint32_t>((1-r)*mem2,SS_2,threshold_2,65,hash_num,"ColdFilter_Single");

        adaptor->Reset();

        memset(&t, 0, sizeof(tuple_t));
        t1 = now();
        while(adaptor->GetNext(&t) == 1) {
            uint32_t appKey = (uint32_t)t.key.AppProto;
            uint32_t srcKey = t.key.src_ip;
            FullKey fKey;
            fKey.key[0] = appKey;
            fKey.key[1] = srcKey;
            if(t.bytes_sum!=0){
                sketch->Insert(fKey, t.bytes_sum);
                sketch_1->Insert(appKey, t.bytes_sum);
                sketch_2->Insert(srcKey, t.bytes_sum);
            }
        }
        t2 = now();

        std::unordered_map<FullKey, COUNT_TYPE> estFKey = sketch->AllQuery();
        std::unordered_map<uint32_t, COUNT_TYPE> estPKey_1 = sketch_1->AllQuery();
        std::unordered_map<uint32_t, COUNT_TYPE> estPKey_2 = sketch_2->AllQuery();

        t3 = now();

        uint32_t prt_idx = 7;
        if(!th_K_flag) {
            CompareHH(estFKey, fKey_mp, threshold, 1, prt_idx);
            CompareHH(estPKey_1, pKey_mp[0], threshold_1, 2, prt_idx);
            CompareHH(estPKey_2, pKey_mp[1], threshold_2, 3, prt_idx);
        }
        else {
            CompareTopK(estFKey, fKey_mp, K[0], 1, prt_idx);
            CompareTopK(estPKey_1, pKey_mp[0], K[1], 2, prt_idx);
            CompareTopK(estPKey_2, pKey_mp[1], K[2], 3, prt_idx);
        }

        insert_t[prt_idx] += durationms(t2,t1);
        query_t[prt_idx] += durationms(t3,t2);

        delete sketch;
    }

    void HHMultiBench_DMatrix(uint32_t MEMORY, bool th_K_flag, double alpha, uint32_t* K, uint32_t hash_num) {
        TP t1,t2,t3;

        const double p = (double)pKey_mp[0].size() / pKey_mp[1].size();

        DMatrix<FullKey>* sketch = new DMatrix<FullKey>(MEMORY,p,hash_num);

        adaptor->Reset();

        COUNT_TYPE sum = 0;
        tuple_t t;
        memset(&t, 0, sizeof(tuple_t));
        t1 = now();
        while(adaptor->GetNext(&t) == 1) {
            sum += t.bytes_sum;
            uint32_t appKey = (uint32_t)t.key.AppProto;
            uint32_t srcKey = t.key.src_ip;
            FullKey fKey;
            fKey.key[0] = appKey;
            fKey.key[1] = srcKey;
            if(t.bytes_sum!=0){
                sketch->Insert(fKey, t.bytes_sum);
            }
        }
        t2 = now();

        std::unordered_map<FullKey, COUNT_TYPE> estFKey = sketch->AllQuery();
        std::unordered_map<uint32_t, COUNT_TYPE> estPKey[2];

        for(auto it = estFKey.begin();it != estFKey.end();++it){
            estPKey[0][(it->first).key[0]] += it->second;
            estPKey[1][(it->first).key[1]] += it->second;
        }
        t3 = now();

        uint32_t prt_idx = 8;
        if(!th_K_flag) {
            COUNT_TYPE threshold = alpha * sum;

            CompareHH(estFKey, fKey_mp, threshold, 1, prt_idx);
            CompareHH(estPKey[0], pKey_mp[0], threshold, 2, prt_idx);
            CompareHH(estPKey[1], pKey_mp[1], threshold, 3, prt_idx);
        }
        else {
            CompareTopK(estFKey, fKey_mp, K[0], 1, prt_idx);
            CompareTopK(estPKey[0], pKey_mp[0], K[1], 2, prt_idx);
            CompareTopK(estPKey[1], pKey_mp[1], K[2], 3, prt_idx);
        }

        insert_t[prt_idx] += durationms(t2,t1);
        query_t[prt_idx] += durationms(t3,t2);

        delete sketch;
    }

    void Bench_CocoWaving(uint32_t MEMORY, bool th_K_flag, double alpha, uint32_t* K, uint32_t hash_num) {
        TP t1,t2,t3;
        CocoWaving<FullKey,8,1>* sketch = new CocoWaving<FullKey,8,1>(MEMORY,hash_num);

        adaptor->Reset();

        COUNT_TYPE sum = 0;
        tuple_t t;
        memset(&t, 0, sizeof(tuple_t));
        t1 = now();
        while(adaptor->GetNext(&t) == 1) {
            sum += t.bytes_sum;
            uint32_t appKey = (uint32_t)t.key.AppProto;
            uint32_t srcKey = t.key.src_ip;
            FullKey fKey;
            fKey.key[0] = appKey;
            fKey.key[1] = srcKey;
            if(t.bytes_sum!=0){
                sketch->Insert(fKey, t.bytes_sum);
            }
        }
        t2 = now();

        std::unordered_map<FullKey, COUNT_TYPE> estFKey = sketch->AllQuery();
        std::unordered_map<uint32_t, COUNT_TYPE> estPKey[2];

        for(auto it = estFKey.begin();it != estFKey.end();++it){
            estPKey[0][(it->first).key[0]] += it->second;
            estPKey[1][(it->first).key[1]] += it->second;
        }
        t3 = now();

        uint32_t prt_idx = 9;
        if(!th_K_flag) {
            COUNT_TYPE threshold = alpha * sum;

            CompareHH(estFKey, fKey_mp, threshold, 1, prt_idx);
            CompareHH(estPKey[0], pKey_mp[0], threshold, 2, prt_idx);
            CompareHH(estPKey[1], pKey_mp[1], threshold, 3, prt_idx);
        }
        else {
            CompareTopK(estFKey, fKey_mp, K[0], 1, prt_idx);
            CompareTopK(estPKey[0], pKey_mp[0], K[1], 2, prt_idx);
            CompareTopK(estPKey[1], pKey_mp[1], K[2], 3, prt_idx);
        }

        insert_t[prt_idx] += durationms(t2,t1);
        query_t[prt_idx] += durationms(t3,t2);

        delete sketch;
    }

private:
    std::string fileName;
    Adaptor* adaptor;

    std::unordered_map<FullKey, COUNT_TYPE> fKey_mp;
    std::unordered_map<uint32_t, COUNT_TYPE> pKey_mp[2];

    template<class T>
    void CompareHH(T mp, T record, COUNT_TYPE threshold, uint32_t key_type, uint32_t prt_idx){
        double realHH = 0, estHH = 0, bothHH = 0, aae = 0, are = 0;

        for(auto it = record.begin();it != record.end();++it){
            bool real, est;
            double realF = it->second, estF = mp[it->first];

            real = (realF > threshold);
            est = (estF > threshold);

            realHH += real;
            estHH += est;

            if(real && est){
                bothHH += 1;
                aae += abs(realF - estF);
                are += abs(realF - estF) / realF;
            }
        }

        //std::cout << "key-type: " << key_type << std::endl;
        //std::cout << "threshold: " << threshold << std::endl;

        //std::cout << "Recall: " << bothHH / realHH << std::endl;
        //std::cout << "Precision: " << bothHH / estHH << std::endl;

        //std::cout << "aae: " << aae / bothHH << std::endl;
        //std::cout << "are: " << are / bothHH << std::endl;
        //std::cout << std::endl;

        TH = threshold;
        Recall_TH[prt_idx][key_type-1] += bothHH / realHH;
        Precision_TH[prt_idx][key_type-1] += bothHH / estHH;
        aae_TH[prt_idx][key_type-1] += aae / bothHH;
        are_TH[prt_idx][key_type-1] += are / bothHH;
    }

    template<typename T,typename T2>
    void CompareTopK(std::unordered_map<T, T2> _est, std::unordered_map<T, T2> _real, uint32_t _K, uint32_t _key_type, uint32_t _prt_idx) {

        std::vector< std::pair<T, T2> > est_v(_est.begin(),_est.end());
        std::vector< std::pair<T, T2> > real_v(_real.begin(),_real.end());

        bool (*cmp)(const std::pair<T,T2>& a, const std::pair<T,T2>& b) = comp<T,T2>;

        sort(est_v.begin(),est_v.end(),cmp);
        sort(real_v.begin(),real_v.end(),cmp);

        if(real_v.size() <= _K) {
            est_v.resize(real_v.size());
        }
        else {
            est_v.resize(_K);
            real_v.resize(_K);
        }

        if(real_v.size() <= _K) {
            CompareK(est_v, real_v, real_v.size(), _key_type, _prt_idx);
        }
        else {
            CompareK(est_v, real_v, _K, _key_type, _prt_idx);
        }
    }

    template<typename T,typename T2>
    void CompareK(std::vector< std::pair<T, T2> > mp, std::vector< std::pair<T, T2> > record, uint32_t K, uint32_t key_type, uint32_t prt_idx){
        //double realHH = 0, estHH = 0;
        double bothHH = 0, aae = 0, are = 0;
        double ia = 0, js = 0, dcg = 0, idcg =0, ndcg =0;

        //realHH = record.size();
        //estHH = mp.size();

        int idx_real = 0;
        for(auto it_real = record.begin();it_real != record.end();++it_real){
            double realF = it_real->second, estF = 0;
            idx_real += 1;
            bool match = false;
            int idx_est = 0;
            for(auto it_est = mp.begin();it_est != mp.end();++it_est) {
                idx_est += 1;
                if(it_real->first == it_est->first) {
                    match = true;
                    estF = it_est->second;
                    break;
                }
            }
            if(match) {
                bothHH += 1;
                aae += abs(realF - estF);
                are += abs(realF - estF) / realF;

                dcg += (K-abs(idx_real-idx_est))/(log(idx_real+2)/log(2));
            }
            else {
                aae += abs(realF - 0);
                are += abs(realF - 0) / realF;
            }
            idcg += K/(log(idx_real+2)/log(2));
        }

        ia = bothHH / K;
        js = bothHH / (2 * K - bothHH);
        ndcg = dcg / idcg;

        //std::cout << "key-type: " << key_type << std::endl;
        //std::cout << "K: " << record.size() << std::endl;

        //std::cout << "ia: " << ia << std::endl;
        //std::cout << "js: " << js << std::endl;
        //std::cout << "ndcg: " << ndcg << std::endl;

        //std::cout << "aae: " << aae / bothHH << std::endl;
        //std::cout << "are: " << are / bothHH << std::endl;

        //std::cout << std::endl;

        KKK[key_type-1] = record.size();
        ia_K[prt_idx][key_type-1] += ia;
        js_K[prt_idx][key_type-1] += js;
        ndcg_K[prt_idx][key_type-1] += ndcg;
        aae_K[prt_idx][key_type-1] += aae / K;
        are_K[prt_idx][key_type-1] += are / K;
    }
};

#endif
