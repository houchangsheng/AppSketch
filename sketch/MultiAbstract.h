#ifndef MULTIABSTRACT_H
#define MULTIABSTRACT_H

#include <unordered_map>

#include <string.h>

#include "Util.h"

template<typename DATA_TYPE>
class MultiAbstract{
public:

    struct Counter{
        DATA_TYPE ID;
        COUNT_TYPE count;
    };
    typedef std::unordered_map<DATA_TYPE, COUNT_TYPE> HashMap;

    MultiAbstract(){}
    virtual ~MultiAbstract(){};

    std::string name;

    virtual void Insert(const DATA_TYPE& item, const COUNT_TYPE& p_s) = 0;
    //virtual COUNT_TYPE Query(const DATA_TYPE& item) = 0;
    virtual HashMap AllQuery() = 0;
};

#endif
