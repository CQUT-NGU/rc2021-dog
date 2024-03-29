#include "navigation.h"

int LinearCorrection = Deny;

bool BalanceCorrection = Deny;

bool  _Sensor[10];
float step_len_initial  = 14;
float step_high_initial = 16;

float step_len_dev       = 0.0;
float step_high_dev      = 0.0;
float flight_percent_dev = 0.0;

float yaw_now = 0.0;
float yaw_set = 0.0;

float roll_set  = 0.0;
float pitch_set = 0.0;

float roll_offset  = 0.0;
float pitch_offset = 0.0;

float _dev_angel;

int stage = 0;

int _count_navi;

void Navi_task(void *pvParameters)
{
    for (;;)
        navigation_execute();
}

/*------------------------欧拉角奇点---------------------------------

------- -180 --- 0 --- +180 -------

0 --- 180 --- 360

*/

float normal_params_l, normal_params_r;  //左腿参数，右腿参数。
void  navigation_execute(void)
{
    //    _Pitch_rev=_Pitch_initial-imuinfo.ActVal[1];
    //    _Roll_rev=_Roll_initial-imuinfo.ActVal[2];

    if (LinearCorrection == normal_correction)  //直线矫正
    {
        step_len_dev = pid_calc(&pid_imu[2], imuinfo.ActVal[0] / 1.0, yaw_set / 1.0);  //计算陀螺仪yaw轴角度，输出给两腿腿部差速 保持角度

        float _dev_limit = 14.0;
        if (step_len_dev > _dev_limit)
            step_len_dev = _dev_limit;
        else if (step_len_dev < -_dev_limit)
            step_len_dev = -_dev_limit;

        normal_params_l = step_len_initial - step_len_dev;
        normal_params_r = step_len_initial + step_len_dev;
        if (normal_params_l < 0)
            normal_params_l = 0;
        if (normal_params_r < 0)
            normal_params_r = 0;

        state_detached_params[TROT].detached_params_0.step_length = normal_params_l;
        state_detached_params[TROT].detached_params_2.step_length = normal_params_l;
        state_detached_params[TROT].detached_params_1.step_length = normal_params_r;
        state_detached_params[TROT].detached_params_3.step_length = normal_params_r;

        //                flight_percent_dev=pid_calc(&pid_imu[2],imuinfo.ActVal[0]/100,yaw_set/100);

        //        if(flight_percent_dev>0.25)    flight_percent_dev=0.25;
        //        else if(flight_percent_dev<-0.25)    flight_percent_dev=-0.25;

        //        state_detached_params[TROT].detached_params_0.flight_percent=0.25+flight_percent_dev;
        //        state_detached_params[TROT].detached_params_2.flight_percent=0.25+flight_percent_dev;

        //        state_detached_params[TROT].detached_params_1.flight_percent=0.25-flight_percent_dev;
        //        state_detached_params[TROT].detached_params_3.flight_percent=0.25-flight_percent_dev;
    }

    else if (LinearCorrection == test1_correction)
    {
        //test1步态 纠偏
        step_len_dev = pid_calc(&pid_test1, imuinfo.ActVal[0] / 1.0, yaw_set / 1.0);  //计算陀螺仪yaw轴角度，输出给两腿腿部差速 保持角度

        float _dev_limit = 6.0;
        if (step_len_dev > _dev_limit)
            step_len_dev = _dev_limit;
        else if (step_len_dev < -_dev_limit)
            step_len_dev = -_dev_limit;

        state_detached_params[TEST1].detached_params_0.step_length = 10 - step_len_dev;
        state_detached_params[TEST1].detached_params_2.step_length = 10 - step_len_dev;

        state_detached_params[TEST1].detached_params_1.step_length = 10 + step_len_dev;
        state_detached_params[TEST1].detached_params_3.step_length = 10 + step_len_dev;
    }

    else if (LinearCorrection == climbing_correction)
    {
        //        step_len_dev=pid_calc(&pid_climbing,imuinfo.ActVal[0]/1.2,yaw_set/1.2);  //计算陀螺仪yaw轴角度，输出给两腿腿部差速 保持角度

        //        float _dev_limit = 10.0;
        //        if(step_len_dev>_dev_limit)    step_len_dev=_dev_limit;
        //        else if(step_len_dev<-_dev_limit)    step_len_dev=-_dev_limit;

        //        normal_params_l=10-step_len_dev;
        //        normal_params_r=10+step_len_dev;
        //        if(normal_params_l<0) normal_params_l=0;
        //        if(normal_params_r<0) normal_params_r=0;

        //        state_detached_params[CLIMBING].detached_params_0.step_length=normal_params_l;
        //        state_detached_params[CLIMBING].detached_params_2.step_length=normal_params_l;
        //        state_detached_params[CLIMBING].detached_params_1.step_length=normal_params_r;
        //        state_detached_params[CLIMBING].detached_params_3.step_length=normal_params_r;

        flight_percent_dev = pid_calc(&pid_climbing, imuinfo.ActVal[0] / 100, yaw_set / 100);

        if (flight_percent_dev > 0.25)
            flight_percent_dev = 0.25;
        else if (flight_percent_dev < -0.25)
            flight_percent_dev = -0.25;

        //                    if(flight_percent_dev>0.25)    flight_percent_dev=0.25;
        //        else if(flight_percent_dev<-0.25)    flight_percent_dev=-0.25;

        state_detached_params[CLIMBING].detached_params_0.flight_percent = 0.25 - flight_percent_dev;  // +
        state_detached_params[CLIMBING].detached_params_2.flight_percent = 0.25 - flight_percent_dev;  //

        state_detached_params[CLIMBING].detached_params_1.flight_percent = 0.25 + flight_percent_dev;  //
        state_detached_params[CLIMBING].detached_params_3.flight_percent = 0.25 + flight_percent_dev;  //
    }

    if (BalanceCorrection == Permit)  //平衡矫正
    {
        if (!(state == STOP))
        {  //只有在STOP状态下能够开启平衡矫正
            printf("不在STOP状态\r\n");
            return;
        }

        roll_offset = 15.0 * sin(pid_calc(&pid_imu[0], imuinfo.ActVal[2] * PI / 180, roll_set));  //计算陀螺仪roll轴角度，输出腿部高度修正值

        pitch_offset = 22.5 * sin(pid_calc(&pid_imu[1], -(_Pitch_rev * PI / 180) / 2, pitch_set / 2));  //计算陀螺仪pitch轴角度，输出腿部高度修正值

        x = 0;
        y = state_detached_params[TROT].detached_params_0.stance_height + roll_offset + pitch_offset;
        if (y > 29.5)
            y = 29.5;
        else if (y < 10.5)
            y = 10.5;
        CartesianToTheta(1.0);
        temp_pid.ref_agle[1] = -theta1 * ReductionAndAngleRatio;
        temp_pid.ref_agle[0] = -theta2 * ReductionAndAngleRatio;

        x = 0;
        y = state_detached_params[TROT].detached_params_1.stance_height - roll_offset + pitch_offset;
        if (y > 29.5)
            y = 29.5;
        else if (y < 10.5)
            y = 10.5;
        CartesianToTheta(1.0);
        temp_pid.ref_agle[2] = theta1 * ReductionAndAngleRatio;
        temp_pid.ref_agle[3] = theta2 * ReductionAndAngleRatio;

        x = 0;
        y = state_detached_params[TROT].detached_params_2.stance_height + roll_offset - pitch_offset;
        if (y > 29.5)
            y = 29.5;
        else if (y < 10.5)
            y = 10.5;
        CartesianToTheta(1.0);
        temp_pid.ref_agle[5] = -theta1 * ReductionAndAngleRatio;
        temp_pid.ref_agle[4] = -theta2 * ReductionAndAngleRatio;

        x = 0;
        y = state_detached_params[TROT].detached_params_3.stance_height - roll_offset - pitch_offset;
        if (y > 29.5)
            y = 29.5;
        else if (y < 10.5)
            y = 10.5;
        CartesianToTheta(1.0);
        temp_pid.ref_agle[6] = theta1 * ReductionAndAngleRatio;
        temp_pid.ref_agle[7] = theta2 * ReductionAndAngleRatio;
        IsMotoReadyOrNot     = IsReady;  //数据填充完毕
    }

    vTaskDelay(200);
}
