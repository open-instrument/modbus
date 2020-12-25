/* 
 * DGUSII master mode handdle.
 * 未验证
 */
/*-------------------------引用FreeModbus 的文件-------------------------------*/
#include "mbmaster.h"
#include "vgus_dev.h"
#include "mbcom.h"

extern uint32_t mbMcuCheck(void);

static void VGUS_appSendCmd(mb_obj_type *mbObj);
extern void mbappCmdSearchNext(mb_obj_type *mbObj);

/*****************************************************************************/ /*!
* @brief   adu crc and send .
*               
* @param   none
*
* @return  none
*
* @ Pass/ Fail criteria: none
*****************************************************************************/

void VGUS_PackageSend(mb_obj_type *mbObj)
{
    mb_data_type Data;
    Data.Val = mb_crc(&mbObj->AduBuff[3], mbObj->SndSize - 3); //从指令开始
    mbObj->AduBuff[mbObj->SndSize++] = Data.byte.LB;
    mbObj->AduBuff[mbObj->SndSize++] = Data.byte.HB;
    mbObj->SndSt = MB_TX_XMIT;
    mbObj->SndCnt = 0;
    mbObj->AduSend();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/////////////                      功能码处理                       ////////////
////////////////////////////////////////////////////////////////////////////////

/*----------------------------------------------------------------------------*/
/*----------------------------读多个寄存器(0x81)-------------------------------*/
/*----------------------------------------------------------------------------
*                  按字节编地址，按字节计数
 * ---------------------------------------------------------------------------*/
uint8_t VGUS_ReadReg(mb_obj_type *mbObj, uint16_t DataAdd, uint16_t Len)
{

    if ((Len > 112) || (Len == 0) || ((0xFFFF - DataAdd) < (Len - 1)))
        return MB_ERR_NOREG;

    mbObj->Fun.Master.LenReq = Len;

    mbObj->AduBuff[0] = 0x5A;
    mbObj->AduBuff[1] = 0xA5;
    mbObj->AduBuff[2] = 1 + 2 + 2; //包括指令、数据、校验
    mbObj->AduBuff[3] = 0x81;
    mbObj->AduBuff[4] = DataAdd;
    mbObj->AduBuff[5] = Len;

    mbObj->SndSize = 6;

    VGUS_PackageSend(mbObj);

    return MB_ERR_NONE;
}

/*----------------------------------------------------------------------------*/
/*-----------------------------写多个寄存器(0x80)------------------------------*/

/*----------------------------------------------------------------------------
 *    按字节编地址，按字节计数
 *    DataAdd:寄存器地址，
 *    Len:写入的字节数
 * ---------------------------------------------------------------------------
 * 
 * ---------------------------------------------------------------------------*/
uint8_t VGUS_WriteReg(mb_obj_type *mbObj, uint16_t DataAdd, uint16_t Len)
{
    uint16_t DataIndex;
    uint8_t *Src;

    if ((Len > 112) || (Len == 0) || ((0xFFFF - DataAdd) < (Len - 1)))
        return MB_ERR_NOREG;

    mbObj->AduBuff[0] = 0x5A;
    mbObj->AduBuff[1] = 0xA5;
    mbObj->AduBuff[2] = 1 + 1 + Len + 2; //包括指令、数据、校验
    mbObj->AduBuff[3] = 0x80;
    mbObj->AduBuff[4] = DataAdd;

    DataIndex = 5;
    Src = (uint8_t *)mbObj->Fun.Master.pData;
    while (Len--)
    {
        mbObj->AduBuff[DataIndex++] = *Src++;
    }

    mbObj->SndSize = DataIndex;

    VGUS_PackageSend(mbObj);

    return MB_ERR_NONE;
}

/*----------------------------------------------------------------------------*/
/*----------------------------读多个寄存器(0x83)-------------------------------*/
/*----------------------------------------------------------------------------
*                  按字编地址，按字计数
 * ---------------------------------------------------------------------------*/
uint8_t VGUS_ReadPV(mb_obj_type *mbObj, uint16_t DataAdd, uint16_t Len)
{
    mb_data_type WToB;

    if ((Len > 112) || (Len == 0) || ((0xFFFF - DataAdd) < (Len - 1)))
        return MB_ERR_NOREG;

    mbObj->Fun.Master.LenReq = Len;

    mbObj->AduBuff[0] = 0x5A;
    mbObj->AduBuff[1] = 0xA5;
    mbObj->AduBuff[2] = 1 + 3 + 2; //包括指令、数据、校验
    mbObj->AduBuff[3] = 0x83;
    WToB.Val = DataAdd;
    mbObj->AduBuff[4] = WToB.byte.HB;
    mbObj->AduBuff[5] = WToB.byte.LB;

    mbObj->AduBuff[6] = Len;

    mbObj->SndSize = 7;

    VGUS_PackageSend(mbObj);

    return MB_ERR_NONE;
}

/*----------------------------------------------------------------------------*/
/*-----------------------------写多个寄存器(0x82)------------------------------*/

/*----------------------------------------------------------------------------
 *    按字节编地址，按字节计数
 *    DataAdd:变量地址，
 *    Len:写入的字数
 * ---------------------------------------------------------------------------
 * 
 * ---------------------------------------------------------------------------*/
uint8_t VGUS_WritePV(mb_obj_type *mbObj, uint16_t DataAdd, uint16_t Len)
{
    uint16_t DataIndex;
    mb_data_type WToB;
    uint16_t *Src;

    if ((Len > 112) || (Len == 0) || ((0xFFFF - DataAdd) < (Len - 1)))
        return MB_ERR_NOREG;

    mbObj->AduBuff[0] = 0x5A;
    mbObj->AduBuff[1] = 0xA5;
    mbObj->AduBuff[2] = 1 + 2 + Len * 2 + 2; //包括指令、数据、校验
    mbObj->AduBuff[3] = 0x82;
    WToB.Val = DataAdd;
    mbObj->AduBuff[4] = WToB.byte.HB;
    mbObj->AduBuff[5] = WToB.byte.LB;

    DataIndex = 6;
    Src = mbObj->Fun.Master.pData;
    while (Len--)
    {
        WToB.Val = *Src++;
        mbObj->AduBuff[DataIndex++] = WToB.byte.HB;
        mbObj->AduBuff[DataIndex++] = WToB.byte.LB;
    }

    mbObj->SndSize = DataIndex;

    VGUS_PackageSend(mbObj);

    return MB_ERR_NONE;
}

/*----------------------------------------------------------------------------*/
/*-----------------------------写曲线(0x84)------------------------------*/

/*----------------------------------------------------------------------------
 *    按字节编地址，按字节计数
 *    Mode:曲线通道，
 *    Len:写入的字数
 * ---------------------------------------------------------------------------
 * 
 * ---------------------------------------------------------------------------*/
uint8_t VGUS_WriteLine(mb_obj_type *mbObj, uint16_t Mode, uint16_t Len)
{
    uint16_t DataIndex;
    mb_data_type WToB;
    uint16_t *Src;

    if ((Len > 112) || (Len == 0) )
        return MB_ERR_NOREG;

    mbObj->AduBuff[0] = 0x5A;
    mbObj->AduBuff[1] = 0xA5;
    mbObj->AduBuff[2] = 1 + 1 + Len * 2 + 2; //包括指令、数据、校验
    mbObj->AduBuff[3] = 0x84;

    mbObj->AduBuff[4] = Mode;

    DataIndex = 5;
    
    Src = mbObj->Fun.Master.pData;
    while (Len--)
    {
        WToB.Val = *Src++;
        mbObj->AduBuff[DataIndex++] = WToB.byte.HB;
        mbObj->AduBuff[DataIndex++] = WToB.byte.LB;
    }

    mbObj->SndSize = DataIndex;

    VGUS_PackageSend(mbObj);

    return MB_ERR_NONE;
}

/*----------------------------------------------------------------------------*/
/*-----------------------------写曲线(0x84)------------------------------*/

/*----------------------------------------------------------------------------
 *    按字节编地址，按字节计数
 *    Mode:曲线通道，
 *    Len:写入的字数
 * ---------------------------------------------------------------------------
 * 
 * ---------------------------------------------------------------------------*/
uint8_t VGUS_WriteAuto(mb_obj_type *mbObj, uint16_t ref, uint16_t Len)
{
    uint16_t DataIndex;
    uint8_t *Src;

    if ((Len > 240) || (Len == 0) )
        return MB_ERR_NOREG;

    mbObj->AduBuff[0] = 0x5A;
    mbObj->AduBuff[1] = 0xA5;
    mbObj->AduBuff[2] = Len + 2; //包括指令、数据、校验

    DataIndex = 3;
    
    Src = (uint8_t *)mbObj->Fun.Master.pData;
    while (Len--)
    {
        mbObj->AduBuff[DataIndex++] = *Src++;
    }
    
    mbObj->SndSize = DataIndex;

    VGUS_PackageSend(mbObj);

    return MB_ERR_NONE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////                      读取数据处理                     ////////////
////////////////////////////////////////////////////////////////////////////////

/*----------------------------------------------------------------------------*/
/*------------------------------读寄存器、输入处理-----------------------------*/

/* ---------------------------------------------------------------------------
 * 读多个寄存器和输入寄存器均用此函数处理,读-写多个寄存器也用此函数处理接收数据
 *   响应PDU
 * ---------------------------------------------------------------------------
 *   5A + A5 + 字节数 + 命令 + 地址 + 长度 + 数据 + CRC (数据前均为1B) 
 * ---------------------------------------------------------------------------*/
uint8_t VGUS_ReadRegProc(mb_obj_type *mbObj)
{
    uint8_t DataCnt, DataIndex;
    uint8_t *pDes;
    uint8_t crc_ack_len;
    
    crc_ack_len = 7;
    DataCnt = mbObj->AduBuff[crc_ack_len+5];
    if ((DataCnt != (mbObj->RcvCnt - (crc_ack_len+8))) || (DataCnt != mbObj->Fun.Master.LenReq))
        return MB_ERR_NOREG;

    pDes = (uint8_t *)mbObj->Fun.Master.pData;
    DataIndex = crc_ack_len+6;
    while (DataCnt--)
    {
        *(pDes++) = mbObj->AduBuff[DataIndex];
        DataIndex += 1;
    }

    return MB_ERR_NONE;
}

//   5A + A5 + 字节数 + 命令 + 地址H + 地址L + 长度 + 数据 + CRC (数据前均为1B)
uint8_t VGUS_ReadPvProc(mb_obj_type *mbObj)
{
    uint8_t DataCnt, DataIndex;
    uint8_t *pDes;
    uint8_t crc_ack_len;
    
    crc_ack_len = 7;
    
    DataCnt = mbObj->AduBuff[crc_ack_len+6];
    if ((DataCnt*2 != (mbObj->RcvCnt - (crc_ack_len+9))) || (DataCnt != mbObj->Fun.Master.LenReq))
        return MB_ERR_NOREG;

    pDes = (uint8_t *)mbObj->Fun.Master.pData;
    DataIndex = crc_ack_len+7;
    while (DataCnt--)
    {
        *(pDes++) = mbObj->AduBuff[DataIndex + 1]; //高低位互换，存放在16位数据区
        *(pDes++) = mbObj->AduBuff[DataIndex];
        DataIndex += 2;
    }

    return MB_ERR_NONE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/////////////                      缓冲池处理                       ////////////
////////////////////////////////////////////////////////////////////////////////
/*****************************************************************************/ /*!
* @brief   Master device handle.
*               
* @param   none
*
* @return  none
*
* @ Pass/ Fail criteria: none
*****************************************************************************/

void VGUS_poll(mb_obj_type *mbObj)
{

    uint8_t ErrSt;
    uint32_t Cnt;
    uint32_t bStNextSearchReq;

    if (0 == mbObj->RunSt.bits.MasterMode)
        return;

    if (mbMcuCheck())
        return;

    //状态处理
    bStNextSearchReq = 0;

    if ((MB_RX_RCVEND == mbObj->RcvSt)&&(mbObj->RtuRcvendKeepCnt >= mbObj->RtuRcvendDelaySet))
    {
        ErrSt = MB_ERR_NONE;
        if ((mbObj->AduBuff[3] == mbObj->Fun.Master.pcmdBuff[mbObj->Fun.Master.CmdHandleIndex].Cmd)
            || (FUN_GUS_AUTO == mbObj->Fun.Master.pcmdBuff[mbObj->Fun.Master.CmdHandleIndex].Cmd))
        {
            mbObj->RunSt.bits.Connect = 1;
            mbObj->TimeOutCnt = 0;

            switch (mbObj->Fun.Master.pcmdBuff[mbObj->Fun.Master.CmdHandleIndex].Cmd)
            {
            case FUN_GUS_REG_READ:
                if((mbObj->RcvCnt > (7+9)) 
                && (0 == mb_crc(&(mbObj->AduBuff[3]), 4)) //crc应答
                && (0 == mb_crc(&(mbObj->AduBuff[7+3]), mbObj->RcvCnt - (7+3))) //数据返回
                )
                {
                    ErrSt = VGUS_ReadRegProc(mbObj);
                }
                else
                {
                    ErrSt = MB_ERR_CRC_LRC;
                }
                break;
            case FUN_GUS_PV_READ:
                if((mbObj->RcvCnt > (7+10)) 
                && (0 == mb_crc(&(mbObj->AduBuff[3]), 4)) //crc应答
                && (0 == mb_crc(&(mbObj->AduBuff[7+3]), mbObj->RcvCnt - (7+3))) //数据返回
                )
                {
                    ErrSt = VGUS_ReadPvProc(mbObj);
                }
                else
                {
                    ErrSt = MB_ERR_CRC_LRC;
                }
                break;
            case FUN_GUS_LINE_WRITE:
            case FUN_GUS_PV_WRITE:
            case FUN_GUS_REG_WRITE:
            case FUN_GUS_AUTO:
                if((mbObj->RcvCnt > 3) && (0 == mb_crc(&(mbObj->AduBuff[3]), mbObj->RcvCnt - 3)))
                {
                    if (mbObj->AduBuff[4] != 0xFF)
                    {
                        ErrSt = MB_ERR_NOREG;
                    }
                    else
                    {
                        ErrSt = MB_ERR_NONE;
                    }
                }
                else
                {
                    ErrSt = MB_ERR_CRC_LRC;
                }
                break;
            default:
                //功能码错误 或者返回错误码
                ErrSt = MB_ERR_FUN_CODE;
                break;
            }
            
            //准备执行下一条命令
            if (MB_ERR_NONE == ErrSt)
            {
                mbObj->Fun.Master.ErrCnt = 0;
                //执行回调函数
                if (mbObj->Fun.Master.pcmdBuff[mbObj->Fun.Master.CmdHandleIndex].CallBack != MB_NULL)
                    mbObj->Fun.Master.pcmdBuff[mbObj->Fun.Master.CmdHandleIndex].CallBack(mbObj->Fun.Master.CmdHandleIndex, MB_ERR_NONE);
                //下一条命令
                bStNextSearchReq = 1;

                //执行成功 通信计数
                if (mbObj->success_cnt < UINT32_MAX)
                    mbObj->success_cnt++;
                if (mbObj->success_keep_cnt < UINT16_MAX)
                    mbObj->success_keep_cnt++;
                mbObj->fail_keep_cnt = 0;
            }
            else
            {
                //有错误则累计一次错误计数器，重发本次命令
                mbObj->Fun.Master.ErrCnt++;
            }
        }
        else
        {
            //CRC校验错误或者返回地址错误
            mbObj->Fun.Master.ErrCnt++;
            ErrSt = MB_ERR_CRC_LRC;
        }
        //恢复缓冲区状态
        mbObj->RcvSt = MB_RX_IDLE;
        mbObj->RcvCnt = 0;
        mbObj->ErrSt.St = 0;

        if (mbObj->rcv_end_handle_comp)
            mbObj->rcv_end_handle_comp(ErrSt);
    }
    else if (MB_RX_TIMEOUT == mbObj->RcvSt)
    {
        mbObj->TimeOutCnt = 0;
        if (mbObj->Fun.Master.CmdHandleIndex >= mbObj->Fun.Master.ScmdBuffSize)
        {
            mbObj->Fun.Master.ErrCnt++; //动态命令正常计数
        }
        else
        {
            mbObj->Fun.Master.ErrCnt = MB_ERR_CNT_PV; //静态命令直接跳过
        }

        if (mbObj->rcv_end_handle_comp)
        {
            mbObj->rcv_end_handle_comp(MB_ETIMEDOUT);
        }

        mbObj->RcvCnt = 0;
        mbObj->SndSize = 0;
        mbObj->ErrSt.St = 0;
        mbObj->SndSt = MB_TX_IDLE;
        mbObj->RcvSt = MB_RX_IDLE;
        ErrSt = MB_ETIMEDOUT;
    }
    else
    {
    }

    //error
    if (mbObj->Fun.Master.ErrCnt >= MB_ERR_CNT_PV)
    {
        mbObj->RunSt.bits.Connect = 0;
        mbObj->RunSt.bits.PortSt = 0;
        //执行回调函数
        if (mbObj->Fun.Master.pcmdBuff[mbObj->Fun.Master.CmdHandleIndex].CallBack != MB_NULL)
            mbObj->Fun.Master.pcmdBuff[mbObj->Fun.Master.CmdHandleIndex].CallBack(mbObj->Fun.Master.CmdHandleIndex, ErrSt);
        //准备执行下一条命令
        bStNextSearchReq = 1;

        //通信计数
        if (mbObj->fail_cnt < UINT32_MAX)
            mbObj->fail_cnt++;
        if (mbObj->fail_keep_cnt < UINT16_MAX)
            mbObj->fail_keep_cnt++;
        mbObj->success_keep_cnt = 0;
    }

    //master control //发送下一条命令
    if (bStNextSearchReq)
        mbappCmdSearchNext(mbObj);
    Cnt = mbObj->Fun.Master.CmdBuffSize;
    do
        VGUS_appSendCmd(mbObj);
    while ((mbObj->RcvSt == MB_RX_IDLE) && (mbObj->SndSt == MB_TX_IDLE) && (Cnt--));
}

/*****************************************************************************/ /*!
* @brief   send next mb command.
*               
* @param   none
*
* @return  none
*
* @ Pass/ Fail criteria: none
*****************************************************************************/

void VGUS_appSendCmd(mb_obj_type *mbObj)
{

    mb_cmd_buff_type *pmbCmd;

    if ((mbObj->RcvSt != MB_RX_IDLE) || (mbObj->SndSt != MB_TX_IDLE))
    {
        return;
    }

    pmbCmd = &(mbObj->Fun.Master.pcmdBuff[mbObj->Fun.Master.CmdHandleIndex]); //当前缓冲指针
    if (MB_CMD_ST_READY == pmbCmd->ReqSt)
    {

        mbObj->Fun.Master.pData = pmbCmd->Data; //保存数据指针
        mbObj->SlaveId = pmbCmd->DeviceId;
        switch (pmbCmd->Cmd)
        {
        case FUN_GUS_REG_WRITE:
            VGUS_WriteReg(mbObj, pmbCmd->mbAddr, pmbCmd->amount);
            break;
        case FUN_GUS_REG_READ:
            VGUS_ReadReg(mbObj, pmbCmd->mbAddr, pmbCmd->amount);
            break;
        case FUN_GUS_PV_WRITE:
            VGUS_WritePV(mbObj, pmbCmd->mbAddr, pmbCmd->amount);
            break;
        case FUN_GUS_PV_READ:
            VGUS_ReadPV(mbObj, pmbCmd->mbAddr, pmbCmd->amount);
            break;
        case FUN_GUS_LINE_WRITE:
            VGUS_WriteLine(mbObj, pmbCmd->mbAddr, pmbCmd->amount);
            break;
        case FUN_GUS_AUTO:
            VGUS_WriteAuto(mbObj, pmbCmd->mbAddr, pmbCmd->amount);
            break;
        default:
            //无效的指令
            mbappCmdSearchNext(mbObj);
            break;
        }
    }
    else
    {
        mbappCmdSearchNext(mbObj);
    }
}



/*-----------------------------------------------------------------*
* 应用层接口
*------------------------------------------------------------------*/
void vgus_page_set(mb_obj_type *mbObj, uint16_t *page, void (*CallBack)(uint8_t,uint8_t))
{
    mmb_dyna_cmd_req(mbObj,1,FUN_GUS_REG_WRITE, page, VGUS_RGE_PAGE, 2, CallBack);
}

void vgus_pv_read(mb_obj_type *mbObj, uint16_t *dat, uint16_t addr, uint16_t Len , void (*CallBack)(uint8_t,uint8_t))
{
    mmb_dyna_cmd_req(mbObj, 1, FUN_GUS_PV_READ, dat, addr, Len, CallBack);
}

void vgus_pv_write(mb_obj_type *mbObj, uint16_t *dat, uint16_t addr, uint16_t Len , void (*CallBack)(uint8_t,uint8_t))
{
    mmb_dyna_cmd_req(mbObj, 1, FUN_GUS_PV_WRITE, dat, addr, Len, CallBack);
}



