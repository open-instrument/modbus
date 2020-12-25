#ifndef _MB_COM_H
#define _MB_COM_H

#include "mbdef.h"
void mb_package_send( mb_obj_type * mbObj);

//以下函数可以在外部实现以提高效率
void mb_rtu_timer_run(mb_obj_type *mbObj); //1ms中断中调用

//使用硬件定时器时不调用这两个函数，有外部重新实现为硬件定时
void mb_timer_en(mb_obj_type *mbObj); //接收中断调用
void mb_timer_dis(mb_obj_type *mbObj);

void mb_obj_init(mb_obj_type *mbObj);
void mb_poll(mb_obj_type *mbObj);

//长按执行一次的操作， check_val_now为被监控变量  plse提供计数脉冲
//超时释放计数值6     长按动作计数15
void  mb_gus_key_opt(mb_gus_keep_key_t *key, uint8_t check_val_now, uint8_t b_pulse);

#endif
