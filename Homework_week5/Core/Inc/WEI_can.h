#include "main.h"
typedef struct{
    //原始数据
    uint16_t ecd; //机械角度 0 - 8191,刻度共有8192格
    int16_t speed_rpm; //转速，单位为rpm
    int16_t real_current; //实际电流
    uint8_t temperature; //温度 摄氏度

    //处理后的数据
    int16_t real_current_handle; //处理后的电流
    float speed_aps; //角速度 度/s
    uint16_t last_ecd; //上次读取的编码器值
    float angle_siggle_round; //单圈角度
    float total_angle; //总角度
    int32_t total_round; //总圈数

}DJI_Motor_Measure_s;

typedef struct 
{
    // CAN_HandleTypeDef* hcanHandle; //can句柄
    // CAN_TxHeaderTypeDef* txconf; //报文发送头配置
    // uint32_t tx_id; //发送ID
    //uint32_t tx_mailbox; //发送邮箱号
    uint8_t tx_buff[8]; //发送缓存

    uint8_t rx_buff[8]; //接收缓存
    uint32_t rx_id; //接收ID
    uint8_t rx_len; //接收长度

    DJI_Motor_Measure_s* this;

}CANInstance; //储存电机参数用

typedef enum{
WEI_OK,
WEI_ERROR
}WEI_STATUE;

typedef struct{
CAN_HandleTypeDef* hcanHandle; //can句柄
CAN_TxHeaderTypeDef* txconf; //报文发送头配置
uint32_t tx_id; //发送ID
uint32_t tx_mailbox; //发送邮箱号
uint32_t rx_mailbox; //接收邮箱号


CANInstance* motor[4];

WEI_STATUE statue;//注：仅statue仅作为接收到信号后发现信号和任何motor不匹配时提示用，本身不具备逻辑效力

}DJI_Motor_Controller; //专门用于控制一条CAN线上的电机的结构体，用于简化自己写回调函数的流程

void WEI_CAN_RxExchange(CAN_HandleTypeDef* _hcan,uint32_t fifox/*接收邮箱*/,DJI_Motor_Controller* line);
void WEI_CAN_RxExplain(uint8_t* Data,DJI_Motor_Measure_s* aim);
void WEI_CAN_DJIContorller_Init(DJI_Motor_Controller* aim);
void WEI_CAN_MotorInstance_Init(DJI_Motor_Measure_s* measure,CANInstance* motor,DJI_Motor_Controller* aim,uint8_t N/*把第几个motor设置为目标*/); //说明：这函数必须传入全局变量