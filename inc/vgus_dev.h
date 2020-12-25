/* 
 * File:   DUGS_MASTER_H.h
 * Author: apleilx
 * 
 */
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

#ifndef VUGS_MASTER_H
#define	VUGS_MASTER_H

#include "mbdef.h"

#ifdef	__cplusplus
extern "C" {
#endif

/*所有配置接口兼容mbmaster
 *屏幕配置
 *开启crc，启用crc应答
 *禁用自动上传到串口
 
 * 指令支持
 * FUN_GUS_REG_WRITE
 * FUN_GUS_REG_READ
 * FUN_GUS_PV_WRITE
 * FUN_GUS_PV_READ
 * FUN_GUS_LINE_WRITE  ，写入的曲线数据后续在屏幕原曲线数据的后面
 * FUN_GUS_AUTO
*/

//主机处理 循环中调用
void VGUS_poll(mb_obj_type *mbObj);            

#define VGUS_RGE_LED         0x01
#define VGUS_RGE_BUZZER      0x02
#define VGUS_RGE_PAGE        0x03
#define VGUS_RGE_RUN_TIME    0x0C

#define VGUS_RGE_RTC_ADJ     0x1F
#define VGUS_RGE_RTC_NOW     0x20

#define VGUS_REG_MUSIC_PLAY       0x50
#define VGUS_REG_MUSIC_NUM        0x51
#define VGUS_REG_VOICE_ADJ        0x53
#define VGUS_REG_VOICE_VAL        0x54

#define VGUS_REG_AVI         0x60


//应用层接口
void vgus_page_set(mb_obj_type *mbObj, uint16_t *page, void (*CallBack)(uint8_t,uint8_t));
void vgus_pv_read(mb_obj_type *mbObj, uint16_t *dat, uint16_t addr, uint16_t Len , void (*CallBack)(uint8_t,uint8_t));
void vgus_pv_write(mb_obj_type *mbObj, uint16_t *dat, uint16_t addr, uint16_t Len , void (*CallBack)(uint8_t,uint8_t));

#ifdef	__cplusplus
}
#endif

#endif	/* VUGS_MASTER_H */

