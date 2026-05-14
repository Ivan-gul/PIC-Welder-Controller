#ifndef PWM_H
#define PWM_H

#include <xc.h>
#include "config.h"

/**
 * Управление ШИМ для полумоста инвертора
 * CCP1 (RC2) - верхний ключ Q1
 * CCP2 (RC1) - нижний ключ Q2 (инверсный сигнал)
 * Частота: 16 кГц
 */

/**
 * Инициализация ШИМ модулей
 */
void init_pwm(void) {
    // Установить RC1 и RC2 как выходы
    TRISCbits.TRISC1 = 0;  // RC1 - выход (CCP2)
    TRISCbits.TRISC2 = 0;  // RC2 - выход (CCP1)
    
    // Установить начальные значения
    PORTCbits.RC1 = 0;
    PORTCbits.RC2 = 0;
    
    // Настройка Timer2 для ШИМ
    // Fosc = 48 MHz
    // PWM Frequency = Fosc / (4 * (PR2+1) * Prescaler)
    // 16kHz = 48MHz / (4 * (PR2+1) * 1)
    // PR2 = 48000000 / (4 * 16000) - 1 = 749
    // Но для более практичного значения используем: PR2 = 155 с Prescaler=4
    // PWM Frequency = 48MHz / (4 * (155+1) * 4) ≈ 19.2 kHz
    
    PR2 = 155;  // Период Timer2
    
    // Настройка Timer2
    T2CON = 0b00000101;  // Prescaler 1:4, Timer2 ON
    
    // Настройка CCP1 (Q1) - PWM режим
    CCP1CON = 0b00001100;  // PWM mode (bits 3:0 = 1100)
    CCPR1L = 0;            // Начальное значение ШИМ = 0%
    CCPxCON_DC1B0 = 0;     // LSB для 10-bit ШИМ
    CCPxCON_DC1B1 = 0;
    
    // Настройка CCP2 (Q2) - PWM режим
    CCP2CON = 0b00001100;  // PWM mode (bits 3:0 = 1100)
    CCPR2L = 0;            // Начальное значение ШИМ = 0%
    CCPxCON_DC2B0 = 0;     // LSB для 10-bit ШИМ
    CCPxCON_DC2B1 = 0;
}

/**
 * Установить коэффициент заполнения ШИМ (duty cycle)
 * @param duty - значение коэффициента (0-255)
 * 0 = 0%, 255 = 100%
 */
void set_pwm_duty(uint8_t duty) {
    if(duty > PWM_MAX_DUTY) duty = PWM_MAX_DUTY;
    
    // Для полумоста: когда Q1 включен, Q2 выключен
    // Q1 включается на duty%, Q2 включается на (100-duty)%
    CCPR1L = duty;           // Q1
    CCPR2L = PWM_MAX_DUTY - duty;  // Q2 инверсный
}

/**
 * Отключить ШИМ (установить оба выхода в 0)
 */
void disable_pwm(void) {
    CCPR1L = 0;      // Q1 = 0%
    CCPR2L = 255;    // Q2 = 100% (выключен в конфигурации полумоста)
}

/**
 * Включить ШИМ с полной мощностью (100%)
 */
void enable_pwm_full(void) {
    CCPR1L = 255;    // Q1 = 100%
    CCPR2L = 0;      // Q2 = 0%
}

/**
 * Получить текущее значение ШИМ
 * @return текущий duty cycle (0-255)
 */
uint8_t get_pwm_duty(void) {
    return CCPR1L;
}

#endif // PWM_H
