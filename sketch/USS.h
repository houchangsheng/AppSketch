#ifndef USS_H
#define USS_H

#include "MultiAbstract.h"
#include "StreamSummary.h"

template<typename DATA_TYPE>
class USS : public MultiAbstract<DATA_TYPE>{
public:

    typedef std::unordered_map<DATA_TYPE, COUNT_TYPE> HashMap;

    USS(uint32_t _MEMORY, std::string _name = "USS"){
        this->name = _name;
        std::cout << this->name << std::endl;

        summary = new StreamSummary<DATA_TYPE, COUNT_TYPE>(summary->Memory2Size(_MEMORY));
    }

    ~USS(){
        delete summary;
    }

    void Insert(const DATA_TYPE& item, const COUNT_TYPE& p_s){
        if(summary->mp->Lookup(item))
            summary->Add_Data(item, p_s);
        else{
            if(summary->isFull()){
                if(randomGenerator() % (summary->getMin() + p_s) < p_s)
                    summary->SS_Replace(item, p_s);
                else
                    summary->Add_Min(p_s);
            }
            else
                summary->New_Data(item, p_s);
        }
    }

    HashMap AllQuery(){
        return summary->AllQuery();
    }

private:
    StreamSummary<DATA_TYPE, COUNT_TYPE>* summary;
};

#endif
