#ifndef __WT588D_H
#define __WT588D_H
#include "sys.h"


//LED端口定义
#define P_DATA PAout(12)	// DS0

void WT_Init(void);//初始化		 				    
void Line_1A(unsigned char dat);
void Data_Pro(int m);
void Dwon_Pro(void);

#endif

