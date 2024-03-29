
#include "led.h"

void led_configuration(void)
{
    GPIO_InitTypeDef GPIO_Initure;
    __HAL_RCC_GPIOF_CLK_ENABLE();  //开启GPIOB时钟
    __HAL_RCC_GPIOE_CLK_ENABLE();  //开启GPIOB时钟
    __HAL_RCC_GPIOG_CLK_ENABLE();  //开启GPIOB时钟

    GPIO_Initure.Pin   = GPIO_PIN_14;
    GPIO_Initure.Mode  = GPIO_MODE_OUTPUT_PP;  //推挽输出
    GPIO_Initure.Pull  = GPIO_PULLUP;          //上拉
    GPIO_Initure.Speed = GPIO_SPEED_HIGH;      //高速
    HAL_GPIO_Init(GPIOF, &GPIO_Initure);

    GPIO_Initure.Pin = GPIO_PIN_11;
    HAL_GPIO_Init(GPIOE, &GPIO_Initure);

    led_green_off();
    led_red_off();

    GPIO_Initure.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8;
    HAL_GPIO_Init(GPIOG, &GPIO_Initure);

    flow_led_off(GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8);
}

void IndLED_On(int color)
{
    if (color == IndColorRed)
        HAL_GPIO_WritePin(GPIOI, GPIO_PIN_2, GPIO_PIN_SET);

    else if (color == IndColorBlue)
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

    else if (color == IndColorGreen)
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
}
void IndLED_Off(void)
{
    HAL_GPIO_WritePin(GPIOI, GPIO_PIN_2, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
}

void led_green_off(void)
{
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_14, GPIO_PIN_SET);
}
void led_green_on(void)
{
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_14, GPIO_PIN_RESET);
}
void led_green_toggle(void)
{
    HAL_GPIO_TogglePin(GPIOF, GPIO_PIN_14);
}

void led_red_off(void)
{
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_SET);
}
void led_red_on(void)
{
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_RESET);
}
extern void led_red_toggle(void)
{
    HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_11);
}

void flow_led_on(uint16_t num)
{
    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_8 >> num, GPIO_PIN_RESET);
}
void flow_led_off(uint16_t num)
{
    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_8 >> num, GPIO_PIN_SET);
}
void flow_led_toggle(uint16_t num)
{
    HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_8 >> num);
}
