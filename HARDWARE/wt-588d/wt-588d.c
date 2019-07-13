#include "sys.h"
#include "wt-588d.H"
#include "delay.h"

//初始化PA8为输出口.并使能这个口的时钟		    
//LED IO初始化

void WT_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	 //使能PA端口时钟
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;				 //LED0-->PA.8 端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 GPIO_Init(GPIOA, &GPIO_InitStructure);					 //根据设定参数初始化GPIOA.8
 GPIO_SetBits(GPIOA,GPIO_Pin_12);						 //PA.8 输出高

}

/*-------------------------------------- 
;模块名称:Line_1A(void)
;功    能:实现一线单字节低位在前串口通信函数
;入    参:无
;出    参:0	   返回主函数
;-------------------------------------*/
void Line_1A(unsigned char dat)
{
	unsigned char i;
	u8 B_DATA;
//********************************************************

	P_DATA = 0;

	delay_ms(5);   				//拉低至少5ms

	B_DATA = dat&0X01;
	for(i=0;i<8;i++)
	{
		P_DATA = 1;				//拉高数据传输线，准备传输数据
		if(B_DATA==0)
		{	/*表示逻辑电平0*/
			delay_us(200);     // 延时200us
			P_DATA = 0;
			delay_us(600);	    // 延时600us    
		}
		else
		{  /*表示逻辑电平1*/
			delay_us(600);
			P_DATA = 0;
			delay_us(200);
		}
		dat = dat>>1;
		B_DATA = dat&0X01;
	}
	P_DATA = 1;					//拉高电平
}



/*-------------------------------------- 
;模块名称:void Data_Pro(int m)
;功    能:实现数据处理和语音输出
;入    参:距离
;出    参:无	   返回主函数
:备    注:程序有待优化！
;-------------------------------------*/
void Data_Pro(int m)
{
	u8 b, s, g;		//百位，十位， 个位
	
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
//	Line_1A(12);//加这里会出错
}

void Dwon_Pro(void)
{
//	while(1)
		Line_1A(14);
	delay_ms(2000);
	Line_1A(14);
		delay_ms(2000);

}


