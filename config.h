#ifndef CONFIG_H
#define CONFIG_H

/**
 * Конфигурация микроконтроллера PIC18F4550
 * Частота: 48 МГц (20 МГц кварц + PLL)
 */

// Pragma конфигурационные биты
#pragma config PLLDIV = 5          // 20 MHz -> 4 MHz (Fosc/5)
#pragma config CPUDIV = OSC1_PLL2  // 96 MHz PLL / 2 = 48 MHz
#pragma config USBDIV = 2           // 48 MHz для USB
#pragma config FOSC = HSPLL_HS      // HS oscилляtor + PLL
#pragma config FCMEN = OFF          // Fail-Safe Clock Monitor OFF
#pragma config IESO = OFF           // Internal/External Osc Switch OFF
#pragma config PWRT = OFF           // Power-Up Timer OFF
#pragma config BOR = ON             // Brown-Out Reset ON
#pragma config BORV = 3             // Brown-Out Voltage: 2.05V
#pragma config VREGEN = ON          // USB Voltage Regulator ON
#pragma config WDT = OFF            // Watchdog Timer OFF
#pragma config WDTPS = 32768        // Watchdog Prescale
#pragma config MCLRE = ON           // Master Clear Pin ON
#pragma config LPT1OSC = OFF        // Low-Power Timer1 OFF
#pragma config PBADEN = OFF         // Analog/Digital
#pragma config CCP2MX = OFF         // CCP2 on RC1
#pragma config STVREN = ON          // Stack Overflow Reset ON
#pragma config LVP = OFF            // Low-Voltage Programming OFF
#pragma config ICPRT = OFF          // ICP Pin OFF
#pragma config DEBUG = OFF          // Background Debugging OFF
#pragma config CP0 = OFF            // Code Protection Block 0 OFF
#pragma config CP1 = OFF            // Code Protection Block 1 OFF
#pragma config CP2 = OFF            // Code Protection Block 2 OFF
#pragma config CP3 = OFF            // Code Protection Block 3 OFF
#pragma config CPB = OFF            // Code Protection Boot OFF
#pragma config CPD = OFF            // Data EEPROM Code Protection OFF
#pragma config WRT0 = OFF           // Write Protection Block 0 OFF
#pragma config WRT1 = OFF           // Write Protection Block 1 OFF
#pragma config WRT2 = OFF           // Write Protection Block 2 OFF
#pragma config WRT3 = OFF           // Write Protection Block 3 OFF
#pragma config WRTB = OFF           // Write Protection Boot OFF
#pragma config WRTC = OFF           // Write Protection Config OFF
#pragma config WRTD = OFF           // Write Protection Data OFF
#pragma config EBTR0 = OFF          // EEPROM Table Read Block 0 OFF
#pragma config EBTR1 = OFF          // EEPROM Table Read Block 1 OFF
#pragma config EBTR2 = OFF          // EEPROM Table Read Block 2 OFF
#pragma config EBTR3 = OFF          // EEPROM Table Read Block 3 OFF
#pragma config EBTRB = OFF          // EEPROM Table Read Boot OFF

// Определение системной частоты
#define _XTAL_FREQ 48000000  // 48 МГц

// Выводы контроллера
#define PWM_Q1_PIN PORTCbits.RC2    // CCP1 - верхний ключ Q1 (PWM)
#define PWM_Q2_PIN PORTCbits.RC1    // CCP2 - нижний ключ Q2 (PWM инверсный)
#define SOFT_START_PIN PORTBbits.RB0  // Мягкий старт
#define LED_ERROR_PIN PORTBbits.RB1   // Светодиод ошибки

// Параметры защиты
#define MAX_WELDING_CURRENT 100     // Макс. ток сварки (А)
#define MIN_WELDING_CURRENT 5       // Мин. ток сварки (А)
#define MAX_GRID_VOLTAGE 280        // Макс. сетевое напряжение (В)
#define MIN_GRID_VOLTAGE 160        // Мин. сетевое напряжение (В)
#define MAX_DIODE_TEMP 85           // Макс. температура диодов (°C)
#define OVERCURRENT_THRESHOLD 110   // Порог перегрузки по току (A)

// Параметры мягкого старта
#define SOFT_START_TIME 1000        // Время разгона (мс)
#define SOFT_START_STEPS 100        // Количество шагов разгона

// Параметры ШИМ
#define PWM_FREQUENCY 16000         // Частота ШИМ (Гц)
#define PWM_PERIOD 3                // Период для PR2 (48MHz/16kHz/4 ≈ 3)
#define PWM_MAX_DUTY 255            // Максимальное значение ШИМ

// UART параметры
#define UART_BAUDRATE 9600          // Скорость UART
#define UART_TX_PIN TRISCbits.TRISC6  // TX на RC6
#define UART_RX_PIN TRISCbits.TRISC7  // RX на RC7

#endif // CONFIG_H
