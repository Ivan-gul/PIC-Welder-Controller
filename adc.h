#ifndef ADC_H
#define ADC_H

#include <xc.h>
#include "config.h"

/**
 * Управление АЦП для чтения датчиков
 * AN0 (RA0) - датчик сварочного тока
 * AN1 (RA1) - датчик сетевого напряжения
 * AN2 (RA2) - датчик температуры диодов
 */

#define ADC_CURRENT_CHANNEL 0     // AN0
#define ADC_VOLTAGE_CHANNEL 1     // AN1
#define ADC_TEMP_CHANNEL 2        // AN2

/**
 * Инициализация АЦП
 */
void init_adc(void) {
    // Выбрать аналоговые каналы
    ADCON1 = 0b00001010;  // AN0, AN1, AN2 - аналоговые; остальные - цифровые
    
    // Установить RA0, RA1, RA2 как входы
    TRISAbits.TRISA0 = 1;  // RA0 - вход (AN0)
    TRISAbits.TRISA1 = 1;  // RA1 - вход (AN1)
    TRISAbits.TRISA2 = 1;  // RA2 - вход (AN2)
    
    // Конфигурация АЦП
    ADCON2 = 0b10101010;   // Right justified, Tad=10, Fosc/32
    // ADFM = 1 (правое выравнивание)
    // ACQT = 101 (10 TAD - время установки)
    // ADCS = 010 (Fosc/32 - предделитель)
    
    // Включить АЦП
    ADCON0 = 0b00000001;   // AN0 selected, ADON = 1 (АЦП включен)
}

/**
 * Прочитать значение с АЦП
 * @param channel - номер канала (0-7)
 * @return 10-бит��ое значение (0-1023)
 */
uint16_t read_adc(uint8_t channel) {
    // Выбрать канал
    ADCON0 = (ADCON0 & 0x3F) | ((channel & 0x07) << 2);
    
    // Время установки канала
    __delay_us(10);
    
    // Начать преобразование
    ADCON0bits.ADGO = 1;
    
    // Ждать завершения преобразования
    while(ADCON0bits.ADGO);
    
    // Прочитать результат (10-бит, правое выравнивание)
    uint16_t result = ((ADRESH << 8) | ADRESL);
    
    return result;
}

/**
 * Прочитать ток сварки из датчика
 * AN0: 0V = 0A, 5V = 100A
 * Формула: Current = (ADC_value / 1023) * 100
 * @return значение тока в амперах (0-100)
 */
uint16_t read_current(void) {
    uint16_t adc_val = read_adc(ADC_CURRENT_CHANNEL);
    // Преобразование: (ADC * 100) / 1024
    return (adc_val * 100) >> 10;  // Деление на 1024 через битовый сдвиг
}

/**
 * Прочитать сетевое напряжение из датчика
 * AN1: 0V = 0V, 5V = 300V (с делителем напряжения)
 * Формула: Voltage = (ADC_value / 1023) * 300
 * @return значение напряжения в вольтах (0-300)
 */
uint16_t read_grid_voltage(void) {
    uint16_t adc_val = read_adc(ADC_VOLTAGE_CHANNEL);
    // Преобразование: (ADC * 300) / 1024
    return (adc_val * 300) >> 10;  // Деление на 1024 через битовый сдвиг
}

/**
 * Прочитать температуру силовых диодов
 * AN2: датчик температуры (NTC терморезистор или LM35)
 * Формула зависит от типа датчика
 * @return значение температуры в градусах Цельсия (0-120)
 */
uint8_t read_diode_temperature(void) {
    uint16_t adc_val = read_adc(ADC_TEMP_CHANNEL);
    // Для LM35: 10mV/°C -> (ADC * 120) / 1024
    // Для NTC: требуется калибровка
    uint8_t temp = (adc_val * 120) >> 10;
    if(temp > 120) temp = 120;
    return temp;
}

/**
 * Прочитать сырое значение АЦП конкретного канала
 * @param channel - номер канала (0-2)
 * @return 10-битное значение (0-1023)
 */
uint16_t read_adc_raw(uint8_t channel) {
    return read_adc(channel);
}

#endif // ADC_H
