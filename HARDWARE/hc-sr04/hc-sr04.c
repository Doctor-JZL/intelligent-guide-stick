#include "hc-sr04.h"
#include "delay.h"
/*
 * ��������UltrasonicWave_Configuration
 * ����  ��������ģ��ĳ�ʼ���� ��ʼ��PC1(trig)
 * ����  ����
 * ���  ����	
 */
void UltrasonicWave_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	 //ʹ��PC�˿�ʱ��
	
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;				 //PC1 �˿�����
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
	 GPIO_Init(GPIOC, &GPIO_InitStructure);					 //�����趨������ʼ��GPIOC.1

	
//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;					 //PC1��TRIG
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		     //��Ϊ�������ģʽ
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	         
//	GPIO_Init(GPIOC, &GPIO_InitStructure);	                 //��ʼ������GPIO 

}



/*
 * ��������UltrasonicWave_StartMeasure
 * ����  ����ʼ��࣬����һ��>10us�����壬Ȼ��������صĸߵ�ƽʱ��
 * ����  ����
 * ���  ����	
 */
void UltrasonicWave_StartMeasure(void)
{
	Trig = 1;
	delay_us(20);		                      //��ʱ20US
	Trig = 0;
	
}
