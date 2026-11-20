#include "main.h"
#include "can.h"
#include "WEI_can.h"
#include <string.h>

void WEI_CAN_RxExchange(CAN_HandleTypeDef* _hcan,uint32_t fifox/*接收邮箱*/,DJI_Motor_Controller* line) //接收触发的hcan和线路
{
    uint8_t can_rx_buff[8];
    static CAN_RxHeaderTypeDef rxconf; //接收头段
    CANInstance* motor_this = NULL;
    while(HAL_CAN_GetRxFifoFillLevel(_hcan,fifox))
    {
        if(HAL_CAN_GetRxMessage(_hcan,fifox,&rxconf,can_rx_buff) == HAL_OK)
        //依据GetRxMessage得到的头段确定this指向谁
        {
            motor_this = NULL;

            for(int i = 0;i < 4;i ++)
            {
                if(line->motor[i] != NULL && line->motor[i]->this != NULL && line->motor[i]->rx_id == rxconf.StdId){motor_this = line->motor[i];}
            }
            
            if(motor_this == NULL) 
            {
                line->statue = WEI_ERROR;
                continue;
            }

            motor_this->rx_len = rxconf.DLC; //保存接收到的数据长度
            memcpy(motor_this->rx_buff,can_rx_buff,rxconf.DLC); //消息拷贝到对应实例
            motor_this->this->last_ecd = motor_this->this->ecd;

            //解析电机数据
            WEI_CAN_RxExplain(motor_this->rx_buff,motor_this->this);//转换数据到目标电机的this里

            //处理数据
            motor_this->this ->angle_siggle_round = 360.0f/8192.0f * (float)motor_this->this ->ecd;
            motor_this->this ->speed_aps = (1.0f - 0.85f/*给上次数据加权*/) * motor_this->this ->speed_aps + 0.85f * (360.f / 60.0f) * motor_this->this ->speed_rpm;
            motor_this->this ->real_current_handle = (1.0f - 0.5f) * motor_this->this ->real_current_handle + 0.5f * (float)motor_this->this-> real_current;
            
            if(motor_this->this ->ecd - motor_this->this ->last_ecd > 4096)
            {
                motor_this->this ->total_round --;
            }
            else if(motor_this->this ->ecd - motor_this->this ->last_ecd < -4096)
            {
                motor_this->this ->total_round ++;
            }

            motor_this->this ->total_angle = motor_this->this ->total_round * 360 + motor_this->this ->angle_siggle_round;

        }
        else
        {
            //错误处理区域
            break;
        }
    }
}

void WEI_CAN_RxExplain(uint8_t* Data,DJI_Motor_Measure_s* aim) //适配8位数据的C620电调
//依照规则将Data里的数据转移到aim里
{
    aim->ecd = (Data[0] << 8) | Data[1];
    aim->speed_rpm = (Data[2] << 8) | Data[3];
    aim->real_current = (Data[4] << 8) | Data[5];
    aim->temperature = Data[6];
}

void WEI_CAN_DJIContorller_Init(DJI_Motor_Controller* aim)
{
    for(int i = 0;i < 4;i ++)
    {
        aim->motor[i] = NULL;
    }
    aim->statue = WEI_OK;
}

void WEI_CAN_MotorInstance_Init(DJI_Motor_Measure_s* measure,CANInstance* motor,DJI_Motor_Controller* aim,uint8_t N/*把第几个motor设置为目标*/) //说明：这函数必须传入全局变量
{
    aim->motor[N] = motor;
    motor->this = measure;
}