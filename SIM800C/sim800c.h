#ifndef __SIM800C_H__
#define __SIM800C_H__	 
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32开发板
//ATK-SIM800C GSM/GPRS模块驱动	  
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2016/4/1
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved	
//********************************************************************************
//无
//////////////////////////////////////////////////////////////////////////////////	

#define swap16(x) (x&0XFF)<<8|(x&0XFF00)>>8	//高低字节交换宏定义
 
extern u8 Scan_Wtime;

void Send_Message(u8 *num,u8 *Msg);
void sim_send_sms(u8*phonenumber,u8*msg);
void sim_at_response(u8 mode);	
u8* sim800c_check_cmd(u8 *str);
unsigned int sendCommand(char *Command, char *Response, unsigned long Timeout, unsigned char Retry);
u8 sim800c_wait_request(u8 *request ,u16 waittime);
u8 sim800c_chr2hex(u8 chr);
//u8 sim800c_hex2chr(u8 hex);
//void sim800c_unigbk_exchange(u8 *src,u8 *dst,u8 mode);
//void sim800c_load_keyboard(u16 x,u16 y,u8 **kbtbl);
//void sim800c_key_staset(u16 x,u16 y,u8 keyx,u8 sta);
//u8 sim800c_get_keynum(u16 x,u16 y);
//u8 sim800c_call_test(void);			 //拨号测试
//void sim800c_sms_read_test(void);	 //读短信测试
//void sim800c_sms_send_test(void);	 //发短信测试 
//void sim800c_sms_ui(u16 x,u16 y);	 //短信测试UI界面函数
u8 sim800c_sms_test(void);			 //短信测试
//void sim800c_GB_ui(void);            //BT、GPRSUI界面函数
//void sim800c_GB_test(void);          //BT、GPRS选择
//void sim800c_spp_ui(u16 x,u16 y);    //蓝牙测试UI界面函数
//u8 sim800c_spp_test(void);           //蓝牙spp测试
//u8 sim800c_spp_mode(u8 mode);        //连接模式选择
void sim800c_mtest_ui(u16 x,u16 y);	 //SIM800C GSM/GPRS主测试UI
//u8 sim800c_gsminfo_show(u16 x,u16 y);//显示GSM模块信息
void ntp_update(void);               //网络同步时间
void sim800c_test(void);			 //SIM800C主测试函数
u8 Send_Text_Sms(u8 wh);
#endif





