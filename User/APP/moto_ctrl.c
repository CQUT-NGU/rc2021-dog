
#include "moto_ctrl.h"

void send_chassis_cur1_4(int16_t motor1, int16_t motor2, int16_t motor3, int16_t motor4);
void send_chassis_cur5_8(int16_t motor5, int16_t motor6, int16_t motor7, int16_t motor8);

bool IsMotoReadyOrNot= NotReady;
float ref_agle[8];
float temp_angle;
temp_data temp_pid;      //pid�м�����

/*******************************************************************************************
	*@ �������ƣ�void MotorControl_task(void *pvParameters)
	*@ ���ܣ� ���ܵ���������ݲ��ҽ���PID���� ���������С���Ƶ��λ��
	*@ ��ע�� FREERTOS������
 *******************************************************************************************/
void MotorControl_task(void *pvParameters)
{

    my_can_filter_init_recv_all(&hcan1);
    HAL_CAN_Receive_IT(&hcan1, CAN_FIFO0);
    HAL_CAN_Receive_IT(&hcan2, CAN_FIFO0);

    for(;;)
        moto_behaviour();
}

/**
* NAME: void moto_behaviour(void)
* FUNCTION : ���pid��Ϊ
*/
void moto_behaviour(void)
{

    if(IsMotoReadyOrNot== IsReady) {
        for(int i=0; i<8; i++)
            ref_agle[i]=temp_pid.ref_agle[i];
        IsMotoReadyOrNot= NotReady;
    }

    for(int i=0; i<8; i++)
    {
        pid_calc(&pid_pos[i],moto_chassis[i].total_angle/100,ref_agle[i]/100);  //λ�û� �Ƕȿ���
        temp_pid.out[i] = pid_calc(&pid_spd[i],moto_chassis[i].speed_rpm,pid_pos[i].pos_out);  //�ٶȻ� �ٶȿ���
    }
    send_chassis_cur1_4(temp_pid.out[0],temp_pid.out[1],temp_pid.out[2],temp_pid.out[3]);		//����1-4���ݸ�can�շ���
    send_chassis_cur5_8(temp_pid.out[4],temp_pid.out[5],temp_pid.out[6],temp_pid.out[7]);		//����5-8���ݸ�can�շ���

    vTaskDelay(2);		//���Ʋ���Ƶ��  ʵ��� 1     7�ź�8�ŵ����ʧ��

}

/**
* NAME: void moto_param_init(void)
* FUNCTION : pid��������ʼ��
*/
void moto_param_init(void)
{

    for (int i = 0; i < 8; i++)//���50 ��Ȼ��ɢ
    {   //																										20,0.01,0
        PID_struct_init(&pid_pos[i], POSITION_PID, 1000, 10, 25.0,0.01, 0);  //λ�û�PID�������ã�pid�ṹ�壬PID���ͣ����������������ƣ�P , I , D ��
    }

    for (int i = 0; i < 8; i++)
    {   //																										15.5f,0,0
        PID_struct_init(&pid_spd[i], POSITION_PID, 10000, 0, 15.5f, 0, 0);		//�ٶȻ�PID��pid�ṹ�壬PID���ͣ����������������ƣ�P , I , D ��
    }


}

//���͵��̵����������
void send_chassis_cur1_4(int16_t motor1, int16_t motor2, int16_t motor3, int16_t motor4)
{
    hcan1.pTxMsg->StdId   = 0x200;
    hcan1.pTxMsg->IDE     = CAN_ID_STD;
    hcan1.pTxMsg->RTR     = CAN_RTR_DATA;
    hcan1.pTxMsg->DLC     = 0x08;
    hcan1.pTxMsg->Data[0] = motor1 >> 8;
    hcan1.pTxMsg->Data[1] = motor1;
    hcan1.pTxMsg->Data[2] = motor2 >> 8;
    hcan1.pTxMsg->Data[3] = motor2;
    hcan1.pTxMsg->Data[4] = motor3 >> 8;
    hcan1.pTxMsg->Data[5] = motor3;
    hcan1.pTxMsg->Data[6] = motor4 >> 8;
    hcan1.pTxMsg->Data[7] = motor4;
    HAL_CAN_Transmit(&hcan1, 10);
}

//���͵��̵����������
void send_chassis_cur5_8(int16_t motor5, int16_t motor6, int16_t motor7, int16_t motor8)
{
    hcan1.pTxMsg->StdId   = 0x1ff;
    hcan1.pTxMsg->IDE     = CAN_ID_STD;
    hcan1.pTxMsg->RTR     = CAN_RTR_DATA;
    hcan1.pTxMsg->DLC     = 0x08;
    hcan1.pTxMsg->Data[0] = motor5 >> 8;
    hcan1.pTxMsg->Data[1] = motor5;
    hcan1.pTxMsg->Data[2] = motor6 >> 8;
    hcan1.pTxMsg->Data[3] = motor6;
    hcan1.pTxMsg->Data[4] = motor7 >> 8;
    hcan1.pTxMsg->Data[5] = motor7;
    hcan1.pTxMsg->Data[6] = motor8 >> 8;
    hcan1.pTxMsg->Data[7] = motor8;
    HAL_CAN_Transmit(&hcan1, 10);
}


