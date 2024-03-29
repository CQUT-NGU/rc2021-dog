
#include "posture_ctrl.h"

float x, y, theta1, theta2;  //足端坐标下x，y，角度1，角度2

enum States state = REALSE;

extern TaskHandle_t MotorControlTask_Handler;

float _angle_initial, _rotate_angle;

bool TurnLeftFlag1, TurnLeftFlag2, TurnRightFlag1, TurnRightFlag2, ClimbingFlag1, ClimbingFlag2;

bool _leg_active[4] = {1, 1, 1, 1};

float now_time;

bool rc_ctrl_flag = 1;

//初始化步态增益
LegGain gait_gains = {15.5, 0.00, 25.0, 0.00};
//设定每一种步态的具体增益
LegGain state_gait_gains[] = {
    //{kp_pos, kd_pos} kp_spd, kd_spd,
    {22.0, 0.00, 8.0, 0.00},  //位置环 速度环 TROT
    {22.0, 0.00, 8.0, 0.00},  //TEST1
    {22.0, 0.00, 8.0, 0.00},
    {22.0, 0.00, 8.0, 0.00},
    {22.0, 0.00, 8.0, 0.00},
    {22.0, 0.00, 8.0, 0.00},
    {22.0, 0.00, 8.0, 0.00},
    {22.0, 0.00, 8.0, 0.00},
    {22.0, 0.00, 8.0, 0.00},
    {22.0, 0.00, 8.0, 0.00},
    {22.0, 0.00, 8.0, 0.00},
    {22.0, 0.00, 8.0, 0.00},
    {22.0, 0.00, 8.0, 0.00},  //TEST12
    {22.0, 0.00, 8.0, 0.00},  //CLIMB
    {22.0, 0.00, 8.0, 0.00},
    {22.0, 0.00, 8.0, 0.00},
    {22.0, 0.00, 8.0, 0.00},
    {22.0, 0.00, 8.0, 0.00},
    {22.0, 0.00, 8.0, 0.00},
    {22.0, 0.00, 8.0, 0.00},
    {22.0, 0.00, 8.0, 0.00},  //STOP
    {22.0, 0.00, 8.0, 0.00},  //REL
    {22.0, 0.00, 8.0, 0.00},
    {22.0, 0.00, 8.0, 0.00},
    {22.0, 0.00, 8.0, 0.00},
};

//初始化步态参数
GaitParams gait_params = {17.3, 12.0, 4.0, 0.00, 0.25, 1};
//设定每一种步态的具体参数
GaitParams state_gait_params[] = {
    //{stance_height, step_length, up_amp, down_amp, flight_percent, freq}  cm
    //{步高, 步长, 抬腿高, 压腿高, 飞行占比, 频率}    单位 cm
    {16.0, 16.0, 5.00, 2.00, 0.25, 1.85},  // TROT    双拍步态 对角小跑
    {26.0, 10.00, 3, 2.00, 0.20, 2.0},     //1
    {26.0, 10.00, 3, 2.00, 0.20, 2.0},     //2
    {26.0, 10.00, 3, 2.00, 0.20, 2.0},     //3
    {NAN, NAN, NAN, NAN, NAN, NAN},        //4
    {NAN, NAN, NAN, NAN, NAN, NAN},        //5
    {NAN, NAN, NAN, NAN, NAN, NAN},        //6
    {NAN, NAN, NAN, NAN, NAN, NAN},        //7
    {NAN, NAN, NAN, NAN, NAN, NAN},        //8
    {NAN, NAN, NAN, NAN, NAN, NAN},        //9
    {NAN, NAN, NAN, NAN, NAN, NAN},        //10
    {NAN, NAN, NAN, NAN, NAN, NAN},        //11
    {NAN, NAN, NAN, NAN, NAN, NAN},        //12

    {16.3, 0.00, 5.70, 3.00, 0.30, 2.8},  // CLIMBING    爬坡

    {16.3, 0.00, 6.0, 0.00, 2.5, 1.5},    // WALK
    {16.0, 8.00, 5.00, 0.00, 0.30, 1.2},  // WALK BACK

    {23.0, 7.00, 3, 2.00, 0.25, 3.0},  // ROTATE LEFT            {23.0, 10.00, 3, 2.00, 0.20, 2.0},  {21.0, 16.0, 5.00, 3.00, 0.25, 2.3},
    {21.0, 7.00, 5, 2.00, 0.25, 3.0},  // ROTATE RIGHT            {23.0, 10.00, 3, 2.00, 0.20, 2.0},

    {17.0, 0.00, 6.00, 4.00, 0.35, 2.0},  //BOUND
    {17.0, 0.00, 6.00, 0.00, 0.35, 2.0},  // GALLOP

    {NAN, NAN, NAN, NAN, NAN, NAN},  // STOP
    {NAN, NAN, NAN, NAN, NAN, NAN},  // RELASE
    {NAN, NAN, NAN, NAN, NAN, NAN},  // JUMP
    {NAN, NAN, NAN, NAN, NAN, NAN},  // START
    {NAN, NAN, NAN, NAN, NAN, NAN},  // END

    {25.3, 16.0, 15.00, 0.00, 0.30, 1.2}  // TEST
};

/**
*    最低walk {12.0, 15.0, 1.8, 0.00, 0.50, 1.0}
*    效果不错{14.0, 12.0, 3.5, 0.00, 0.50, 1.0},
*    {16.0, 12.00, 5.0, 0.00, 0.20, 2.0}, // WALK        ahead 不错
*    {17.3, 12.00, 4.0, 0.00, 0.35, 2.5}, // WALK        ahead
* 对角小跑步态不错速度和稳定都非常好
*/

GaitParams test_gait_params = {16.0, 15.0, 5.00, 2.00, 0.25, 2.3};  // TROT        //双拍步态 对角小跑
LegGain    test_gait_gains  = {15.5, 0.00, 25.0, 0.00};

const float TROT_stance_height  = 26.0;
const float TROT_step_length    = 25.0;
const float TROT_up_amp         = 15.2;
const float TROT_down_amp       = 0.0;
const float TROT_flight_percent = 0.20;
const float TROT_freq           = 0.2;

const float TROT_step_length2 = 25.0;
const float TROT_up_amp2      = 15.2;

//        {26.0, 25.00, 15.0, 0.00, 0.20, 0.2},
//        {26.0, 25.00, 15.0, 0.00, 0.20, 0.2},

//        {26.0, 25.00-15, 15.0-13, 0.00, 0.20, 0.2},
//        {26.0, 25.00-15, 15.0-13, 0.00, 0.20, 0.2}

DetachedParam detached_params;
DetachedParam state_detached_params[] = {

    {                                      //TROT 对角小跑
     {21.0, 14.0, 3.00, 2.00, 0.25, 4.0},  //   {21.0, 12.0, 3.00, 2.00, 0.25, 4.0}
     {21.0, 14.0, 3.00, 2.00, 0.25, 4.0},
     {21.0, 14.0, 3.00, 2.00, 0.25, 4.0},
     {21.0, 14.0, 3.00, 2.00, 0.25, 4.0}},
    {// TEST1
     {25.0, 10.00, 3, 2.00, 0.25, 3.8},
     {25.0, 10.00, 3, 2.00, 0.25, 3.8},

     {25.0, 10.00, 3, 2.00, 0.25, 3.8},
     {25.0, 10.00, 3, 2.00, 0.25, 3.8}},
    {//TEST2
     {28.0, 25.00, 17, 0.00, 0.49, 0.4},
     {28.0, 25.00, 17, 0.00, 0.49, 0.4},

     {28.0, 12.00, 4, 0.00, 0.49, 0.4},
     {28.0, 12.00, 4, 0.00, 0.49, 0.4}},
    {
        //TEST3
        {28.0, 12.00, 2, 0.00, 0.49, 0.4},
        {28.0, 12.00, 2, 0.00, 0.49, 0.4},

        {28.0, 20.00, 17, 0.00, 0.48, 0.4},
        {28.0, 20.00, 17, 0.00, 0.48, 0.4},
    },
    {// TEST4
     {17.5, 14.00, 4, 0.00, 0.25, 2.0},
     {17.5, 14.00, 4, 0.00, 0.25, 2.0},

     {20.5, 14.00, 4, 0.00, 0.25, 2.0},
     {20.5, 14.00, 4, 0.00, 0.25, 2.0}},
    {
        // TEST5
        {23.0, 18.00, 6, 1.00, 0.25, 2.0},
        {23.0, 18.00, 6, 1.00, 0.25, 2.0},

        {16.0, 10.00, 3, 1.00, 0.25, 2.0},
        {16.0, 10.00, 3, 1.00, 0.25, 2.0},
    },
    {// TEST6
     {26.0, 6.00, 3, 0.00, 0.25, 3.0},
     {26.0, 6.00, 3, 0.00, 0.25, 3.0},

     {26.0, 6.00, 3, 0.00, 0.25, 3.0},
     {26.0, 6.00, 3, 0.00, 0.25, 3.0}},
    {// TEST7       //调整方向 小碎步转弯 左
     {26.0, 8.0, 4.00, 3.00, 0.25, 2.1},
     {26.0, 8.0, 4.00, 3.00, 0.25, 2.1},
     {26.0, 8.0, 4.00, 3.00, 0.25, 2.1},
     {26.0, 8.0, 4.00, 3.00, 0.25, 2.1}},
    {// TEST8            //调整方向 小碎步转弯 右
     {26.0, 8.0, 4.00, 3.00, 0.25, 2.1},
     {26.0, 8.0, 4.00, 3.00, 0.25, 2.1},
     {26.0, 8.0, 4.00, 3.00, 0.25, 2.1},
     {26.0, 8.0, 4.00, 3.00, 0.25, 2.1}},
    {// TEST9
     {28.0, 4.00, 3, 0.00, 0.25, 2.3},
     {28.0, 4.00, 3, 0.00, 0.25, 2.3},

     {28.0, 4.00, 3, 0.00, 0.25, 2.3},
     {28.0, 4.00, 3, 0.00, 0.25, 2.3}},
    {// TEST10
     {28.0, 4.00, 3, 0.00, 0.25, 2.3},
     {28.0, 4.00, 3, 0.00, 0.25, 2.3},

     {28.0, 4.00, 3, 0.00, 0.25, 2.3},
     {28.0, 4.00, 3, 0.00, 0.25, 2.3}},
    {// TEST11
     {28.0, 4.00, 3, 0.00, 0.25, 2.3},
     {28.0, 4.00, 3, 0.00, 0.25, 2.3},

     {28.0, 4.00, 3, 0.00, 0.25, 2.3},
     {28.0, 4.00, 3, 0.00, 0.25, 2.3}},
    {
        // TEST12
        {20.3, 30.00, 5.70, 3.00, 0.44, 2.8},
        {20.3, 30.00, 5.70, 3.00, 0.44, 2.8},

        {20.3, 30.00, 5.70, 3.00, 0.44, 2.8},
        {20.3, 30.00, 5.70, 3.00, 0.44, 2.8},
    },

    {
        //CLIMBING    爬坡

        {16.0, 9.0, 3.00, 1.00, 0.25, 3.6},  //小步子可以迈上去
        {16.0, 9.0, 3.00, 1.00, 0.25, 3.6},

        {18.0, 9.0, 3.00, 1.00, 0.25, 3.6},
        {18.0, 9.0, 3.00, 1.00, 0.25, 3.6}

        //                {16.0, 16.0, 3.00, 1.00, 0.25, 3.0},        //大步子迈上去可以
        //        {16.0, 16.0, 3.00, 1.00, 0.25, 3.0},
        //
        //        {17.0, 16.0, 3.00, 1.00, 0.25, 3.0},
        //        {17.0, 16.0, 3.00, 1.00, 0.25, 3.0}

    },

    {
        {27.3, 12.00, 16.0, 0.00, 0.20, 1.0},  // WALK
        {27.3, 12.00, 16.0, 0.00, 0.20, 1.0},  // WALK
        {27.3, 12.00, 16.0, 0.00, 0.20, 1.0},  // WALK
        {27.3, 12.00, 16.0, 0.00, 0.20, 1.0}   // WALK
    },

    {{25.0, 10.00, 3, 2.00, 0.25, 3.8},
     {25.0, 10.00, 3, 2.00, 0.25, 3.8},
     {25.0, 10.00, 3, 2.00, 0.25, 3.8},
     {25.0, 10.00, 3, 2.00, 0.25, 3.8}

    },

    {
        {23.0, 7.00, 3, 2.00, 0.25, 3.0},  // ROTATE LEFT
        {23.0, 7.00, 3, 2.00, 0.25, 3.0},  // ROTATE LEFT
        {23.0, 7.00, 3, 2.00, 0.25, 3.0},  // ROTATE LEFT
        {23.0, 7.00, 3, 2.00, 0.25, 3.0}   // ROTATE LEFT
    },

    {
        {21.0, 7.00, 5, 2.00, 0.25, 3.0},  // ROTATE RIGHT
        {21.0, 7.00, 5, 2.00, 0.25, 3.0},  // ROTATE RIGHT
        {21.0, 7.00, 5, 2.00, 0.25, 3.0},  // ROTATE RIGHT
        {21.0, 7.00, 5, 2.00, 0.25, 3.0}   // ROTATE RIGHT
    },

    {
        {17.0, 0.00, 6.00, 4.00, 0.35, 2.0},  //BOUND
        {17.0, 0.00, 6.00, 4.00, 0.35, 2.0},  //BOUND
        {17.0, 0.00, 6.00, 4.00, 0.35, 2.0},  //BOUND
        {17.0, 0.00, 6.00, 4.00, 0.35, 2.0}   //BOUND
    },

    {
        {21.0, 16.00, 6.00, 1.00, 0.25, 3.8},  // GALLOP
        {21.0, 16.00, 6.00, 1.00, 0.25, 3.8},  // GALLOP
        {21.0, 16.00, 6.00, 1.00, 0.25, 3.8},  // GALLOP
        {21.0, 16.00, 6.00, 1.00, 0.25, 3.8}   // GALLOP
    },

};

/*
                                                                                    0----------1
                                                                                    -          -
                                                                                    -          -
                                                                                    -          -
                                                                                    -          -
                                                                                    2----------3
*/
DetachedParam RcDetachedParam = {
    {21.0, 0.0, 3.00, 2.00, 0.25, 3.2},
    {21.0, 0.0, 3.00, 2.00, 0.25, 3.2},
    {21.0, 0.0, 3.00, 2.00, 0.25, 3.2},
    {21.0, 0.0, 3.00, 2.00, 0.25, 3.2}};
/*******************************************************************************************
    *@ 任务名称：void PostureControl_task(void *pvParameters)
    *@ 功能：姿态控制
    *@ 备注：选择状态
 *******************************************************************************************/
void PostureControl_task(void *pvParameters)
{
    //            step_len_initial = state_detached_params[TROT].detached_params_0.step_length;
    //        step_high_initial = state_detached_params[CLIMBING].detached_params_0.stance_height;
    //
    while (1)
    {
        //        _Pitch_rev=_Pitch_initial-imuinfo.ActVal[1];
        //        _Roll_rev=_Roll_initial-imuinfo.ActVal[2];

        gait_params     = state_gait_params[state];
        detached_params = state_detached_params[state];

        //gait_gains =state_gait_gains[state] ;
        switch (state)
        {
        case STOP:  //停止
            x = 0;
            //y = 17.3205081;
            y = 21;
            CartesianToTheta(1.0);
            LegGain gains = {10, 0.00, 5, 0.00};
            //pid_spd_out_limit=400;
            CommandAllLegs(gains);
            break;

        case REALSE:  // 释放 什么都不做
            vTaskDelay(500);
            break;

        case CLIMBING:  //爬坡
            gait_detached(detached_params, 0.0, 0.5, 0.5, 0.0, -1.0, -1.0, -1.0, -1.0);
            break;

        case WALK:  //前进
            gait_detached(detached_params, 0.25, 0.75, 0.0, 0.5, 1.0, 1.0, 1.0, 1.0);
            break;

        case WALK_BACK:  //后退
            gait(gait_params, gait_gains, 0.0, 0.5, 0.5, 0.0, -1.0, -1.0, -1.0, -1.0);
            break;

        case BOUND:  //跳跑
            gait_detached(detached_params, 0.0, 0.0, 0.5, 0.5, 1.0, 1.0, 1.0, 1.0);
            break;

        case GALLOP:  //袭步
            gait_detached(detached_params, 0.9, 0.7, 0.2, 0.0, 1.0, 1.0, 1.0, 1.0);
            break;

        case ROTAT_LEFT:  //原地左转 YAW+
            //gait_detached(detached_params, 0.0, 0.5, 0.5, 0.0, -1.0, 1.0, -1.0, 1.0);
            gait(gait_params, gait_gains, 0.0, 0.5, 0.5, 0.00, -1.0, 1.0, -1.0, 1.0);  //未分离的步态
                                                                                       //            if(imuinfo.ActVal[0]>=_angle_initial+_rotate_angle)
                                                                                       //                state= STOP;

            break;
        case ROTAT_RIGHT:  //原地右转 YAW-
            // gait_detached(detached_params, 0.0, 0.5, 0.5, 0.0, 1.0, -1.0, 1.0, -1.0);
            gait(gait_params, gait_gains, 0.0, 0.5, 0.5, 0.00, 1.0, -1.0, 1.0, -1.0);  //未分离的步态
                                                                                       //            if(imuinfo.ActVal[0]<=_angle_initial-_rotate_angle)
                                                                                       //                state= STOP;

            break;
        case JUMP:  //跳跃
            //pid_spd_out_limit=6720;
            ExecuteJump();
            break;

        case START:  //初始化
            StartPosToMiddlePos();
            break;

        case END:  //结束
            MiddlePosToEndPos();
            break;

        case TROT:  //双拍步态 对角小跑

            if (rc_ctrl_flag == 0)
                gait_detached(detached_params, 0.0, 0.5, 0.5, 0.0, 1.0, 1.0, 1.0, 1.0);
            //pid_spd_out_limit=6720;
            else if (rc_ctrl_flag == 1)
                gait_detached(RcDetachedParam, 0.0, 0.5, 0.5, 0.0, 1.0, 1.0, 1.0, 1.0);

            break;

        case TEST1:  //测试步态
            gait_detached(detached_params, 0.0, 0.5, 0.50, 0.0, 1.0, 1.0, 1.0, 1.0);
            break;

        case TEST2:  //测试步态
            gait_detached(detached_params, 0.0, 0.5, 0.25, 0.75, 1.0, 1.0, 1.0, 1.0);
            break;

        case TEST3:  //测试步态
            gait_detached(detached_params, 0.0, 0.5, 0.25, 0.75, 1.0, 1.0, 1.0, 1.0);
            break;

        case TEST4:  //测试步态
            gait_detached(detached_params, 0.0, 0.5, 0.50, 0.0, 1.0, 1.0, 1.0, 1.0);
            break;

        case TEST5:  //测试步态
            gait_detached(detached_params, 0.0, 0.5, 0.5, 0.00, 1.0, 1.0, 1.0, 1.0);
            break;

        case TEST6:  //测试步态
            gait_detached(detached_params, 0.0, 0.5, 0.5, 0.0, 1.0, 1.0, 1.0, 1.0);
            break;

        case TEST7:  //测试步态
            gait_detached(detached_params, 0.0, 0.5, 0.5, 0.00, -1.0, 1.0, -1.0, 1.0);
            break;

        case TEST8:  //测试步态
            gait_detached(detached_params, 0.0, 0.5, 0.5, 0.00, 1.0, -1.0, 1.0, -1.0);
            break;

        case TEST9:  //测试步态
            gait_detached(detached_params, 0.0, 0.5, 0.25, 0.75, 1.0, 1.0, 1.0, 1.0);
            break;

        case TEST10:  //测试步态
            gait_detached(detached_params, 0.0, 0.5, 0.25, 0.75, 1.0, 1.0, 1.0, 1.0);
            break;

        case TEST11:  //测试步态
            gait_detached(detached_params, 0.0, 0.5, 0.25, 0.75, 1.0, 1.0, 1.0, 1.0);
            break;

        case TEST12:  //测试步态
            gait_detached(detached_params, 0.0, 0.0, 0.5, 0.5, -1.0, -1.0, -1.0, -1.0);
            break;
        }
        vTaskDelay(3);
    }
}

/**
* NAME: void gait_detached
* FUNCTION : 四腿分离的腿部增益函数
*/
void gait_detached(DetachedParam d_params,
                   float         leg0_offset,
                   float         leg1_offset,
                   float         leg2_offset,
                   float         leg3_offset,
                   float         leg0_direction,
                   float         leg1_direction,
                   float         leg2_direction,
                   float         leg3_direction)
{
    float t = HAL_GetTick() / 1000.0 - now_time / 1000.0;

    // const float leg0_direction = 1.0;
    if (_leg_active[0] == YES)
        CoupledMoveLeg(t, d_params.detached_params_0, leg0_offset, leg0_direction, 0);

    // const float leg1_direction = 1.0;
    if (_leg_active[1] == YES)
        CoupledMoveLeg(t, d_params.detached_params_1, leg1_offset, leg1_direction, 1);

    // const float leg2_direction = 1.0;
    if (_leg_active[2] == YES)
        CoupledMoveLeg(t, d_params.detached_params_2, leg2_offset, leg2_direction, 2);

    //  const float leg3_direction = 1.0;
    if (_leg_active[3] == YES)
        CoupledMoveLeg(t, d_params.detached_params_3, leg3_offset, leg3_direction, 3);
}

/**
* NAME: void gait(    GaitParams params,float leg0_offset, float leg1_offset,float leg2_offset, float leg3_offset)
* FUNCTION : 产生时间脉冲 设定每个腿的参数 调整腿的运行方向 进行补偿
*/
void gait(GaitParams params, LegGain gains, float leg0_offset, float leg1_offset, float leg2_offset, float leg3_offset, float leg0_direction, float leg1_direction, float leg2_direction, float leg3_direction)
{
    //    if (!IsValidGaitParams(params) || !IsValidLegGain(gains)) {
    //        return;
    //    }

    float t = HAL_GetTick() / 1000.0 - now_time / 1000.0;

    //printf("\r\n t=%f",t);

    // const float leg0_direction = 1.0;
    CoupledMoveLeg(t, params, leg0_offset, leg0_direction, 0);

    // const float leg1_direction = 1.0;
    CoupledMoveLeg(t, params, leg1_offset, leg1_direction, 1);

    // const float leg2_direction = 1.0;
    CoupledMoveLeg(t, params, leg2_offset, leg2_direction, 2);

    //  const float leg3_direction = 1.0;
    CoupledMoveLeg(t, params, leg3_offset, leg3_direction, 3);

    //改变PD
    // ChangeTheGainsOfPD(gains);
}

/**
* NAME: SinTrajectory (float t,GaitParams params, float gaitOffset)
* FUNCTION : 正弦轨迹生成器
*/
void SinTrajectory(float t, GaitParams params, float gaitOffset)
{
    static float p      = 0;
    static float prev_t = 0;

    float stanceHeight  = params.stance_height;
    float downAMP       = params.down_amp;
    float upAMP         = params.up_amp;
    float flightPercent = params.flight_percent;
    float stepLength    = params.step_length;
    float FREQ          = params.freq;

    p += FREQ * (t - prev_t);
    prev_t = t;

    float gp = fmod((p + gaitOffset), 1.0);
    if (gp <= flightPercent)
    {
        x = (gp / flightPercent) * stepLength - stepLength / 2.0;
        y = -upAMP * sin(PI * gp / flightPercent) + stanceHeight;
    }
    else
    {
        float percentBack = (gp - flightPercent) / (1.0 - flightPercent);
        x                 = -percentBack * stepLength + stepLength / 2.0;
        y                 = downAMP * sin(PI * percentBack) + stanceHeight;
    }
}

/**
* NAME: void CoupledMoveLeg(float t, GaitParams params,float gait_offset, float leg_direction, int LegId)
* FUNCTION : 驱动并联腿 传递参数
*/
void CoupledMoveLeg(float t, GaitParams params, float gait_offset, float leg_direction, int LegId)
{
    CartesianToTheta(leg_direction);  //笛卡尔坐标转换到伽马坐标

    SinTrajectory(t, params, gait_offset);  //足端摆线轨迹生成器

    SetCoupledPosition(LegId);  //发送数据给电机驱动函数
    //  printf("\r\nt=%f x=%f  y=%f  theta1=%f  theta2=%f  legid=%d he%f",t,x,y,theta1,theta2,LegId,theta1+theta2);
}

/**
* NAME: void CartesianToThetaGamma(float leg_direction)
* FUNCTION : 笛卡尔坐标转换到角度坐标 也就是将xy转换成theta
*/
void CartesianToTheta(float leg_direction)
{
    float  L  = 0;  //当前足端在坐标系中的长度。
    float  N  = 0;
    double M  = 0;
    float  A1 = 0;  //腿上一个电机的角度。
    float  A2 = 0;  //腿上另一个电机的角度。

    L = sqrt(pow(x, 2) + pow(y, 2));

    if (L < 10)
        L = 10;
    else if (L > 30)
        L = 30;

    // vTaskSuspend(MotorControlTask_Handler);

    N  = asin(x / L) * 180.0 / PI;
    M  = acos((pow(L, 2) + pow(L1, 2) - pow(L2, 2)) / (2 * L1 * L)) * 180.0 / PI;
    A1 = M - N;

    A2 = M + N;

    if (leg_direction == 1.0)
    {
        theta2 = (A1 - 90.0);
        theta1 = (A2 - 90.0);
    }
    else if (leg_direction == -1.0)
    {
        theta1 = (A1 - 90.0);
        theta2 = (A2 - 90.0);
    }

    //    printf("\r\n x=%f  y=%f  theta1=%f  theta2=%f   he=%f   L=%f  M=%lf  N=%f   SIXI=%f",x,y,theta1,theta2,theta1+theta2,L,M,N,(    (pow(L,2)+pow(L1,2)-pow(L2,2))/(2*L1*L)    ));
}

bool  climbing_offset_flag   = NO;
float _climbing_offset_angle = 15;
/**
* NAME: void SetCoupledPosition( int LegId)
* FUNCTION : 发送电机控制角度
*/
void SetCoupledPosition(int LegId)
{
    //    //限位保护
    //    if((theta1+theta2)>178||(theta1+theta2)<-178||theta1>170||theta1<-170||theta2>170||theta2<-170)
    //        vTaskSuspend(MotorControlTask_Handler);

    if (climbing_offset_flag == YES)
    {
        if (LegId == 0)
        {
            temp_pid.ref_agle[1] = -theta1 * ReductionAndAngleRatio + _climbing_offset_angle;
            temp_pid.ref_agle[0] = -theta2 * ReductionAndAngleRatio - _climbing_offset_angle;
        }
        else if (LegId == 1)
        {
            temp_pid.ref_agle[2] = theta1 * ReductionAndAngleRatio - _climbing_offset_angle;
            temp_pid.ref_agle[3] = theta2 * ReductionAndAngleRatio + _climbing_offset_angle;
        }

        else if (LegId == 2)
        {
            temp_pid.ref_agle[5] = -theta1 * ReductionAndAngleRatio + _climbing_offset_angle;
            temp_pid.ref_agle[4] = -theta2 * ReductionAndAngleRatio - _climbing_offset_angle;
        }
        else if (LegId == 3)
        {
            temp_pid.ref_agle[6] = +theta1 * ReductionAndAngleRatio - _climbing_offset_angle;
            temp_pid.ref_agle[7] = +theta2 * ReductionAndAngleRatio + _climbing_offset_angle;
        }
    }
    else
    {
        if (LegId == 0)
        {
            temp_pid.ref_agle[1] = -theta1 * ReductionAndAngleRatio;
            temp_pid.ref_agle[0] = -theta2 * ReductionAndAngleRatio;
        }
        else if (LegId == 1)
        {
            temp_pid.ref_agle[2] = theta1 * ReductionAndAngleRatio;
            temp_pid.ref_agle[3] = theta2 * ReductionAndAngleRatio;
        }

        else if (LegId == 2)
        {
            temp_pid.ref_agle[5] = -theta1 * ReductionAndAngleRatio;
            temp_pid.ref_agle[4] = -theta2 * ReductionAndAngleRatio;
        }
        else if (LegId == 3)
        {
            temp_pid.ref_agle[6] = +theta1 * ReductionAndAngleRatio;
            temp_pid.ref_agle[7] = +theta2 * ReductionAndAngleRatio;
        }
    }

    //后腿儿朝向向后
    //         else if(LegId==2)
    //    {
    //        temp_pid.ref_agle[4]=theta1*ReductionAndAngleRatio;
    //        temp_pid.ref_agle[5]=theta2*ReductionAndAngleRatio;
    //    }
    //    else if(LegId==3)
    //    {
    //        temp_pid.ref_agle[7]=-theta1*ReductionAndAngleRatio;
    //        temp_pid.ref_agle[6]=-theta2*ReductionAndAngleRatio;
    //    }

    IsMotoReadyOrNot = IsReady;  //数据填充完毕
}

/**
* NAME: void CommandAllLegs(void)
* FUNCTION : 控制所有电机
*/
void CommandAllLegs(LegGain gains)
{
    //    if (!IsValidLegGain(gains)) {
    //        return;
    //    }

    //    //限位保护
    //    if((theta1+theta2)>170||(theta1+theta2)<-170||theta1>140||theta1<-140||theta2>140||theta2<-140)
    //        vTaskSuspend(MotorControlTask_Handler);

    ChangeTheGainsOfPD(gains);

    SetCoupledPosition(0);
    SetCoupledPosition(1);
    SetCoupledPosition(2);
    SetCoupledPosition(3);
}

void CommandAllLegs_v(void)
{
    SetCoupledPosition(0);
    SetCoupledPosition(1);
    SetCoupledPosition(2);
    SetCoupledPosition(3);
}

/**
 *
 * 检测步态增益是否正确
 * @param  gains LegGain to check
 * @return       True if valid gains, false if invalid
 */
bool IsValidLegGain(LegGain gains)
{
    // check for unstable gains
    bool bad = gains.kp_spd < 0 || gains.kd_spd < 0 ||
               gains.kp_pos < 0 || gains.kd_pos < 0;
    if (bad)
    {
        printf("Invalid gains: <0");
        vTaskDelay(500);
        return false;
    }
    // check for instability / sensor noise amplification
    bad = bad || gains.kp_spd > 50 || gains.kd_spd > 50 ||
          gains.kp_pos > 50 || gains.kd_pos > 50;
    if (bad)
    {
        printf("Invalid gains: too high.");
        vTaskDelay(500);
        return false;
    }
    // check for underdamping -> instability
    bad = bad || (gains.kp_spd > 50 && gains.kd_spd < 0.1);
    bad = bad || (gains.kp_pos > 50 && gains.kd_pos < 0.1);
    if (bad)
    {
        printf("Invalid gains: underdamped");
        vTaskDelay(500);
        return false;
    }
    return true;
}
/**
 *
 * 检测步态参数是否正确
 * @param  gains LegGain to check
 * @return       True if valid gains, false if invalid
 */
bool IsValidGaitParams(GaitParams params)
{
    const float maxL = 30.1;
    const float minL = 9.9;

    float stanceHeight  = params.stance_height;
    float downAMP       = params.down_amp;
    float upAMP         = params.up_amp;
    float flightPercent = params.flight_percent;
    float stepLength    = params.step_length;
    float FREQ          = params.freq;

    if (stanceHeight + downAMP > maxL || sqrt(pow(stanceHeight, 2) + pow(stepLength / 2.0, 2)) > maxL)
    {
        printf("Gait overextends leg");
        vTaskDelay(500);
        return false;
    }
    if (stanceHeight - upAMP < minL)
    {
        printf("Gait underextends leg");
        vTaskDelay(500);
        return false;
    }

    if (flightPercent <= 0 || flightPercent > 1.0)
    {
        printf("Flight percent is invalid");
        vTaskDelay(500);
        return false;
    }

    if (FREQ < 0)
    {
        printf("Frequency cannot be negative");
        vTaskDelay(500);
        return false;
    }

    if (FREQ > 10.0)
    {
        printf("Frequency is too high (>10)");
        vTaskDelay(500);
        return false;
    }

    return true;
}
/**
 *改变腿部增益
 *调用了PID_reset_kpKd 函数
 */
void ChangeTheGainsOfPD(LegGain gains)
{
    for (int i = 0; i < 8; i++)
    {
        pid_reset_kpkd(&pid_pos[i], gains.kp_pos, gains.kd_pos);
        pid_reset_kpkd(&pid_spd[i], gains.kp_spd, gains.kd_spd);
    }
}

/**
* NAME: void CycloidTrajectory (float t, GaitParams params, float gait_offset)
* FUNCTION : 摆线轨迹生成器
*/
//void CycloidTrajectory (float t, GaitParams params, float gait_offset)
//{
//    float S0 = params.step_length;
//    float H0 = params.stance_height;
//    float T = params.gait_cycle;
//    float Ty = params.swing_cycle;
//

////        printf("\r\n t=%f",t);
//    if(t>=0&&t<=Ty)        //足端摆动相
//    {
//        x=S0/(2*PI)*( (2*PI*t/Ty)-sin((2*PI*t/Ty)) );
//        if(t>=0&&t<=Ty/2)
//        {
//            y=2*H0*( t/Ty - sin( 4*PI*t/Ty )/(4*PI) );
//        }
//        else if(t>=Ty/2&&t<=Ty)
//        {
//            y=-2*H0*( t/Ty - sin( 4*PI*t/Ty )/(4*PI) )+2*H0;
//        }
//    }
//    else if(t>=Ty&&t<=T)        //足端支撑相
//    {
//        x=S0 - S0/(2*PI)*( (2*PI*(t-Ty)/(T-Ty))-sin((2*PI*(t-Ty)/(T-Ty))) );
//        y=0;
//    }
//    else
//        vTaskDelay(10);

//    x-=6.0;//将0点设置在穿过电机中心的竖直线上
//    y+=10.0*sqrt(3.0);//电机y初始位置 表示两条腿为水平的时

//}