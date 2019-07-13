#include "hc-sr04.h"
#include "delay.h"
/*
 * 函数名：UltrasonicWave_Configuration
 * 描述  ：超声波模块的初始化。 初始化PC1(trig)
 * 输入  ：无
 * 输出  ：无	
 */
void UltrasonicWave_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	 //使能PC端口时钟
	
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;				 //PC1 端口配置
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	 GPIO_Init(GPIOC, &GPIO_InitStructure);					 //根据设定参数初始化GPIOC.1

	
//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;					 //PC1接TRIG
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		     //设为推挽输出模式
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	         
//	GPIO_Init(GPIOC, &GPIO_InitStructure);	                 //初始化外设GPIO 

}



/*
 * 函数名：UltrasonicWave_StartMeasure
 * 描述  ：开始测距，发送一个>10us的脉冲，然后测量返回的高电平时间
 * 输入  ：无
 * 输出  ：无	
 */
void UltrasonicWave_StartMeasure(void)
{
	Trig = 1;
	delay_us(20);		                      //延时20US
	Trig = 0;
	
}
