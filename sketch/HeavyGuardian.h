#ifndef HEAVYGUARDIAN_H
#define HEAVYGUARDIAN_H

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <string>
#include <cstring>
#include "MultiAbstract.h"
#define G 8
#define HK_b 1.08

template<typename DATA_TYPE>
class HeavyGuardian : public MultiAbstract<DATA_TYPE> {
    public:

        typedef struct node
        {
            DATA_TYPE K;
            COUNT_TYPE C;
        }node_t;
        typedef std::unordered_map<DATA_TYPE, COUNT_TYPE> HashMap;

        HeavyGuardian(uint32_t _MEMORY, std::string _name = "HeavyGuardian")
        {
            this->name = _name;
            std::cout << this->name << std::endl;

            M = _MEMORY / sizeof(node_t) / G;
            HK = new node_t* [M];
            for (uint32_t i=0; i<M; i++)
            {
                HK[i] = new node_t [G];
                memset(HK[i], 0, sizeof(node_t) * G);
            }
        }

        void Insert(const DATA_TYPE& item, const COUNT_TYPE& p_s)
        {
            uint32_t Hsh=hash(item,0) % M;
            bool FLAG=false;
            for (int k=0; k<G; k++)
            {
                if (HK[Hsh][k].K==item)
                {
                    HK[Hsh][k].C += p_s;
                    FLAG=true;
                    break;
                }
            }
            if (!FLAG)
            {
                int X;
                COUNT_TYPE min_num = INT_MAX;
                for (int k=0; k<G; k++)
                {
                    COUNT_TYPE c=HK[Hsh][k].C;
                    if (c<min_num) {min_num=c; X=k;}
                }
                if (randomGenerator() % int(pow(HK_b,HK[Hsh][X].C)) == 0)
                {
                    HK[Hsh][X].C -= p_s;
                    if (HK[Hsh][X].C<=0)
                    {
                        HK[Hsh][X].K=item;
                        HK[Hsh][X].C=p_s;
                    }
                }
            }
        }

        COUNT_TYPE Query(const DATA_TYPE& item)
        {
            uint32_t Hsh=hash(item,0) % M;
            for (int k=0; k<G; k++)
            {
                if (HK[Hsh][k].K==item)
                    return HK[Hsh][k].C;
            }
            return 0;
        }

        HashMap AllQuery(){
            HashMap ret;

            for(uint32_t i = 0;i < M;++i){
                for(uint32_t j = 0;j < G;++j){
                    ret[HK[i][j].K] = HK[i][j].C;
                }
            }

            return ret;
        }

    private:
        node_t** HK;
        uint32_t M;
};
#endif
