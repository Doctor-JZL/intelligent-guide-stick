#ifndef __WT588D_H
#define __WT588D_H
#include "sys.h"


//LED�˿ڶ���
#define P_DATA PAout(12)	// DS0

void WT_Init(void);//��ʼ��		 				    
void Line_1A(unsigned char dat);
void Data_Pro(int m);
void Dwon_Pro(void);

#endif

