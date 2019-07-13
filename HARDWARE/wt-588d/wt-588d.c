#include "sys.h"
#include "wt-588d.H"
#include "delay.h"

//��ʼ��PA8Ϊ�����.��ʹ������ڵ�ʱ��		    
//LED IO��ʼ��

void WT_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	 //ʹ��PA�˿�ʱ��
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;				 //LED0-->PA.8 �˿�����
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
 GPIO_Init(GPIOA, &GPIO_InitStructure);					 //�����趨������ʼ��GPIOA.8
 GPIO_SetBits(GPIOA,GPIO_Pin_12);						 //PA.8 �����

}

/*-------------------------------------- 
;ģ������:Line_1A(void)
;��    ��:ʵ��һ�ߵ��ֽڵ�λ��ǰ����ͨ�ź���
;��    ��:��
;��    ��:0	   ����������
;-------------------------------------*/
void Line_1A(unsigned char dat)
{
	unsigned char i;
	u8 B_DATA;
//********************************************************

	P_DATA = 0;

	delay_ms(5);   				//��������5ms

	B_DATA = dat&0X01;
	for(i=0;i<8;i++)
	{
		P_DATA = 1;				//�������ݴ����ߣ�׼����������
		if(B_DATA==0)
		{	/*��ʾ�߼���ƽ0*/
			delay_us(200);     // ��ʱ200us
			P_DATA = 0;
			delay_us(600);	    // ��ʱ600us    
		}
		else
		{  /*��ʾ�߼���ƽ1*/
			delay_us(600);
			P_DATA = 0;
			delay_us(200);
		}
		dat = dat>>1;
		B_DATA = dat&0X01;
	}
	P_DATA = 1;					//���ߵ�ƽ
}



/*-------------------------------------- 
;ģ������:void Data_Pro(int m)
;��    ��:ʵ�����ݴ�����������
;��    ��:����
;��    ��:��	   ����������
:��    ע:�����д��Ż���
;-------------------------------------*/
void Data_Pro(int m)
{
	u8 b, s, g;		//��λ��ʮλ�� ��λ
	
	b = (m%1000)/100;
	s = ((m%1000)%100)/10;
	g = ((m%1000)%100)%10;
	
	Line_1A(13);
	delay_ms(1500);
	if(m>=0&&m<10)
	{
		Line_1A(g);
		delay_ms(500);
		Line_1A(12);
	}
	else if(m>=10&&m<100)
	{
		Line_1A(s);
		delay_ms(500);
		Line_1A(10);
		delay_ms(500);
		Line_1A(g);
		delay_ms(500);
		Line_1A(12);
	}
	else if(m>=100&&m<1000)
	{
		Line_1A(b);
		delay_ms(500);
		Line_1A(11);
		delay_ms(500);
		Line_1A(s);
		delay_ms(500);
		Line_1A(10);
		delay_ms(500);
		Line_1A(g);
		delay_ms(500);
		Line_1A(12);
	}
	else return;
//	Line_1A(12);//����������
}

void Dwon_Pro(void)
{
//	while(1)
		Line_1A(14);
	delay_ms(2000);
	Line_1A(14);
		delay_ms(2000);

}


