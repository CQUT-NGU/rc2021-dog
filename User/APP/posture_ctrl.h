#ifndef PostureCtrl_H
#define PostureCtrl_H

#include "robocon.h"

#define L1 10
#define L2 20

#define TransData 3591/187*8191/360

static float x, y, theta1, theta2;

typedef struct
{
    double L;
    double X;
    double Y;
    double M;
    double N;
    double A1;
    double A2;
    double R[4];

} Coordinate_Trans_Data;

extern Coordinate_Trans_Data coor_calc;

enum States {
    STOP = 0,
    PRONK = 1,
    TROT = 2,
    PACE = 3,
    BOUND = 4,
    GALLOP = 5,
    WALK = 6,
    ROTATE = 7,
    WALK_AHEAD=8,
    WALK_BACK=9,
    WALK_LEFT=10,
    WALK_RIGHT=11,
    ROTAT_LEFT=12,
    ROTAT_RIGHT=13,
		JUMP=14
};
extern enum States state;

// �Ȳ�PID����ṹ��
typedef struct  {
    float kp_spd;		//�ٶȻ�
    float kd_spd;

    float kp_pos;		//λ�û�
    float kd_pos;
} LegGain;

typedef struct  {
    float stance_height ; // ��������ľ��� (cm)
    float step_length ; // һ���ľ��� (cm)
    float up_amp ; // �ϲ����y (cm)
    float down_amp ; // �²���� (cm)
    float flight_percent ; // �ڶ���ٷֱ� (cm)
    float freq ; // һ����Ƶ�� (Hz)
} GaitParams;

extern GaitParams gait_params;

#endif
