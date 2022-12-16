#ifndef ADAPTOR_H
#define ADAPTOR_H
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string.h>
#include "Util.h"

class Adaptor {

    typedef struct {
        unsigned char* databuffer;
        uint64_t cnt=0;
        uint64_t cur=0;
        unsigned char* ptr;
    } adaptor_t;

    public:
        Adaptor(std::string filename, uint64_t buffersize);

        ~Adaptor();

        int GetNext(tuple_t* t);

        void Reset();

        uint64_t GetDataSize();

        adaptor_t* data;

};

inline int IPToValue(const std::string& strIP)
{
        int a[4];
        std::string IP = strIP;
        std::string strTemp;
        size_t pos;
        size_t i=3;

        do
        {
            pos = IP.find("."); 
	
            if(pos != std::string::npos)
            {		
                strTemp = IP.substr(0,pos);	
                a[i] = atoi(strTemp.c_str());		
                i--;		
                IP.erase(0,pos+1);
            }
            else
            {					
                strTemp = IP;
                a[i] = atoi(strTemp.c_str());
                break;
            }
        }while(1);
 
        int nResult = (a[3]<<24) + (a[2]<<16)+ (a[1]<<8) + a[0];
        return nResult;
}

inline std::string ValueToIP(const int& nValue)
{
        char strTemp[20];
        sprintf( strTemp,"%d.%d.%d.%d", (nValue&0xff000000)>>24, (nValue&0x00ff0000)>>16, (nValue&0x0000ff00)>>8, (nValue&0x000000ff) );

        return std::string(strTemp);
}
#endif
