#include<iostream>
#include<string>
#include<stdlib.h>

class BitMap{
private:
	char *bitmap;
	int gsize;
public:
	/*BitMap(){
		gsize=(10000>>3)+1;//default 10000
		bitmap= new char[gsize];
		memset(bitmap,0,sizeof(bitmap));
	}*/

	BitMap(int n){
		gsize=(n>>3)+1;
		bitmap=new char[gsize];
		memset(bitmap,0,sizeof(char)*gsize);
	}

	~BitMap(){delete []bitmap;}
	
	bool get(int x){
		int cur=x>>3;
		int red=x&7;
		if(cur>gsize)return -1;
		return ((bitmap[cur]&1<<red)>0); 
		//return ((bitmap[cur]&1>>red)>0); 
	}

	bool set(int x){
		int cur=x>>3;//获取元素位置,除8得到哪个元素，x/2^3得到那一个byte 
		int red=x&(7);//逻辑与，获取进准位置,x&7==x%8.该Byte里第几个 
		if(cur>gsize)return 0;
		//bitmap[cur]|=1>>red;//赋值，1向右移动red位，|表示该位赋值1
		bitmap[cur]|=1<<red;
		return 1; 
	}

	void clear(){
		memset(bitmap,0,sizeof(char)*gsize);
	}

	int get_gsize(){
		return gsize;
	}

	char* get_bitmap(){
		return bitmap;
	}

	bool is_in(BitMap* temp){
		bool flag = true;
		char *bm_tmp;
		bm_tmp = temp->get_bitmap();
		for (int i = 0; i < gsize; i++) {
			if ((bm_tmp[i] & bitmap[i]) != bitmap[i]) {
				flag = false;
				/*std::cout << "block " << i << ":  ";
				for (int j=7;j>=0;j--)  
				{
					std::cout << ((bm_tmp[i] >> j) & 1);    
				}
				std::cout << "  &  ";
				for (int j=7;j>=0;j--)  
				{
					std::cout << ((bitmap[i] >> j) & 1);    
				}
				std::cout << "  =  ";
				for (int j=7;j>=0;j--)  
				{
					std::cout << (((bm_tmp[i] & bitmap[i]) >> j) & 1);    
				}
				std::cout << std::endl;*/
			}
		}
		return flag;
	}

	bool equal_to(BitMap* temp){
		bool flag = true;
		char *bm_tmp;
		bm_tmp = temp->get_bitmap();
		for (int i = 0; i < gsize; i++) {
			if ((bm_tmp[i] ^ bitmap[i]) != 0) {
				flag = false;
			}
		}
		return flag;
	}

	void add(BitMap* temp){
		char *bm_tmp;
		bm_tmp = temp->get_bitmap();
		for (int i = 0; i < gsize; i++) {
			bitmap[i] = bitmap[i] | bm_tmp[i];
		}
	}

	void c_add(BitMap* temp){
		memset(bitmap,0,sizeof(char)*gsize);
		char *bm_tmp;
		bm_tmp = temp->get_bitmap();
		for (int i = 0; i < gsize; i++) {
			bitmap[i] = bitmap[i] | bm_tmp[i];
		}
	}
};
