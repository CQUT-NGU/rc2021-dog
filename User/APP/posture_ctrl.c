
#include "posture_ctrl.h"

void gait(	GaitParams params,LegGain gait_gains,
            float leg0_offset, float leg1_offset,float leg2_offset, float leg3_offset,
            float leg0_direction, float leg1_direction,float leg2_direction, float leg3_direction);
void gait_detached(	GaitParams params_left,GaitParams params_right,
                    float leg0_offset, float leg1_offset,float leg2_offset, float leg3_offset,
                    float leg0_direction, float leg1_direction,float leg2_direction, float leg3_direction);
void CoupledMoveLeg(float t, GaitParams params,float gait_offset, float leg_direction, int LegId);
void CycloidTrajectory (float t, GaitParams params, float gait_offset) ;
void SinTrajectory (float t, GaitParams params, float gait_offset) ;
void CartesianToTheta(float leg_direction) ;
void SetCoupledPosition(int LegId);
void CommandAllLegs(void);

static  float x;
static  float y;
static  float theta1;
static  float theta2;

enum States state = STOP;
//Coordinate_Trans_Data coor_calc;
extern TaskHandle_t MotorControlTask_Handler;
extern void pid_reset_kpkd(pid_t*pid, float kp, float kd);
//��ʼ����̬����
GaitParams gait_params = {17.3,12.0,4.0,0.00,0.25,1};
//��ʼ����̬����
LegGain gait_gains = {15.5, 0.00, 25.0, 0.00};
LegGain state_gait_gains[] = {
    //{kp_spd, kd_spd, kp_pos, kd_pos}
    {15.5, 0.00, 25.0, 0.00}, // STOP
    {15.5, 0.00, 25.0, 0.00}, // PRONK
    {15.5, 0.00, 25.0, 0.00},// TROT
    {15.5, 0.00, 25.0, 0.00}, // PACE
    {15.5, 0.00, 25.0, 0.00}, // BOUND
    {15.5, 0.00, 25.0, 0.00}, // GALLOP
    {15.5, 0.00, 25.0, 0.00}, // WALK
    {15.5, 0.00, 25.0, 0.00}, // ROTATE
    {15.5, 0.00, 25.0, 0.00}, // WALK		ahead
    {15.5, 0.00, 25.0, 0.00}, // WALK		back
    {15.5, 0.00, 25.0, 0.00}, // WALK		left
    {15.5, 0.00, 25.0, 0.00}, // WALK		right
    {15.5, 0.00, 25.0, 0.00}, // ROTATE		left
    {15.5, 0.00, 25.0, 0.00}, // ROTATE		right
};
//�趨ÿһ�ֲ�̬�ľ������
GaitParams state_gait_params[] = {
    //{stance_height,step_length,up_amp,down_amp,flight_percent,freq}  cm
    {NAN, NAN, NAN, NAN, NAN, NAN}, // STOP
    {16.0, 0.00, 0.00, 5.00, 0.75, 1.0}, // PRONK		//���Ĳ�̬ ������Ծ
    {16.0, 15.0, 6.00, 2.00, 0.25, 2.0}, // TROT		//˫�Ĳ�̬ �Խ�С��
    {17.0, 15.0, 6.00, 4.00, 0.35, 2.0}, // PACE		//˫�Ĳ�̬ ͬ���ﲽ
    {17.0, 0.00, 6.00, 4.00, 0.35, 2.0}, // BOUND		//˫�Ĳ�̬ ����
    {17.0, 0.00, 6.00, 4.00, 0.35, 2.0}, // GALLOP		//׼���Ĳ�̬ ��Ծ

    {15.3, 0.00, 5.0, 0.00, 0.10, 1.5}, // WALK		//���Ĳ�̬ ��������
    {17.3, 10.0, 4.0, 0.00, 0.35, 2.0}, // ROTATE		//��ת

    {16.0, 16.00, 5.0, 0.00, 0.20, 2.0}, // WALK		ahead
    {16.0, 8.00, 5.0, 0.00, 0.25, 2.0}, // WALK		back

    {17.3, 10.0, 6.0, 0.00, 0.35, 2.0}, // WALK		left
    {17.3, 10.0, 6.0, 0.00, 0.35, 2.0}, // WALK		right

    {15.3, 12.00, 5.0, 0.00, 0.25, 2.5}, // ROTATE		left
    {15.3, 12.00, 5.0, 0.00, 0.25, 2.5} // ROTATE		right
};

/*���walk {12.0, 15.0, 1.8, 0.00, 0.50, 1.0}
*Ч������{14.0, 12.0, 3.5, 0.00, 0.50, 1.0},
*
*  {16.0, 12.00, 5.0, 0.00, 0.20, 2.0}, // WALK		ahead ����
*  {17.3, 12.00, 4.0, 0.00, 0.35, 2.5}, // WALK		ahead
*
*/
long rotate_start = 0; // milliseconds when rotate was commanded

GaitParams gait_params_1 = {16.0, 16.00, 5.0, 0.00, 0.50, 2.0};
GaitParams gait_params_2 = {16.0, 16.00, 5.0, 0.00, 0.20, 2.0};


/*******************************************************************************************
	*@ �������ƣ�void PostureControl_task(void *pvParameters)
	*@ ���ܣ���̬����
	*@ ��ע��ѡ��״̬
 *******************************************************************************************/
void PostureControl_task(void *pvParameters)
{
    for(;;)
    {
        GaitParams gait_params = state_gait_params[state];
        LegGain gait_gains =state_gait_gains[state] ;
        switch(state) {
        case STOP:		//ֹͣ
            x=0;
            y = 17.3205081;
            CartesianToTheta(1.0);
            CommandAllLegs();
            // vTaskDelay(10);
            //printf("\r\n x=%f  y=%f  theta1=%f  theta2=%f   he=%f ",x,y,theta1,theta2,theta1+theta2);
            break;
        case PRONK:		//���Ĳ�̬ ������Ծ
            gait(gait_params, gait_gains, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0);
            break;
        case TROT:		//˫�Ĳ�̬ �Խ�С��
            gait(gait_params, gait_gains, 0.0, 0.5, 0.5, 0.0, 1.0, 1.0, 1.0, 1.0);
            break;
        case PACE:		//˫�Ĳ�̬ ͬ���ﲽ
            gait(gait_params, gait_gains, 0.5, 0.0, 0.5, 0.0, 1.0, 1.0, 1.0, 1.0);
            break;
        case BOUND:		//˫�Ĳ�̬ ����
            gait(gait_params, gait_gains, 0.0, 0.0, 0.5, 0.5, 1.0, 1.0, 1.0, 1.0);
            break;
        case GALLOP:	//׼���Ĳ�̬ ��Ծ
            gait(gait_params, gait_gains, 0.0, 0.1, 0.5, 0.6, 1.0, 1.0, 1.0, 1.0);
            break;
        case WALK:		//���Ĳ�̬ ��������
            gait(gait_params, gait_gains, 0.0, 0.5, 0.75, 0.25, 1.0, 1.0, 1.0, 1.0);
            //	printf("\r\n x=%f  y=%f  theta1=%f  theta2=%f   he=%f ",x,y,theta1,theta2,theta1+theta2);
            break;
        case ROTATE:	//��ת
            // printf("\r\n ROTATE");
            //float theta;
            //x=0;
            //y=0.24;
            //CartesianToTheta(1.0);
            //float freq = 0.1;
            //float phase = freq * (times - rotate_start)/1000.0f;
            //theta = (-cos(2*PI * phase) + 1.0`f) * 0.5 * 2 * PI;
            //CommandAllLegs();

            gait(gait_params, gait_gains, 0.0, 0.5, 0.75, 0.25, -1.0, -1.0, -1.0, -1.0);

            //	printf("\r\n x=%f  y=%f  theta1=%f  theta2=%f   he=%f ",x,y,theta1,theta2,theta1+theta2);
            break;
        case WALK_AHEAD:		//ǰ��
            gait(gait_params, gait_gains, 0.0, 0.5, 0.75, 0.25, 1.0, 1.0, 1.0, 1.0);
            break;
        case WALK_BACK:		//����
            gait(gait_params, gait_gains, 0.00, 0.5, 0.75, 0.25, -1.0, -1.0, -1.0, -1.0);
            break;
        case WALK_LEFT:		//����ƫ��
            gait_detached(gait_params_1, gait_params_2, 0.0, 0.5, 0.75, 0.25, 1.0, 1.0, 1.0, 1.0);
            break;
        case WALK_RIGHT:		//����ƫ��
            gait_detached(gait_params_2, gait_params_1, 0.0, 0.5, 0.75, 0.25, 1.0, 1.0, 1.0, 1.0);
            break;
        case ROTAT_LEFT:		//ԭ����ת
            gait(gait_params, gait_gains, 0.0, 0.5, 0.5, 0.00, -1.0, 1.0, -1.0, 1.0);
            break;
        case ROTAT_RIGHT:		//ԭ����ת
            gait(gait_params, gait_gains, 0.0, 0.5, 0.5, 0.00, 1.0, -1.0, 1.0, -1.0);
            break;

        }

    }
}



void gait_detached(	GaitParams params_left,GaitParams params_right,
                    float leg0_offset, float leg1_offset,float leg2_offset, float leg3_offset,
                    float leg0_direction, float leg1_direction,float leg2_direction, float leg3_direction) {

    float t = times/1000.0;

    //printf("\r\n t=%f",t);
    // const float leg0_direction = 1.0;
    CoupledMoveLeg( t, params_left, leg0_offset, leg0_direction, 0);

    // const float leg1_direction = 1.0;
    CoupledMoveLeg( t, params_right, leg1_offset, leg1_direction, 1);

    // const float leg2_direction = 1.0;
    CoupledMoveLeg( t, params_left, leg2_offset, leg2_direction, 2);

    //  const float leg3_direction = 1.0;
    CoupledMoveLeg( t, params_right, leg3_offset, leg3_direction, 3);

}

/**
* NAME: void gait(	GaitParams params,float leg0_offset, float leg1_offset,float leg2_offset, float leg3_offset)
* FUNCTION : ����ʱ������ �趨ÿ���ȵĲ��� �����ȵ����з��� ���в���
*/
void gait(	GaitParams params,LegGain gait_gains,
            float leg0_offset, float leg1_offset,float leg2_offset, float leg3_offset,
            float leg0_direction, float leg1_direction,float leg2_direction, float leg3_direction) {

    float t = times/1000.0;

    //printf("\r\n t=%f",t);
    // const float leg0_direction = 1.0;
    CoupledMoveLeg( t, params, leg0_offset, leg0_direction, 0);

    // const float leg1_direction = 1.0;
    CoupledMoveLeg( t, params, leg1_offset, leg1_direction, 1);

    // const float leg2_direction = 1.0;
    CoupledMoveLeg( t, params, leg2_offset, leg2_direction, 2);

    //  const float leg3_direction = 1.0;
    CoupledMoveLeg( t, params, leg3_offset, leg3_direction, 3);

//    for (int i = 0; i < 8; i++)
//        pid_reset_kpkd(&pid_pos[i],gait_gains.kp_pos,gait_gains.kd_pos);

//    for (int i = 0; i < 8; i++)
//        pid_reset_kpkd(&pid_spd[i],gait_gains.kp_spd,gait_gains.kd_spd);
}

/**
* NAME: void CoupledMoveLeg(float t, GaitParams params,float gait_offset, float leg_direction, int LegId)
* FUNCTION : ���������� ���ݲ���
*/
void CoupledMoveLeg(float t, GaitParams params,float gait_offset, float leg_direction, int LegId)
{
    SinTrajectory(t, params, gait_offset);		//��˰��߹켣������
    CartesianToTheta(leg_direction);		//�ѿ�������ת����٤������
    SetCoupledPosition(LegId);		//�������ݸ������������
//    printf("\r\nt=%f x=%f  y=%f  theta1=%f  theta2=%f  legid=%d he%f",t,x,y,theta1,theta2,LegId,theta1+theta2);
}

/**
* NAME: void CycloidTrajectory (float t, GaitParams params, float gait_offset)
* FUNCTION : ���߹켣������
*/
//void CycloidTrajectory (float t, GaitParams params, float gait_offset)
//{
//    float S0 = params.step_length;
//    float H0 = params.stance_height;
//    float T = params.gait_cycle;
//    float Ty = params.swing_cycle;
//

////		printf("\r\n t=%f",t);
//    if(t>=0&&t<=Ty)		//��˰ڶ���
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
//    else if(t>=Ty&&t<=T)		//���֧����
//    {
//        x=S0 - S0/(2*PI)*( (2*PI*(t-Ty)/(T-Ty))-sin((2*PI*(t-Ty)/(T-Ty))) );
//        y=0;
//    }
//    else
//        vTaskDelay(10);

//    x-=6.0;//��0�������ڴ���������ĵ���ֱ����
//    y+=10.0*sqrt(3.0);//���y��ʼλ�� ��ʾ������Ϊˮƽ��ʱ

//}

/**
* NAME: SinTrajectory (float t,GaitParams params, float gaitOffset)
* FUNCTION : ���ҹ켣������
*/
void SinTrajectory (float t,GaitParams params, float gaitOffset) {
    static float p = 0;
    static float prev_t = 0;

    float stanceHeight = params.stance_height;
    float downAMP = params.down_amp;
    float upAMP = params.up_amp;
    float flightPercent = params.flight_percent;
    float stepLength = params.step_length;
    float FREQ = params.freq;

    p += FREQ * (t - prev_t);
    prev_t = t;

    float gp = fmod((p+gaitOffset),1.0);
    if (gp <= flightPercent) {
        x = (gp/flightPercent)*stepLength - stepLength/2.0;
        y = -upAMP*sin(PI*gp/flightPercent) + stanceHeight;
    }
    else {
        float percentBack = (gp-flightPercent)/(1.0-flightPercent);
        x = -percentBack*stepLength + stepLength/2.0;
        y = downAMP*sin(PI*percentBack) + stanceHeight;
    }

    //  printf("\r\nt=%f x=%f y=%f",t,x,y);

}

/**
* NAME: void CartesianToThetaGamma(float leg_direction)
* FUNCTION : �ѿ�������ת����٤������ Ҳ���ǽ�thetaת����XY
*/
void CartesianToTheta(float leg_direction)
{
    float L=0;
    float N=0;
    double M=0;
    float A1=0;
    float A2=0;

    L=sqrt(		pow(x,2)	+		pow(y,2)	);
    if(L<9.8||L>29.8)
        vTaskSuspend(MotorControlTask_Handler);
    N=asin(x/L)*180.0/PI;
    M=acos(	(pow(L,2)+pow(L1,2)-pow(L2,2))/(2*L1*L)	)*180.0/PI;
    A1=M-N;

    A2=M+N;
    if(leg_direction==1.0) {
        theta2=(A1-90.0);
        theta1=(A2-90.0);
    } else if(leg_direction==-1.0) {
        theta1=(A1-90.0);
        theta2=(A2-90.0);
    }
    //	printf("\r\n x=%f  y=%f  theta1=%f  theta2=%f   he=%f   L=%f  M=%lf  N=%f   SIXI=%f",x,y,theta1,theta2,theta1+theta2,L,M,N,(	(pow(L,2)+pow(L1,2)-pow(L2,2))/(2*L1*L)	));
}

/**
* NAME: void SetCoupledPosition( int LegId)
* FUNCTION : ���͵�����ƽǶ�
*/
void SetCoupledPosition( int LegId)
{
//
//		pid_reset(pid_t	*pid, float kp, float ki, float kd)
//

    if((theta1+theta2)>179||(theta1+theta2)<-179||theta1>150||theta1<-150||theta2>150||theta2<-150)
        vTaskSuspend(MotorControlTask_Handler);

    if(LegId==0)
    {
        temp_pid.ref_agle[1]=-theta1*TransData;
        temp_pid.ref_agle[0]=-theta2*TransData;
    }
    else if(LegId==1)
    {
        temp_pid.ref_agle[2]=theta1*TransData;
        temp_pid.ref_agle[3]=theta2*TransData;
    }
    else if(LegId==2)
    {
        temp_pid.ref_agle[4]=-theta1*TransData;
        temp_pid.ref_agle[5]=-theta2*TransData;
    }
    else if(LegId==3)
    {
        temp_pid.ref_agle[6]=theta1*TransData;
        temp_pid.ref_agle[7]=theta2*TransData;
    }
    Ready_Flag=1;		//����������

}


void CommandAllLegs(void)
{
    SetCoupledPosition(0);
    SetCoupledPosition(1);
    SetCoupledPosition(2);
    SetCoupledPosition(3);
}