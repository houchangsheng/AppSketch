#ifndef CHANGER_H
#define CHANGER_H

#include "sketch.hpp"
#include "sketch2.hpp"
#include "datatypes.hpp"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <inttypes.h>
#include <signal.h>

#include <libtrace.h>
#include <libtrace_parallel.h>
#include <libflowmanager.h>
#include "tools_common.h"

#include <iostream>
#include <fstream>

typedef std::unordered_map<key_tp, val_tp*, key_tp_hash, key_tp_eq> groundmap;

template <class S>
class HeavyChanger {

public:
    HeavyChanger(int depth, int length, int width, int lgn, int K, int K2);

    ~HeavyChanger();

    void Update(uint32_t * key, val_tp val);

    val_tp ChangeinSketch();

    void Querychangeredge(val_tp thresh, myvector& result);

    void Querychangernode(val_tp thresh, myvector& result);

    void Changedhitteredge(myvector& oldresult, myvector& curresult); // The number of changed heavy hitters on edge between two adjacent epoch

    void Changedhitternode(myvector& oldresult, myvector& curresult, unsigned int oldnum, unsigned int curnum);

    void Reset();


    S* GetCurSketch();

    S* GetOldSketch();

private:
    S* old_sk;

    S* cur_sk;

    int lgn_;
};

template <class S>
HeavyChanger<S>::HeavyChanger(int depth, int length, int width, int lgn, int K, int K2) {
    old_sk = new S(depth, length, width, lgn, K, K2);
    cur_sk = new S(depth, length, width, lgn, K, K2);
    lgn_ = lgn;
}


template <class S>
HeavyChanger<S>::~HeavyChanger() {
    delete old_sk;
    delete cur_sk;
}

template <class S>
void HeavyChanger<S>::Update(uint32_t * key, val_tp val) {
    cur_sk->Update(key, val);
}

template <class S>
val_tp HeavyChanger<S>::ChangeinSketch() {
    val_tp diff;
    S* S2[2];
    memcpy(&S2[0], &old_sk, sizeof(old_sk));
    memcpy(&S2[1], &cur_sk, sizeof(cur_sk));
    diff = cur_sk->Change(S2);
    return diff;
}

template <class S>
void HeavyChanger<S>::Querychangeredge(val_tp thresh, myvector& result) {
    myvector res1, res2;
    cur_sk->Queryheavychangeredge(thresh, res1);
    old_sk->Queryheavychangeredge(thresh, res2);
    myset reset;
    for (auto it = res1.begin(); it != res1.end(); it++) {
        reset.insert(it->first);
    }
    for (auto it = res2.begin(); it != res2.end(); it++) {
        reset.insert(it->first);
    }
    val_tp old_low, old_up, old_up2;
    val_tp new_low, new_up, new_up2;
    val_tp diff1, diff2;
    val_tp change, change2;
//    val_tp maxchange = 0, sumchange = 0;
    for (auto it = reset.begin(); it != reset.end(); it++) {
        old_low = old_sk->Low_estimate((uint32_t*)(*it).key);
        old_up = old_sk->Up_estimate((uint32_t*)(*it).key);
        new_low = cur_sk->Low_estimate((uint32_t*)(*it).key);
        new_up = cur_sk->Up_estimate((uint32_t*)(*it).key);
        diff1 = new_up > old_low ? new_up - old_low : old_low - new_up;
        diff2 = new_low > old_up ? new_low - old_up : old_up - new_low;
        change = diff1 > diff2 ? diff1 : diff2;
        if (change > thresh) {
            key_tp key;
            memcpy(key.key, it->key, lgn_);
            std::pair<key_tp, val_tp> cand;
            cand.first = key;
            //cand.second = change;

	    old_up2 = old_sk->Up_estimate2((uint32_t*)(*it).key);
	    new_up2 = cur_sk->Up_estimate2((uint32_t*)(*it).key);
	    change2 = new_up2 > old_up2 ? new_up2 - old_up2 : old_up2 - new_up2;
	    cand.second = change < change2 ? change : change2;

            result.push_back(cand);
//            maxchange = std::max(maxchange, change);
//            sumchange += change;
        }
    }
}

//Query for heavy changer on node
template <class S>
void HeavyChanger<S>::Querychangernode(val_tp thresh, myvector& result) {
    myvector res1, res2;
    cur_sk->Queryheavychangernode(thresh, res1);
    old_sk->Queryheavychangernode(thresh, res2);
    myset reset;
    for (auto it = res1.begin(); it != res1.end(); it++) {
	reset.insert(it->first);
    }
    for (auto it = res2.begin(); it != res2.end(); it++) {
        reset.insert(it->first);
    }
    val_tp old_low, old_up, old_up2;
    val_tp new_low, new_up, new_up2;
    val_tp diff1, diff2;
    val_tp change, change1, change2, change3;
    for (auto it = reset.begin(); it != reset.end(); it++) {
        old_low = old_sk->Low_estimatenode((uint32_t*)(*it).key);
        old_up = old_sk->Querynodeweight((uint32_t*)(*it).key);
        new_low = cur_sk->Low_estimatenode((uint32_t*)(*it).key);
        new_up = cur_sk->Querynodeweight((uint32_t*)(*it).key);
        diff1 = new_up > old_low ? new_up - old_low : old_low - new_up;
        diff2 = new_low > old_up ? new_low - old_up : old_up - new_low;
        change1 = diff1 > diff2 ? diff1 : diff2;

	old_up2 = old_sk->Querynodeweight2((uint32_t*)(*it).key);
	new_up2 = cur_sk->Querynodeweight2((uint32_t*)(*it).key);
	change2 = new_up2 > old_up2 ? new_up2 - old_up2 : old_up2 - new_up2;

	change = change1 < change2 ? change1 : change2;

        if (change > thresh) {
            key_tp key;
            memcpy(key.key, it->key, lgn_/2);
            std::pair<key_tp, val_tp> cand;
            cand.first = key;
            //cand.second = change;

	    /*old_up2 = old_sk->Querynodeweight2((uint32_t*)(*it).key);
	    new_up2 = cur_sk->Querynodeweight2((uint32_t*)(*it).key);
	    change2 = new_up2 > old_up2 ? new_up2 - old_up2 : old_up2 - new_up2;

	    change3 = new_up > old_up ? new_up - old_up : old_up - new_up;
	    cand.second = change < change2 ? (change < change3 ? change : change3) : (change2 < change3 ? change2 : change3);*/

	    change3 = new_up > old_up ? new_up - old_up : old_up - new_up;
	    cand.second = change < change3 ? change : change3;

            result.push_back(cand);
        }
    }
}

template <class S>
void HeavyChanger<S>::Changedhitteredge(myvector& oldresult, myvector& curresult) {
    mymap totalre;
    for (unsigned int i = 0; i < oldresult.size(); i++){
        if (totalre.find(oldresult.at(i).first) != totalre.end()) {
            totalre[oldresult.at(i).first] += 1;
        } else {
            totalre[oldresult.at(i).first] = 0;
            }
    }
    for (unsigned int i = 0; i < curresult.size(); i++){
        if (totalre.find(curresult.at(i).first) != totalre.end()) {
            totalre[curresult.at(i).first] += 1;
        } else {
            totalre[curresult.at(i).first] = 0;
            }
    }
    val_tp changecount = 0;
    for (auto it = totalre.begin(); it != totalre.end(); it++){
        if (it->second == 0){
            changecount++;
        }
    }
}

template <class S>
void HeavyChanger<S>::Changedhitternode(myvector& oldresult, myvector& curresult, unsigned int oldnum, unsigned int curnum) {
    mymap totalresrc, totalredst;
    for (unsigned int i = 0; i < oldnum; i++){
        if (totalresrc.find(oldresult.at(i).first) != totalresrc.end()) {
            totalresrc[oldresult.at(i).first] += 1;
        } else {
            totalresrc[oldresult.at(i).first] = 0;
            }
    }
    for (unsigned int i = 0; i < curnum; i++){
        if (totalresrc.find(curresult.at(i).first) != totalresrc.end()) {
            totalresrc[curresult.at(i).first] += 1;
        } else {
            totalresrc[curresult.at(i).first] = 0;
            }
    }
    val_tp changecount = 0;
    for (auto it = totalresrc.begin(); it != totalresrc.end(); it++){
        if (it->second == 0){
            changecount++;
        }
    }

    for (unsigned int i = oldnum; i < oldresult.size(); i++){
        if (totalredst.find(oldresult.at(i).first) != totalredst.end()) {
            totalredst[oldresult.at(i).first] += 1;
        } else {
            totalredst[oldresult.at(i).first] = 0;
            }
    }
    for (unsigned int i = curnum; i < curresult.size(); i++){
        if (totalredst.find(curresult.at(i).first) != totalredst.end()) {
            totalredst[curresult.at(i).first] += 1;
        } else {
            totalredst[curresult.at(i).first] = 0;
            }
    }
    val_tp changecountdst = 0;
    for (auto it = totalredst.begin(); it != totalredst.end(); it++){
        if (it->second == 0){
            changecountdst++;
        }
    }
}


template <class S>
void HeavyChanger<S>::Reset() {
    old_sk->Reset();
    S* temp = old_sk;
    old_sk = cur_sk;
    cur_sk = temp;
}

template <class S>
S* HeavyChanger<S>::GetCurSketch() {
    return cur_sk;
}

template <class S>
S* HeavyChanger<S>::GetOldSketch() {
    return old_sk;
}

enum {
	DIR_METHOD_TRACE,
	DIR_METHOD_MAC,
	DIR_METHOD_PORT
};

struct globalopts {

	int dir_method;
	bool only_dir0;
	bool only_dir1;
	bool require_both;
	bool nat_hole;
	bool ignore_rfc1918;
	char* local_mac;
	uint8_t mac_bytes[6];


	int numfile;
	// Time cost
	double avrupdatetime;
	double avrquerytime9;
	double avria_TopkApp;  //intersection accuracy
	double avrjs_TopkApp;  //jaccard similarity
	double avrndcg_TopkApp;  //normalized discounted cumulative gain
	double avrreerTopkApp;  //relative error
	double avrquerytime10;
	double avria_TopkUser;
	double avrjs_TopkUser;
	double avrndcg_TopkUser;
	double avrreerTopkUser;

	double avrupdatetime_t;
	double avrquerytime9_t;
	double avria_TopkApp_t;
	double avrjs_TopkApp_t;
	double avrndcg_TopkApp_t;
	double avrreerTopkApp_t;
	double avrquerytime10_t;
	double avria_TopkUser_t;
	double avrjs_TopkUser_t;
	double avrndcg_TopkUser_t;
	double avrreerTopkUser_t;
	
	double avrreerTopkApp2;
	double avrreerTopkApp_t2;
	double avrreerTopkUser2;
	double avrreerTopkUser_t2;

	groundmap groundedgetmp;
	groundmap groundnodetmp;
	groundmap groundnodetmp2;
	
	int K = 10;
	int K2 = 10;

	//AppSketch and Improved TCM
	HeavyChanger<DMatrix>* heavychangerdm;
	HeavyChanger<SDMatrix>* heavychangersdm;

	val_tp sum;
};

#endif
