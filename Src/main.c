
#include "robocon.h"

#define START_TASK_PRIO		1		//�������ȼ�
#define START_STK_SIZE 		128  		//�����ջ��С	
TaskHandle_t StartTask_Handler;		//������
void start_task(void *pvParameters);		//������

#define MotorControl_TASK_PRIO		4		//�������ȼ�
#define MotorControl_STK_SIZE 		1024  //�����ջ��С
TaskHandle_t MotorControlTask_Handler;		//������
void MotorControl_task(void *pvParameters);	//������

#define Debug_TASK_PRIO		4		//�������ȼ�
#define Debug_STK_SIZE 		512  //�����ջ��С	
TaskHandle_t DebugTask_Handler;		//������
void Debug_task(void *pvParameters);		//������

#define PostureControl_TASK_PRIO		4		//�������ȼ�
#define PostureControl_STK_SIZE 		512 	 //�����ջ��С	
TaskHandle_t PostureControlTask_Handler;		//������
void PostureControl_task(void *pvParameters);		//������

#define TASK4_TASK_PRIO		4		//�������ȼ�
#define TASK4_STK_SIZE 		128 	 //�����ջ��С	
TaskHandle_t Task4Task_Handler;		//������
void task4_task(void *pvParameters);		//������

#define TEST_TASK_PRIO		4		//�������ȼ�
#define TEST_STK_SIZE 		512 	 //�����ջ��С	
TaskHandle_t TestTask_Handler;		//������
void Test_task(void *pvParameters);		//������

#define VcanGC_TASK_PRIO		4		//�������ȼ�
#define VcanGC_STK_SIZE 		512 	 //�����ջ��С	
TaskHandle_t VcanGCTask_Handler;		//������
void VcanGC_task(void *pvParameters);		//������


void SystemClock_Config(void);
static void MX_NVIC_Init(void);

int main(void)
{
    HAL_Init();						//Hal���ʼ��
    SystemClock_Config();	//ϵͳʱ�ӳ�ʼ��
    MX_GPIO_Init();				//GPIO��ʼ��
		MX_DMA_Init();
    MX_CAN1_Init();				//CAN1�ӿڳ�ʼ��
		MX_CAN2_Init();
    MX_SPI5_Init();				//spi5��ʼ��
    MX_USART2_UART_Init();//usart2���ڳ�ʼ��
		MX_UART7_Init();
		MX_NVIC_Init();
		//��ʱ��ʱ��90M��Ƶϵ��9000-1,��ʱ��3��Ƶ��90M/9000=10K�Զ���װ��10-1,��ʱ������1ms
    TIM3_Init(10-1,9000-1);

    moto_param_init();		//���PID������ʼ��
    buzzer_init(500-1, 90-1);//��������ʼ��
    led_configuration();	//��ˮ�� ���̵Ƴ�ʼ��
		//f=Tck/(psc+1)*(arr+1) ��ʱ��ʱ��Ϊ90M  50Hz=180MHz/(3000*1200)   330HZ=180MHZ/(3000*182)
    Servo_Init(20000-1,90-1);		
	
//	HAL_UART_Transmit_DMA(&huart2, (uint8_t*)res, 4);
//	HAL_UART_Receive_DMA(&huart2, USART2RxBuf, USART2RXBUFSIZE);
//	mpu_device_init();
//	init_quaternion();
//	Servo1_DOWN;
//	Servo2_DOWN_POS;

    Servo1_TEST_POSITION;
    Servo2_TEST_POSITION;
		printf("system init\r\n");
		
    //������ʼ����
    xTaskCreate((TaskFunction_t )start_task,            //������
                (const char*    )"start_task",          //��������
                (uint16_t       )START_STK_SIZE,        //�����ջ��С
                (void*          )NULL,                  //���ݸ��������Ĳ���
                (UBaseType_t    )START_TASK_PRIO,       //�������ȼ�
                (TaskHandle_t*  )&StartTask_Handler);   //������
    vTaskStartScheduler();          //�����������

}

//��ʼ����������
void start_task(void *pvParameters)
{

    taskENTER_CRITICAL();           //�����ٽ���
    //����MotorControl_task		
    xTaskCreate((TaskFunction_t )MotorControl_task,
                (const char*    )"MotorControl_task",
                (uint16_t       )MotorControl_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )MotorControl_TASK_PRIO,
                (TaskHandle_t*  )&MotorControlTask_Handler);
    //����Debug_task			
    xTaskCreate((TaskFunction_t )Debug_task,
                (const char*    )"Debug_task",
                (uint16_t       )Debug_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )Debug_TASK_PRIO,
                (TaskHandle_t*  )&DebugTask_Handler);
    //����PostureControl_task
    xTaskCreate((TaskFunction_t )PostureControl_task,
                (const char*    )"PostureControl_task",
                (uint16_t       )PostureControl_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )PostureControl_TASK_PRIO,
                (TaskHandle_t*  )&PostureControlTask_Handler);
    //����TASK4����
    xTaskCreate((TaskFunction_t )task4_task,
                (const char*    )"task4_task",
                (uint16_t       )TASK4_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )TASK4_TASK_PRIO,
                (TaskHandle_t*  )&Task4Task_Handler);
    //����Test����
    xTaskCreate((TaskFunction_t )Test_task,
                (const char*    )"Test_task",
                (uint16_t       )TEST_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )TEST_TASK_PRIO,
                (TaskHandle_t*  )&TestTask_Handler);
 //����VcanGC����
    xTaskCreate((TaskFunction_t )VcanGC_task,
                (const char*    )"VcanGC_task",
                (uint16_t       )VcanGC_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )VcanGC_TASK_PRIO,
                (TaskHandle_t*  )&VcanGCTask_Handler);
								
    vTaskDelete(StartTask_Handler); //ɾ����ʼ����
    taskEXIT_CRITICAL();            //�˳��ٽ���

}
uint32_t Timetick1ms = 0;


void Test_task(void *pvParameters)
{

	int count=0;
    for(;;) {

//	mpu_get_data();
//	imu_ahrs_update();
//	imu_attitude_update();
//	HAL_Delay(5);
//	printf(" Roll:  \n");
//	//HAL_UART_Transmit(&huart6, (uint8_t *)buf, (COUNTOF(buf)-1), 55);
//	HAL_Delay(5);
//
//	Servo_DOWN;
			
			
		wave_form_data[0] =count;
		wave_form_data[1] =count;
		wave_form_data[2] =count;
		count+=10;
    vTaskDelay(500);
			
			
//		Servo_PEAK;

    }


//        printf("\r\ngiven_current%d given_current2=%d �¶�1=%d �Ƕ�=%d\r\n ",moto_chassis[2].given_current,moto_chassis[3].given_current,moto_chassis[2].hall,moto_chassis[3].total_angle);
//        printf("\r\n t=%llu",time);
//        printf("\r\n x=%f y=%f",x,y);
    //  vTaskDelay(500);
}



void SystemClock_Config(void)
{

    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_ClkInitTypeDef RCC_ClkInitStruct;

    __HAL_RCC_PWR_CLK_ENABLE();

    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 6;
    RCC_OscInitStruct.PLL.PLLN = 180;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_PWREx_EnableOverDrive() != HAL_OK)
    {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                  |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
    {
        Error_Handler();
    }

    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

    /* SysTick_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(SysTick_IRQn, 15, 0);
}
static void MX_NVIC_Init(void)
{
  /* USART2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(USART2_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(USART2_IRQn);
  /* DMA1_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
  /* DMA1_Stream6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);
}

void Error_Handler(void)
{
    while(1)
    {
    }
    /* USER CODE END Error_Handler */
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
      ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */

}

#endif

/**
  * @}
  */

/**
  * @}
*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
