#ifndef APPSKETCH_H
#define APPSKETCH_H

#include "MultiAbstract.h"

template<typename DATA_TYPE>
class AppSketch : public MultiAbstract<DATA_TYPE>{
public:

    typedef typename MultiAbstract<DATA_TYPE>::Counter Counter;
    typedef std::unordered_map<DATA_TYPE, COUNT_TYPE> HashMap;

    AppSketch(uint32_t _MEMORY, uint32_t _HASH_NUM = 2, std::string _name = "AppSketch"){
        this->name = _name;
        std::cout << this->name << std::endl;

        HASH_NUM = _HASH_NUM;
        LENGTH = _MEMORY / _HASH_NUM / sizeof(Counter);

        counter = new Counter* [HASH_NUM];
        for(uint32_t i = 0;i < HASH_NUM;++i){
            counter[i] = new Counter [LENGTH];
            memset(counter[i], 0, sizeof(Counter) * LENGTH);
        }
    }

    ~AppSketch(){
        for(uint32_t i = 0;i < HASH_NUM;++i){
            delete [] counter[i];
        }
        delete [] counter;
    }

    void Insert(const DATA_TYPE& item, const COUNT_TYPE& p_s){
        COUNT_TYPE minimum = 0x7fffffff;
        //COUNT_TYPE minimum = 0xffffffff;
        uint32_t minPos = 0, minHash = 0;

        for(uint32_t i = 0;i < HASH_NUM;++i){
            uint32_t position = hash(item, i) % LENGTH;
            if(counter[i][position].ID == item){
                counter[i][position].count += p_s;
                return;
            }
            if(counter[i][position].count < minimum){
                minPos = position;
                minHash = i;
                minimum = counter[i][position].count;
            }
        }

        counter[minHash][minPos].count += p_s;
        if(randomGenerator() % (counter[minHash][minPos].count + p_s) < p_s){
            counter[minHash][minPos].ID = item;
        }
    }

    HashMap AllQuery(){
        HashMap ret;

        for(uint32_t i = 0;i < HASH_NUM;++i){
            for(uint32_t j = 0;j < LENGTH;++j){
                ret[counter[i][j].ID] = counter[i][j].count;
            }
        }

        return ret;
    }

private:
    uint32_t LENGTH;
    uint32_t HASH_NUM;

    Counter** counter;
};

#endif
