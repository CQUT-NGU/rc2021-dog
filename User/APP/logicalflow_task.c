/**
  ****************************(C) COPYRIGHT 2016 DJI****************************
  * @file       logicalflow_task.c/h
  * @brief      
  ==============================================================================
  **************************** HBUT ROBOCON 2019****************************
  */

#include "logicalflow_task.h"


/*********�������˿�˵��********************
*
*���˵����1�ţ����˶����H:PD12		2�ţ����ƶ����G:PD13		3�ţ����������F:PD14
*
*��λ���� ���Ƽ��A:PI0		����B:PH12		����1 U:PA2		����2 V:PA3		(����3 W:PI5)	ռʱû���õ� chongqi climb C : PH11
*��翪�أ����ֿ�ʼ���£����1 S:PA0 ���2 T:PA1
*
*ָʾLED Z PI2
**/
//#define keyToken HAL_GPIO_ReadPin(GPIOI,GPIO_PIN_0)
//#define keyStart HAL_GPIO_ReadPin(GPIOH,GPIO_PIN_12)

//#define keyRestart1 HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_2)
//#define keyRestart2 HAL_GPIO_ReadPin(GPIOA,GPazIO_PIN_3)
//#define keyRestart3 HAL_GPIO_ReadPin(GPIOI,GPIO_PIN_5)
//#define keyRestartclimb HAL_GPIO_ReadPin(GPIOH,GPIO_PIN_11)

//#define keyInf1 HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0)
//#define keyInf2 HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_1)

//#define RED_GROUNG 0
//#define BLUE_GROUNG 1

//bool GROUND_SELECT=BLUE_GROUNG;  //������ѡ��

/**
*		���������������߼����� �߼�������
*/
void LogicalFlow_task(void *pvParameters)
{
    for(;;) {

        IndicateLED_Off;

        //KeyToken_Test();

        while(1)
        {
            vTaskDelay(500);
            if(keyStart==0)//�ȴ���λ���ذ���ȥ ����B:PH12
            {
                state = START;
                break;
            }
            else if(keyRestart1==0) //����1 U:PA2
                goto restart1;

            else if(keyRestart2==0) //����2 V:PA3
                goto restart2;

            else if(keyRestartclimb==0) //����climb
						{
							Servo3_CLOSE;
							Servo2_DOWN_POS;
							state = STOP;
							    state_detached_params[CLIMBING].detached_params_0.stance_height=16;
    state_detached_params[CLIMBING].detached_params_1.stance_height=16;
                goto restartclimb;
						}
        }


        while((keyInf2!=0)) //�ȴ���λ���ذ���ȥ �������� A  PI0
            vTaskDelay(500);

        IndicateLED_On;

        vTaskDelay(1000);  				//�ȴ� mr1��



        //Climbing_Comb();  //���²��Գ���

restart1:

        state = STOP;
        vTaskDelay(800);

        IndicateLED_Off;

        yaw_set=0;  //------������趨Ϊ0
        LinearCorrection=normal_correction;		//��ֱ�߽���
        //now_time=times;
        state = TROT;		//С�ܲ�̬


        vTaskDelay(1000);

        OpenMvInspect(openmv_Yellow);  // -------- �ȴ���⵽ת�� ��ɫ --ɳ��

        IndicateLED_On;

        state= STOP ;
        vTaskDelay(200);

        if(GROUND_SELECT==RED_GROUNG) {//�������ж� �쳡
            _rotate_angle=10;
            yaw_set=10;		//----------------------������趨 10 ����0
            //now_time=times;
            state = TEST7;  //--------С�鲽ת�� ��
            while(imuinfo.ActVal[0]<=_rotate_angle)		//ת��45��֮���Զ�ֹͣ 10
                osDelay(50);
        }
        else if(GROUND_SELECT==BLUE_GROUNG) {
            _rotate_angle=-10;
            yaw_set=-10;		//----------------------������趨 -10 ����0
            now_time=times;
            state = TEST8;  //--------С�鲽ת�� ��
            while(imuinfo.ActVal[0]>=_rotate_angle)		//ת��45��֮���Զ�ֹͣ 10
                osDelay(50);
        }

        IndicateLED_Off;

        state=TEST6;				// --��ɳ��ǰ  С�鲽��һ��

        OpenMvInspect(openmv_Yellow);

        vTaskDelay(1000);
        state=REALSE;
        vTaskDelay(400);

        //---------------------Խɳ��---------------------//
        StepOver();

        // now_time=times;
        state=TROT;		    //-------------------С������ɳ��
        vTaskDelay(800);

        state=REALSE;
        vTaskDelay(250);
        state=STOP;
        vTaskDelay(500);

        if(GROUND_SELECT==RED_GROUNG) {  //�������ж� �쳡
            _rotate_angle=14;		//---------------------ת��
            yaw_set=14;  //----------------������趨 14 ����0
            LinearCorrection=normal_correction;		//��ֱ�߽���
            now_time=times;
            state = ROTAT_LEFT;
            while(imuinfo.ActVal[0]<=_rotate_angle)
                osDelay(50);
        }
        else if(GROUND_SELECT==BLUE_GROUNG) {
            _rotate_angle=-14;		//---------------------ת��
            yaw_set=-14;  //----------------������趨 14 ����0
            LinearCorrection=normal_correction;		//��ֱ�߽���
            now_time=times;
            state = ROTAT_RIGHT;
            while(imuinfo.ActVal[0]>=_rotate_angle)
                osDelay(50);
        }

restart2:

        state= STOP;
        vTaskDelay(400);


        if(GROUND_SELECT==RED_GROUNG)  //�������ж� �쳡
            yaw_set=14;  //----------------������趨 14 ����0
        else if(GROUND_SELECT==BLUE_GROUNG)
            yaw_set=-14;  //----------------������趨 14 ����0

        LinearCorrection=normal_correction;		//��ֱ�߽���
        now_time=times;

        state= TROT;		//������ֱ��ת����


        vTaskDelay(1600);


        OpenMvInspect(openmv_Yellow); //�ȴ���⵽��ɫ ɫ��----------Խ���Ӽ��

        IndicateLED_On;

        state=REALSE;

        if(GROUND_SELECT==RED_GROUNG) {  //�������ж� �쳡
            _rotate_angle=0-0;		//-------------ת��-----------��������-------
            yaw_set=0-0;
            state = ROTAT_RIGHT;
            while(imuinfo.ActVal[0]>=_rotate_angle)
                osDelay(20);
        }
        else if(GROUND_SELECT==BLUE_GROUNG) {
            _rotate_angle=0+0;		//-------------ת��-----------��������-------
            yaw_set=0+0;
            state = ROTAT_LEFT;
            while(imuinfo.ActVal[0]<=_rotate_angle)
                osDelay(20);
        }

        state=REALSE;

        IndicateLED_Off;

        state=TEST6;				//С�鲽
        vTaskDelay(2500);

        state=REALSE;

        //---------------------������---------------------//
        LinearCorrection=test1_correction;		//�򿪿�����ʱ��Ĳ�̬��ƫ
        CrossTheLine();

        yaw_set=0-0; //������趨 0 ����-0
        now_time=times;
        state=TEST1;
        vTaskDelay(1500);

restartclimb:
				
        state= STOP;

        //...����------------
        Climbing_Comb();


        while(1)		//ͣ
            vTaskDelay(500);


    }
}
