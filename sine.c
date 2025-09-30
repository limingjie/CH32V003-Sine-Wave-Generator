/*
 * Smart LED Flashlight - CH32V003
 *
 * Aug 2025 by Li Mingjie
 *  - Email:  limingjie@outlook.com
 *  - GitHub: https://github.com/limingjie/
 */

#include <stdio.h>
#include "ch32fun.h"

#define PIN_PWM PC4  // PWM output pin

#define PWM_FREQUENCY              ((uint32_t)200 * 1000)                       // 200kHz
#define PWM_CLOCKS_FULL_DUTY_CYCLE (FUNCONF_SYSTEM_CORE_CLOCK / PWM_FREQUENCY)  // 100% duty cycle
#define PWM_CLOCKS_ZERO_DUTY_CYCLE 0

// sine_table(n) = [sin(2PI x n / 100) + 1] x (PWM_CLOCKS_FULL_DUTY_CYCLE - 1) / 2
// n = 0..99
// Example: PWM_CLOCKS_FULL_DUTY_CYCLE = 240
#define No_OF_SAMPLES 100
static const uint16_t sine_table[No_OF_SAMPLES] = {
    120, 127, 134, 142, 149, 156, 163, 170, 177, 184, 190, 196, 201, 207, 212, 216, 220, 224, 228, 231,
    233, 235, 237, 238, 239, 239, 239, 238, 237, 235, 233, 231, 228, 224, 220, 216, 212, 207, 201, 196,
    190, 184, 177, 170, 163, 156, 149, 142, 134, 127, 120, 112, 105, 97,  90,  83,  76,  69,  62,  55,
    49,  43,  38,  32,  27,  23,  19,  15,  11,  8,   6,   4,   2,   1,   0,   0,   0,   1,   2,   4,
    6,   8,   11,  15,  19,  23,  27,  32,  38,  43,  49,  55,  62,  69,  76,  83,  90,  97,  105, 112,
};

volatile uint16_t output_frequency = 100;  // in Hz

void systick_init(void)
{
    SysTick->CTLR = 0;
    NVIC_EnableIRQ(SysTicK_IRQn);
    SysTick->CMP  = FUNCONF_SYSTEM_CORE_CLOCK / output_frequency / No_OF_SAMPLES - 1;  // 100µs
    SysTick->CNT  = 0;
    SysTick->CTLR = SYSTICK_CTLR_STE | SYSTICK_CTLR_STIE | SYSTICK_CTLR_STCLK;
}

void disable_systick(void)
{
    NVIC_DisableIRQ(SysTicK_IRQn);
}

void SysTick_Handler(void) __attribute__((interrupt));
void SysTick_Handler(void)
{
    static uint16_t counter = 0;

    SysTick->CMP += FUNCONF_SYSTEM_CORE_CLOCK / output_frequency / No_OF_SAMPLES;  // Interval to change PWM value
    SysTick->SR = 0;

    TIM1->CH4CVR = sine_table[counter++];
    counter %= No_OF_SAMPLES;
}

void tim1_pwm_init(void)
{
    // Enable GPIOC and TIM1
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOC | RCC_APB2Periph_TIM1;

    // PC4 is T1CH4, 10MHz Output alt func, push-pull
    GPIOC->CFGLR &= ~(0xf << (4 * 4));
    GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF) << (4 * 4);

    // Reset TIM1 to init all regs
    RCC->APB2PRSTR |= RCC_APB2Periph_TIM1;
    RCC->APB2PRSTR &= ~RCC_APB2Periph_TIM1;

    // Prescaler
    TIM1->PSC = 0x0000;

    // Auto Reload - sets period
    TIM1->ATRLR = PWM_CLOCKS_FULL_DUTY_CYCLE - 1;

    // Reload immediately
    TIM1->SWEVGR |= TIM_UG;

    // Enable CH4 output, positive pol
    TIM1->CCER |= TIM_CC4E | TIM_CC4NP;

    // CH2 Mode is output, PWM1 (CC1S = 00, OC1M = 110)
    TIM1->CHCTLR2 |= TIM_OC4M_2 | TIM_OC4M_1;

    // Set the Capture Compare Register value to 0% initially
    TIM1->CH4CVR = PWM_CLOCKS_ZERO_DUTY_CYCLE;

    // Enable TIM1 outputs
    TIM1->BDTR |= TIM_MOE;

    // Enable TIM1
    TIM1->CTLR1 |= TIM_CEN;
}

int main(void)
{
    SystemInit();
    Delay_Ms(50);

    funGpioInitAll();

    // Init TIM1 for PWM
    tim1_pwm_init();

    systick_init();

    while (1)
    {
        // output_frequency += 10;
        // printf("Output frequency: %d Hz\r\n", output_frequency);
        // Delay_Ms(1000);
    }
}
