#ifndef SOFT_START_H
#define SOFT_START_H

#include <xc.h>
#include "config.h"
#include "pwm.h"

/**
 * Управление схемой мягкого старта
 * Постепенное включение ШИМ за 1 секунду
 * RB0 - выход управления мягким стартом
 */

#define SOFT_START_OUTPUT_PIN TRISBbits.TRISB0
#define SOFT_START_PORT_PIN PORTBbits.RB0

static uint16_t soft_start_timer = 0;
static uint8_t soft_start_active = 0;
static uint8_t soft_start_completed = 0;

/**
 * Инициализация мягкого старта
 */
void init_soft_start(void) {
    // Установить RB0 как выход
    SOFT_START_OUTPUT_PIN = 0;
    SOFT_START_PORT_PIN = 0;
    
    // Инициализировать переменные
    soft_start_timer = 0;
    soft_start_active = 0;
    soft_start_completed = 0;
}

/**
 * Запустить процесс мягкого старта
 */
void start_soft_start(void) {
    if(!soft_start_active) {
        soft_start_active = 1;
        soft_start_completed = 0;
        soft_start_timer = 0;
        SOFT_START_PORT_PIN = 1;  // Включить мягкий старт
    }
}

/**
 * Остановить процесс мягкого старта
 */
void stop_soft_start(void) {
    soft_start_active = 0;
    soft_start_completed = 0;
    soft_start_timer = 0;
    SOFT_START_PORT_PIN = 0;  // Выключить мягкий старт
}

/**
 * Обновить состояние мягкого старта
 * Эта функция должна вызываться каждые 10 мс
 * @param elapsed_time - прошедшее время в мс
 */
void update_soft_start(uint16_t elapsed_time) {
    if(!soft_start_active) return;
    
    soft_start_timer = elapsed_time;
    
    // Рассчитать текущее значение ШИМ
    // Плавный разгон от 0% до 100% за SOFT_START_TIME мс
    if(soft_start_timer < SOFT_START_TIME) {
        // Рассчитать коэффициент (0 - 255)
        uint8_t pwm_value = (soft_start_timer * 255) / SOFT_START_TIME;
        set_pwm_duty(pwm_value);
    } else {
        // Разгон завершен
        set_pwm_duty(128);  // 50% по умолчанию после мягкого старта
        soft_start_completed = 1;
        soft_start_active = 0;
    }
}

/**
 * Проверить, завершен ли мягкий старт
 * @return 1 если завершен, 0 если нет
 */
uint8_t is_soft_start_completed(void) {
    return soft_start_completed;
}

/**
 * Получить прогресс мягкого старта (0-100%)
 * @return процент завершения
 */
uint8_t get_soft_start_progress(void) {
    if(!soft_start_active && !soft_start_completed) return 0;
    if(soft_start_completed) return 100;
    
    return (soft_start_timer * 100) / SOFT_START_TIME;
}

#endif // SOFT_START_H
