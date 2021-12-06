#include "sketch2.hpp"

SDMatrix::SDMatrix(int depth, int length, int width, int lgn, int K, int K2) {

    sdm_.depth = depth;
    sdm_.length = length;
    sdm_.width = width;
    sdm_.lgn = lgn;
    sdm_.total_sum = 0;
    sdm_.counts = new SBucket *[depth*length*width];
    for (int i = 0; i < depth*length*width; i++) {
        sdm_.counts[i] = (SBucket*)calloc(1, sizeof(SBucket));
        memset(sdm_.counts[i], 0, sizeof(SBucket));
        //sdm_.counts[i]->key[0] = '\0';
    }

    sdm_.K_App = K;
    sdm_.K_User = K2;
    sdm_.tka = new TkList[2*sdm_.K_App];
    memset(sdm_.tka, 0, sizeof(TkList)*2*sdm_.K_App);
    /*for (int i = 0; i < 2*sdm_.K_App; i++) {
	sdm_.tka[i].node_bit = new BitMap(sdm_.width*sdm_.depth);
    }*/
    sdm_.tku = new TkList[2*sdm_.K_User];
    memset(sdm_.tku, 0, sizeof(TkList)*2*sdm_.K_User);
    /*for (int i = 0; i < 2*sdm_.K_User; i++) {
	sdm_.tku[i].node_bit = new BitMap(sdm_.length*sdm_.depth);
    }*/
    sdm_.bm_a = new BitMap(width*depth);
    sdm_.bm_u = new BitMap(length*depth);
    sdm_.topk_threshold = 10000;

    sdm_.hash = new unsigned long[depth];
    sdm_.scale = new unsigned long[depth];
    sdm_.hardner = new unsigned long[depth];
    char name[] = "SDMatrix";
    unsigned long seed = AwareHash((unsigned char*)name, strlen(name), 13091204281, 228204732751, 6620830889);

    unsigned long seed1 = seed;
    unsigned long seed2 = seed + 100;
    unsigned long seed3 = seed + 200;

    /*for (int i = 0; i < depth; i++) {
        sdm_.hash[i] = GenHashSeed(seed++);
    }
    for (int i = 0; i < depth; i++) {
        sdm_.scale[i] = GenHashSeed(seed++);
    }
    for (int i = 0; i < depth; i++) {
        sdm_.hardner[i] = GenHashSeed(seed++);
    }*/

    for (int i = 0; i < depth; i++) {
        sdm_.hash[i] = GenHashSeed(seed1++);
    }
    for (int i = 0; i < depth; i++) {
        sdm_.scale[i] = GenHashSeed(seed2++);
    }
    for (int i = 0; i < depth; i++) {
        sdm_.hardner[i] = GenHashSeed(seed3++);
    }
}

SDMatrix::~SDMatrix() {
    for (int i = 0; i < sdm_.depth*sdm_.width*sdm_.length; i++) {
        free(sdm_.counts[i]);
    }
    delete [] sdm_.hash;
    delete [] sdm_.scale;
    delete [] sdm_.hardner;
    delete [] sdm_.counts;

    /*for (int i = 0; i < 2*sdm_.K_App; i++) {
	delete sdm_.tka[i].node_bit;
    }
    for (int i = 0; i < 2*sdm_.K_User; i++) {
	delete sdm_.tku[i].node_bit;
    }*/
    delete [] sdm_.tka;
    delete [] sdm_.tku;
    delete sdm_.bm_a;
    delete sdm_.bm_u;
}

/*void SDMatrix::Update(uint32_t * key, val_tp val) {
    sdm_.total_sum += val;
    unsigned long row = 0;
    unsigned long col = 0;
    int keylen = sdm_.lgn;

    for (int i = 0; i < sdm_.depth; i++) {
        row = MurmurHash64A(key, keylen/2, sdm_.hardner[i]) % sdm_.width;
        col = MurmurHash64A(key+1, keylen/2, sdm_.hardner[i]) % sdm_.length;
        int index = i*sdm_.width*sdm_.length + row*sdm_.length + col;
        SDMatrix::SBucket *sbucket = sdm_.counts[index];
        sbucket->sum += val;
    }

    val_tp app_node = Querynodeweight(key);
    val_tp user_node = Querynodeweight_d(key);

    bool insert_flag = false;
    for (int i = 0; i < 2*sdm_.K_App; i++) {
	if (memcmp(key, &sdm_.tka[i].key[0], keylen/2) == 0) {
	    //sdm_.tka[i].sum = app_node;
	    sdm_.tka[i].sum += val;
	    insert_flag = true;
	    for (int j = i; j > 0; j--) {
		if (sdm_.tka[j].sum > sdm_.tka[j-1].sum) {
		    TkList temp;
		    temp.sum = sdm_.tka[j-1].sum;
		    temp.key[0] = sdm_.tka[j-1].key[0];
		    temp.key[1] = sdm_.tka[j-1].key[1];
		    sdm_.tka[j-1].sum = sdm_.tka[j].sum;
		    sdm_.tka[j-1].key[0] = sdm_.tka[j].key[0];
		    sdm_.tka[j-1].key[1] = sdm_.tka[j].key[1];
		    sdm_.tka[j].sum = temp.sum;
		    sdm_.tka[j].key[0] = temp.key[0];
		    sdm_.tka[j].key[1] = temp.key[1];
		}
		else {
		    break;
		}
	    }
	    break;
	}
    }
    if (!insert_flag) {
	int i = 2*sdm_.K_App - 1;
	if (app_node > sdm_.tka[i].sum) {
	    while(app_node > sdm_.tka[i-1].sum) {
		sdm_.tka[i].sum = sdm_.tka[i-1].sum;
		sdm_.tka[i].key[0] = sdm_.tka[i-1].key[0];
		sdm_.tka[i].key[1] = sdm_.tka[i-1].key[1];
		i--;
		if (i == 0) {
		    break;
		}
	    }
	    sdm_.tka[i].sum = app_node;
	    memcpy(&sdm_.tka[i].key[0], key, keylen/2);
	}
    }

    insert_flag = false;
    for (int i = 0; i < 2*sdm_.K_User; i++) {
	if (memcmp(key+1, &sdm_.tku[i].key[1], keylen/2) == 0) {
	    //sdm_.tku[i].sum = user_node;
	    sdm_.tku[i].sum += val;
	    insert_flag = true;
	    for (int j = i; j > 0; j--) {
		if (sdm_.tku[j].sum > sdm_.tku[j-1].sum) {
		    TkList temp;
		    temp.sum = sdm_.tku[j-1].sum;
		    temp.key[0] = sdm_.tku[j-1].key[0];
		    temp.key[1] = sdm_.tku[j-1].key[1];
		    sdm_.tku[j-1].sum = sdm_.tku[j].sum;
		    sdm_.tku[j-1].key[0] = sdm_.tku[j].key[0];
		    sdm_.tku[j-1].key[1] = sdm_.tku[j].key[1];
		    sdm_.tku[j].sum = temp.sum;
		    sdm_.tku[j].key[0] = temp.key[0];
		    sdm_.tku[j].key[1] = temp.key[1];
		}
		else {
		    break;
		}
	    }
	    break;
	}
    }
    if (!insert_flag) {
	int i = 2*sdm_.K_User - 1;
	if (user_node > sdm_.tku[i].sum) {
	    while(user_node > sdm_.tku[i-1].sum) {
		sdm_.tku[i].sum = sdm_.tku[i-1].sum;
		sdm_.tku[i].key[0] = sdm_.tku[i-1].key[0];
		sdm_.tku[i].key[1] = sdm_.tku[i-1].key[1];
		i--;
		if (i == 0) {
		    break;
		}
	    }
	    sdm_.tku[i].sum = user_node;
	    memcpy(&sdm_.tku[i].key[1], key+1, keylen/2);
	}
    }
}*/

void SDMatrix::Update(uint32_t * key, val_tp val) {
    sdm_.total_sum += val;
    unsigned long row = 0;
    unsigned long col = 0;
    int keylen = sdm_.lgn;

    for (int i = 0; i < sdm_.depth; i++) {
        row = MurmurHash64A(key, keylen/2, sdm_.hardner[i]) % sdm_.width;
        col = MurmurHash64A(key+1, keylen/2, sdm_.hardner[i]) % sdm_.length;
        int index = i*sdm_.width*sdm_.length + row*sdm_.length + col;
        SDMatrix::SBucket *sbucket = sdm_.counts[index];
        sbucket->sum += val;
    }

    val_tp app_node = Querynodeweight(key);
    val_tp user_node = Querynodeweight_d(key);

    bool insert_flag = false;
    for (int i = 0; i < 2*sdm_.K_App; i++) {
	if (memcmp(key, &sdm_.tka[i].key[0], keylen/2) == 0) {
	    sdm_.tka[i].sum = app_node;
	    insert_flag = true;
	    for (int j = i; j > 0; j--) {
		if (sdm_.tka[j].sum > sdm_.tka[j-1].sum) {
		    TkList temp;
		    temp.sum = sdm_.tka[j-1].sum;
		    temp.key[0] = sdm_.tka[j-1].key[0];
		    temp.key[1] = sdm_.tka[j-1].key[1];
		    sdm_.tka[j-1].sum = sdm_.tka[j].sum;
		    sdm_.tka[j-1].key[0] = sdm_.tka[j].key[0];
		    sdm_.tka[j-1].key[1] = sdm_.tka[j].key[1];
		    sdm_.tka[j].sum = temp.sum;
		    sdm_.tka[j].key[0] = temp.key[0];
		    sdm_.tka[j].key[1] = temp.key[1];
		}
		else {
		    break;
		}
	    }
	    break;
	}
    }
    if (!insert_flag) {
	int i = 2*sdm_.K_App - 1;
	if (app_node > sdm_.tka[i].sum) {
	    while(app_node > sdm_.tka[i-1].sum) {
		sdm_.tka[i].sum = sdm_.tka[i-1].sum;
		sdm_.tka[i].key[0] = sdm_.tka[i-1].key[0];
		sdm_.tka[i].key[1] = sdm_.tka[i-1].key[1];
		i--;
		if (i == 0) {
		    break;
		}
	    }
	    sdm_.tka[i].sum = app_node;
	    memcpy(&sdm_.tka[i].key[0], key, keylen/2);
	}
    }

    insert_flag = false;
    for (int i = 0; i < 2*sdm_.K_User; i++) {
	if (memcmp(key+1, &sdm_.tku[i].key[1], keylen/2) == 0) {
	    sdm_.tku[i].sum = user_node;
	    insert_flag = true;
	    for (int j = i; j > 0; j--) {
		if (sdm_.tku[j].sum > sdm_.tku[j-1].sum) {
		    TkList temp;
		    temp.sum = sdm_.tku[j-1].sum;
		    temp.key[0] = sdm_.tku[j-1].key[0];
		    temp.key[1] = sdm_.tku[j-1].key[1];
		    sdm_.tku[j-1].sum = sdm_.tku[j].sum;
		    sdm_.tku[j-1].key[0] = sdm_.tku[j].key[0];
		    sdm_.tku[j-1].key[1] = sdm_.tku[j].key[1];
		    sdm_.tku[j].sum = temp.sum;
		    sdm_.tku[j].key[0] = temp.key[0];
		    sdm_.tku[j].key[1] = temp.key[1];
		}
		else {
		    break;
		}
	    }
	    break;
	}
    }
    if (!insert_flag) {
	int i = 2*sdm_.K_User - 1;
	if (user_node > sdm_.tku[i].sum) {
	    while(user_node > sdm_.tku[i-1].sum) {
		sdm_.tku[i].sum = sdm_.tku[i-1].sum;
		sdm_.tku[i].key[0] = sdm_.tku[i-1].key[0];
		sdm_.tku[i].key[1] = sdm_.tku[i-1].key[1];
		i--;
		if (i == 0) {
		    break;
		}
	    }
	    sdm_.tku[i].sum = user_node;
	    memcpy(&sdm_.tku[i].key[1], key+1, keylen/2);
	}
    }
}

// Query on edge weight
val_tp SDMatrix::Queryedgeweight(uint32_t* key) {
    val_tp ret = 0;
    for (int j = 0; j < sdm_.depth; j++) {
        unsigned long row = MurmurHash64A(key, sdm_.lgn/2, sdm_.hardner[j]) % sdm_.width;
        unsigned long col = MurmurHash64A(key+1, sdm_.lgn/2, sdm_.hardner[j]) % sdm_.length;
        int index = j*sdm_.width*sdm_.length + row*sdm_.length + col;
        val_tp upest = 0;
        upest = sdm_.counts[index]->sum;
        if (j == 0) ret = upest;
        else ret = std::min(ret, upest);
    }
    return ret;
}

// Query on node weight (src)
val_tp SDMatrix::Querynodeweight(uint32_t* key) {
    val_tp ret = 0;
    for (int j = 0; j < sdm_.depth; j++) {
        unsigned long row = MurmurHash64A(key, sdm_.lgn/2, sdm_.hardner[j]) % sdm_.width;
        val_tp value = 0;
        for (int i = 0; i < sdm_.length; i++){
            int index = j*sdm_.width*sdm_.length + row*sdm_.length + i;
            value += sdm_.counts[index]->sum;
        }
        if (j == 0) ret = value;
        else ret = std::min(ret, value);
    }
    return ret;
}

val_tp SDMatrix::Querynodeweight_d(uint32_t* key) {
    val_tp ret = 0;
    for (int j = 0; j < sdm_.depth; j++) {
	unsigned long col = MurmurHash64A(key+1, sdm_.lgn/2, sdm_.hardner[j]) % sdm_.length;
        val_tp value = 0;
        for (int i = 0; i < sdm_.width; i++){
            int index = j*sdm_.width*sdm_.length + i*sdm_.length + col;
            value += sdm_.counts[index]->sum;
        }
        if (j == 0) ret = value;
        else ret = std::min(ret, value);
    }
    return ret;
}

void SDMatrix::QueryTopkApps(int K, std::pair<key_tp, val_tp> *es_topk_apps) {
    /*for (int i = 0; i < 2*K; i++) {
	std::cout << sdm_.tka[i].key[0] << ": " << sdm_.tka[i].sum << std::endl;
    }*/
    for (int i = 0; i < K; i++) {
	if (sdm_.tka[i].sum != 0) {
	    key_tp node;
	    node.key[0] = sdm_.tka[i].key[0];
	    node.key[1] = sdm_.tka[i].key[1];
	    val_tp node_value = sdm_.tka[i].sum;
	    std::pair<key_tp, val_tp> temp = std::make_pair(node, node_value);
	    es_topk_apps[i] = temp;
	}
	else {
	    break;
	}
    }
}

void SDMatrix::QueryTopkUsers(int K2, std::pair<key_tp, val_tp> *es_topk_users) {
    /*for (int i = 0; i < 2*K2; i++) {
	std::cout << sdm_.tku[i].key[1] << ": " << sdm_.tku[i].sum << std::endl;
    }*/
    for (int i = 0; i < K2; i++) {
	if (sdm_.tku[i].sum != 0) {
	    key_tp node;
	    node.key[0] = sdm_.tku[i].key[0];
	    node.key[1] = sdm_.tku[i].key[1];
	    val_tp node_value = sdm_.tku[i].sum;
	    std::pair<key_tp, val_tp> temp = std::make_pair(node, node_value);
	    es_topk_users[i] = temp;
	}
	else {
	    break;
	}
    }
}

val_tp SDMatrix::Change(SDMatrix** sdm_arr){
    SDMatrix* oldsk = sdm_arr[0];
    SDMatrix* cursk = sdm_arr[1];
    int depth = cursk->sdm_.depth, length = cursk->sdm_.length, width = cursk->sdm_.width;
    SDMatrix::SBucket** oldtable = (oldsk)->GetTable();
    SDMatrix::SBucket** curtable = (cursk)->GetTable();
    val_tp diff[depth], maxdiff=0, difftemp=0;
    for (int j = 0; j < depth; j++) {
        diff[j] = 0;
        for (int i = j*length*width; i < length*width*(j+1); i++) {
            difftemp = curtable[i]->sum > oldtable[i]->sum ? curtable[i]->sum - oldtable[i]->sum : oldtable[i]->sum - curtable[i]->sum;
            diff[j] += difftemp;
    }
    maxdiff = std::max(maxdiff, diff[j]);
    }
    return maxdiff;
}

val_tp SDMatrix::GetCount() {
    return sdm_.total_sum;
}

void SDMatrix::Reset() {
    sdm_.total_sum=0;
    for (int i = 0; i < sdm_.depth*sdm_.length*sdm_.width; i++) {
        memset(sdm_.counts[i], 0, sizeof(SBucket));
    }

    memset(sdm_.tka, 0, sizeof(TkList)*2*sdm_.K_App);
    memset(sdm_.tku, 0, sizeof(TkList)*2*sdm_.K_User);
    sdm_.bm_a->clear();
    sdm_.bm_u->clear();
    sdm_.topk_threshold = 10000;
}

SDMatrix::SBucket** SDMatrix::GetTable() {
    return sdm_.counts;
}

