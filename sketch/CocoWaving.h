#ifndef COCOWAVING_H
#define COCOWAVING_H

#include "MultiAbstract.h"

#define factor 1

static const COUNT_TYPE COUNT[2] = {1, -1};

template<typename DATA_TYPE, uint32_t slot_num, uint32_t counter_num>
class CocoWaving : public MultiAbstract<DATA_TYPE>{
public:

    typedef std::unordered_map<DATA_TYPE, COUNT_TYPE> HashMap;

    struct Bucket {
        DATA_TYPE items[slot_num];
        COUNT_TYPE counters[slot_num];
        int16_t incast[counter_num];

        void Insert(const DATA_TYPE& item,const COUNT_TYPE& p_s,uint32_t seed_s,uint32_t seed_incast) {
            uint32_t choice = hash(item, seed_s) & 1;
            uint32_t whichcast = hash(item, seed_incast) % counter_num;
            COUNT_TYPE min_num = INT_MAX;
            uint32_t min_pos = -1;

            for (uint32_t i = 0; i < slot_num; ++i) {
                if (counters[i] == 0) {
                    // The error free item's counter is negative, which is a trick to 
                    // be differentiated from items which are not error free.
                    items[i] = item;
                    counters[i] = -1 * p_s;
                    return;
                }
                else if (items[i] == item) {
                    if (counters[i] < 0)
                        counters[i] -= p_s;
                    else {
                        counters[i] += p_s;
                        incast[whichcast] += COUNT[choice] * p_s;
                    }
                    return;
                }

                COUNT_TYPE counter_val = std::abs(counters[i]);
                if (counter_val < min_num) {
                    min_num = counter_val;
                    min_pos = i;
                }
            }

            if (incast[whichcast] * COUNT[choice] >= int(min_num * factor)) {
                if (counters[min_pos] < 0) {
                    uint32_t min_choice = hash(items[min_pos], seed_s) & 1;
                    incast[hash(items[min_pos], seed_incast) % counter_num] += COUNT[min_choice] * counters[min_pos];
                }
                items[min_pos] = item;
                counters[min_pos] = min_num + p_s;
            }
            incast[whichcast] += COUNT[choice] * p_s;
        }

        COUNT_TYPE Query(const DATA_TYPE item,uint32_t seed_s,uint32_t seed_incast)
        {
            uint32_t choice = hash(item, seed_s) & 1;
            uint32_t whichcast = hash(item, seed_incast) % counter_num;
            COUNT_TYPE retv = 0;

            for (uint32_t i = 0; i < slot_num; ++i) {
                if (items[i] == item) {
                    return std::abs(counters[i]);
                }
            }
            return retv;
        }

        bool Exist(const DATA_TYPE item,uint32_t seed_s,uint32_t seed_incast)
        {
            for (uint32_t i = 0; i < slot_num; ++i) {
                if (items[i] == item) {
                    return true;
                }
            }
            return false;
        }

        COUNT_TYPE Count_value(const DATA_TYPE item,uint32_t seed_s,uint32_t seed_incast)
        {
            uint32_t choice = hash(item, seed_s) & 1;
            uint32_t whichcast = hash(item, seed_incast) % counter_num;
            COUNT_TYPE retv = std::abs(incast[whichcast] * COUNT[choice]);
            return retv;
        }
    };

    CocoWaving(uint32_t _MEMORY, uint32_t _HASH_NUM = 2, std::string _name = "CocoWaving"){
        this->name = _name;
        std::cout << this->name << std::endl;

        HASH_NUM = _HASH_NUM;
        BUCKET_NUM = _MEMORY / _HASH_NUM / sizeof(Bucket);

        buckets = new Bucket* [HASH_NUM];
        for(uint32_t i = 0;i < HASH_NUM;++i){
            buckets[i] = new Bucket [BUCKET_NUM];
            memset(buckets[i], 0, sizeof(Bucket) * BUCKET_NUM);
        }

        seed_incast=_HASH_NUM;
        seed_s=_HASH_NUM+1;
    }

    ~CocoWaving(){
        for(uint32_t i = 0;i < HASH_NUM;++i){
            delete [] buckets[i];
        }
        delete [] buckets;
    }

    void Insert(const DATA_TYPE& item, const COUNT_TYPE& p_s){
        COUNT_TYPE minimum = 0x7fffffff;
        uint32_t minPos = 0, minHash = 0;

        for(uint32_t i = 0;i < HASH_NUM;++i){
            uint32_t position = hash(item, i) % BUCKET_NUM;
            if(buckets[i][position].Exist(item,seed_s,seed_incast)){
                buckets[i][position].Insert(item,p_s,seed_s,seed_incast);
                return;
            }
            if(buckets[i][position].Count_value(item,seed_s,seed_incast) < minimum){
                minPos = position;
                minHash = i;
                minimum = buckets[i][position].Count_value(item,seed_s,seed_incast);
            }
        }

        buckets[minHash][minPos].Insert(item,p_s,seed_s,seed_incast);
    }

    COUNT_TYPE Query(const DATA_TYPE& item) {
        COUNT_TYPE minimum = 0x7fffffff;
        uint32_t minPos = 0, minHash = 0;

        for(uint32_t i = 0;i < HASH_NUM;++i){
            uint32_t position = hash(item, i) % BUCKET_NUM;
            if(buckets[i][position].Exist(item,seed_s,seed_incast)){
                return buckets[i][position].Query(item,seed_s,seed_incast);
            }
            if(buckets[i][position].Count_value(item,seed_s,seed_incast) < minimum){
                minPos = position;
                minHash = i;
                minimum = buckets[i][position].Count_value(item,seed_s,seed_incast);
            }
        }

        return minimum;
    }

    HashMap AllQuery(){
        HashMap ret;

        for(uint32_t i = 0;i < HASH_NUM;++i){
            for(uint32_t j = 0;j < BUCKET_NUM;++j){
                for(uint32_t k = 0; k < slot_num;++k){
                    ret[buckets[i][j].items[k]] = std::abs(buckets[i][j].counters[k]);
                }
            }
        }

        return ret;
    }

private:
    uint32_t HASH_NUM;
    uint32_t BUCKET_NUM;

    Bucket **buckets;

    uint32_t seed_s;
    uint32_t seed_incast;
};

#endif
