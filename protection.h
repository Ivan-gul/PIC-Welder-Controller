#ifndef PROTECTION_H
#define PROTECTION_H

#include <xc.h>
#include "config.h"
#include "pwm.h"

/**
 * Система защиты и контроля ошибок
 * - Контроль сетевого напряжения (завышенное/заниженное)
 * - Контроль тока сварки (перегрузка)
 * - Контроль температуры диодов (перегрев)
 */

// Коды ошибок
typedef enum {
    ERROR_NONE = 0,
    ERROR_OVERVOLTAGE = 1,      // Завышенное напряжение
    ERROR_UNDERVOLTAGE = 2,     // Заниженное напряжение
    ERROR_OVERCURRENT = 3,      // Перегрузка по току
    ERROR_OVERTEMP = 4,         // Перегрев диодов
    ERROR_MULTIPLE = 5          // Множественные ошибки
} ErrorCode;

static uint16_t error_flags = 0;  // Флаги ошибок
static uint8_t error_led_pin = TRISBbits.TRISB1;  // RB1 - светодиод ошибки

/**
 * Инициализация системы защиты
 */
void init_protection(void) {
    // Установить RB1 как выход для LED ошибки
    error_led_pin = 0;
    PORTBbits.RB1 = 0;  // LED выключен
    
    error_flags = 0;
}

/**
 * Проверить завышенное сетевое напряжение
 * @param voltage - сетевое напряжение (В)
 * @return 1 если ошибка, 0 если норма
 */
static uint8_t check_overvoltage(uint16_t voltage) {
    if(voltage > MAX_GRID_VOLTAGE) {
        error_flags |= (1 << ERROR_OVERVOLTAGE);
        return 1;
    }
    error_flags &= ~(1 << ERROR_OVERVOLTAGE);
    return 0;
}

/**
 * Проверить заниженное сетевое напряжение
 * @param voltage - сетевое напряжение (В)
 * @return 1 если ошибка, 0 если норма
 */
static uint8_t check_undervoltage(uint16_t voltage) {
    if(voltage < MIN_GRID_VOLTAGE) {
        error_flags |= (1 << ERROR_UNDERVOLTAGE);
        return 1;
    }
    error_flags &= ~(1 << ERROR_UNDERVOLTAGE);
    return 0;
}

/**
 * Проверить перегрузку по сварочному току
 * @param current - сварочный ток (А)
 * @return 1 если ошибка, 0 если норма
 */
static uint8_t check_overcurrent(uint16_t current) {
    if(current > OVERCURRENT_THRESHOLD) {
        error_flags |= (1 << ERROR_OVERCURRENT);
        return 1;
    }
    error_flags &= ~(1 << ERROR_OVERCURRENT);
    return 0;
}

/**
 * Проверить перегрев силовых диодов
 * @param temperature - температура диодов (°C)
 * @return 1 если ошибка, 0 если норма
 */
static uint8_t check_overtemp(uint8_t temperature) {
    if(temperature > MAX_DIODE_TEMP) {
        error_flags |= (1 << ERROR_OVERTEMP);
        return 1;
    }
    error_flags &= ~(1 << ERROR_OVERTEMP);
    return 0;
}

/**
 * Проверить все ошибки и вернуть код первой найденной ошибки
 * @param current - сварочный ток (А)
 * @param voltage - сетевое напряжение (В)
 * @param temperature - температура диодов (°C)
 * @return код ошибки (ErrorCode)
 */
ErrorCode check_all_faults(uint16_t current, uint16_t voltage, uint8_t temperature) {
    uint8_t error_count = 0;
    ErrorCode first_error = ERROR_NONE;
    
    // Проверить все ошибки
    if(check_overvoltage(voltage)) {
        error_count++;
        if(first_error == ERROR_NONE) first_error = ERROR_OVERVOLTAGE;
    }
    
    if(check_undervoltage(voltage)) {
        error_count++;
        if(first_error == ERROR_NONE) first_error = ERROR_UNDERVOLTAGE;
    }
    
    if(check_overcurrent(current)) {
        error_count++;
        if(first_error == ERROR_NONE) first_error = ERROR_OVERCURRENT;
    }
    
    if(check_overtemp(temperature)) {
        error_count++;
        if(first_error == ERROR_NONE) first_error = ERROR_OVERTEMP;
    }
    
    // Если несколько ошибок одновременно
    if(error_count > 1) {
        first_error = ERROR_MULTIPLE;
    }
    
    // Управление LED ошибки
    if(first_error != ERROR_NONE) {
        PORTBbits.RB1 = 1;  // LED включен
    } else {
        PORTBbits.RB1 = 0;  // LED выключен
    }
    
    return first_error;
}

/**
 * Получить строку описания ошибки
 * @param error - код ошибки
 * @return указатель на строку описания
 */
const char* get_error_string(ErrorCode error) {
    switch(error) {
        case ERROR_OVERVOLTAGE:
            return "Overvoltage";
        case ERROR_UNDERVOLTAGE:
            return "Undervoltage";
        case ERROR_OVERCURRENT:
            return "Overcurrent";
        case ERROR_OVERTEMP:
            return "Overtemp (Diodes)";
        case ERROR_MULTIPLE:
            return "Multiple Faults";
        case ERROR_NONE:
        default:
            return "No Error";
    }
}

/**
 * Получить текущие флаги ошибок
 * @return значение флагов ошибок
 */
uint16_t get_error_flags(void) {
    return error_flags;
}

/**
 * Очистить флаги ошибок
 */
void clear_error_flags(void) {
    error_flags = 0;
    PORTBbits.RB1 = 0;  // LED выключен
}

#endif // PROTECTION_H
