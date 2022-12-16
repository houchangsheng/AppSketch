#ifndef SS_H
#define SS_H

#include "MultiAbstract.h"
#include "StreamSummary.h"

template<typename DATA_TYPE>
class SpaceSaving : public MultiAbstract<DATA_TYPE>{
public:
    typedef std::unordered_map<DATA_TYPE, COUNT_TYPE> HashMap;

    SpaceSaving(uint32_t _MEMORY, std::string _name = "SpaceSaving"){
        this->name = _name;

        summary = new StreamSummary<DATA_TYPE, COUNT_TYPE>(summary->Memory2Size(_MEMORY));
    }

    ~SpaceSaving(){
        delete summary;
    }

    void Insert(const DATA_TYPE& item, const COUNT_TYPE& p_s){
        if(summary->mp->Lookup(item))
            summary->Add_Data(item, p_s);
        else{
            if(summary->isFull())
                summary->SS_Replace(item, p_s);
            else
                summary->New_Data(item, p_s);
        }
    }

    COUNT_TYPE Query(const DATA_TYPE& item){
        COUNT_TYPE temp = summary->Query(item);
        if(temp > 0)
            return temp;
        else
            return summary->getMin() - 1;
    }

    COUNT_TYPE Query_t(const DATA_TYPE& item){
        COUNT_TYPE temp = summary->Query(item);
        return temp;
    }

    HashMap AllQuery(){
        return summary->AllQuery();
    }

private:
    StreamSummary<DATA_TYPE, COUNT_TYPE>* summary;
};

#endif
