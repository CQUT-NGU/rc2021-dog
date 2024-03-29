#ifndef MOTO_CTRL_H
#define MOTO_CTRL_H

#include "robocon.h"

#define IsReady  1
#define NotReady 0

#define Lm298n_break    0
#define Lm298n_forward  1
#define Lm298n_backward 2

void moto_behaviour(void);
void pid_param_init(void);
void coordinate_trans(float X, float Y);

typedef struct
{
    float ref_agle[8];
    float out[8];
} temp_data;

extern u16       pid_spd_out_limit;
extern bool      IsMotoReadyOrNot;
extern float     ref_agle[8];
extern temp_data temp_pid;
extern float     test_speed;

void send_chassis_cur1_4(int16_t motor1, int16_t motor2, int16_t motor3, int16_t motor4);
void send_chassis_cur5_8(int16_t motor5, int16_t motor6, int16_t motor7, int16_t motor8);

void Lm298n_pin_init(void);
void Lm298n_ctrl(int mode);

#endif
