#ifndef WAVINGSKETCH_H
#define WAVINGSKETCH_H

#pragma once

#include "MultiAbstract.h"
#include <unistd.h>
#include <time.h>
#include <algorithm>

#define factor 1

//static const COUNT_TYPE COUNT[2] = {1, -1};

template < typename DATA_TYPE, uint32_t slot_num, uint32_t counter_num >
class WavingSketch : public MultiAbstract<DATA_TYPE>{
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
	};

	WavingSketch(uint32_t _MEMORY, std::string _name = "WavingSketch"){
                this->name = _name;
                std::cout << this->name << std::endl;

                BUCKET_NUM = _MEMORY / sizeof(Bucket);

		buckets = new Bucket[BUCKET_NUM];
		memset(buckets, 0, BUCKET_NUM * sizeof(Bucket));
		/*seed_choice=std::clock();
		sleep(1); // need to sleep for a while, or seed_choice and seed_incast might get the same seed!
		seed_incast=std::clock();
		sleep(1); // need to sleep for a while, or seed_incast and seed_s might get the same seed!
		seed_s=std::clock();*/

		seed_choice=0;
		seed_incast=1;
		seed_s=2;
	}
	~WavingSketch() { delete[] buckets; }

	void Insert(const DATA_TYPE& item, const COUNT_TYPE& p_s) {
		uint32_t bucket_pos = hash(item,seed_choice) % BUCKET_NUM;
		buckets[bucket_pos].Insert(item,p_s,seed_s,seed_incast);
	}

        COUNT_TYPE Query(const DATA_TYPE& item) {
		uint32_t bucket_pos = hash(item,seed_choice) % BUCKET_NUM;
		return buckets[bucket_pos].Query(item,seed_s,seed_incast);
	}

        HashMap AllQuery(){
                HashMap ret;

                for(uint32_t i = 0;i < BUCKET_NUM;++i){
                        for(uint32_t j = 0;j < slot_num;++j){
                            ret[buckets[i].items[j]] = std::abs(buckets[i].counters[j]);
                        }
                }

                return ret;
        }

private:
	Bucket *buckets;
	uint32_t BUCKET_NUM;
	uint32_t seed_choice;
	uint32_t seed_s;
	uint32_t seed_incast;
};

#endif
