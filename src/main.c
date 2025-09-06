#include "main.h"
#include "startup.h"
#include "stdinc.h"
#include "stm32f103xb.h"
#include "system_stm32f1xx.h"
#include <stddef.h>
#include <stm32f1xx.h>

typedef enum ClockMode : u8 {
    ClockMode_Astable,
    ClockMode_Monostable,
} ClockMode;

static constexpr u32 LED_PIN = 0;
static constexpr u32 BTN_PIN = 1;
static constexpr u32 ASTABLE_PIN = 8;
static constexpr u32 BISTABLE_PIN = 9;

static constexpr u32 FREQUENCY = 2;
static constexpr u32 HALF_PERIOD_MS = 1000 / (2 * FREQUENCY);

static constexpr u32 MONO_DELAY = 250;

static volatile u32 timer = 0;
static volatile ClockMode clock_mode = ClockMode_Astable;

static inline void set_led(const bool value)
{
    if (value)
        GPIOA->ODR |= (1 << LED_PIN);
    else
        GPIOA->ODR &= ~(1 << LED_PIN);
}

void systick_handler(void)
{
    if (timer > 0)
        --timer;

    if (timer == 0) {
        if (clock_mode == ClockMode_Astable) {
            // Oscillate
            timer = HALF_PERIOD_MS;
            GPIOA->ODR ^= (1 << LED_PIN);
        } else {
            set_led(false);
        }
    }
}

int main(void)
{
    // Turn on HSE
    RCC->CR |= RCC_CR_HSEON;
    while ((RCC->CR & RCC_CR_HSERDY) == 0)
        ;

    // Select HSE as system clock
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_HSE;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSE)
        ;

    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / 1000);

    // Enable port A GPIO clock
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

    // Set A0 as output
    GPIOA->CRL &= ~(0b1111 << (4 * LED_PIN));
    GPIOA->CRL |= (0b0010 << (4 * LED_PIN));

    // Set A1 as input
    GPIOA->CRL &= ~(0b1111 << (4 * BTN_PIN));
    GPIOA->CRL |= (0b1000 << (4 * BTN_PIN));

    // Set A8 as input
    GPIOA->CRH &= ~(0b1111 << (4 * (ASTABLE_PIN - 8)));
    GPIOA->CRH |= (0b1000 << (4 * (ASTABLE_PIN - 8)));

    // Set A9 as input
    GPIOA->CRH &= ~(0b1111 << (4 * (BISTABLE_PIN - 8)));
    GPIOA->CRH |= (0b1000 << (4 * (BISTABLE_PIN - 8)));

    while (true) {
        const bool astable_sel = (GPIOA->IDR & (1 << ASTABLE_PIN)) != 0;
        const bool bistable_sel = (GPIOA->IDR & (1 << BISTABLE_PIN)) != 0;
        const bool btn_pressed = (GPIOA->IDR & (1 << BTN_PIN)) != 0;

        if (astable_sel && clock_mode != ClockMode_Astable) {
            // Switch to astable mode
            __disable_irq();

            clock_mode = ClockMode_Astable;
            set_led(false);
            timer = HALF_PERIOD_MS;

            __enable_irq();
        } else if (bistable_sel && clock_mode != ClockMode_Monostable) {
            // Switch to monostable mode
            __disable_irq();

            clock_mode = ClockMode_Monostable;
            set_led(false);
            timer = 0;

            __enable_irq();
        }

        if (btn_pressed && clock_mode == ClockMode_Monostable) {
            // Turn on LED
            timer = MONO_DELAY;
            GPIOA->ODR |= (1 << LED_PIN);
        }
    }

    return 0;
}
