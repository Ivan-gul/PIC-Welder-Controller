#ifndef UART_H
#define UART_H

#include <xc.h>
#include "config.h"
#include <stdio.h>

/**
 * Управление UART для диагностики и отладки
 * Скорость: 9600 бод
 * TX: RC6
 * RX: RC7
 */

#define UART_BUFSIZE 32
static unsigned char uart_rx_buffer[UART_BUFSIZE];
static unsigned char uart_rx_head = 0;
static unsigned char uart_rx_tail = 0;

/**
 * Инициализация UART
 */
void init_uart(void) {
    // Установить RC6 (TX) и RC7 (RX) как вводы-выводы
    TRISCbits.TRISC6 = 0;  // RC6 - выход (TX)
    TRISCbits.TRISC7 = 1;  // RC7 - вход (RX)
    
    // Конфигурация UART
    // Скорость: 9600 бод
    // BRGH = 1 (высокая скорость)
    // SPBRG = (Fosc / (4 * Baud * 16)) - 1
    // SPBRG = (48000000 / (4 * 9600 * 16)) - 1 = 77
    
    SPBRG = 77;      // Скорость 9600 бод при Fosc=48MHz
    BAUDCONbits.BRG16 = 0;  // 8-bit mode
    
    // Конфигурация RCSTA
    RCSTA = 0b10010000;  // SPEN=1, CREN=1 (RX включен)
    // SPEN - Serial Port Enable
    // CREN - Continuous Receive Enable
    
    // Конфигурация TXSTA
    TXSTA = 0b00100100;  // TXEN=1, BRGH=1, SYNC=0 (асинхронный режим)
    // TXEN - TX Enable
    // BRGH - Baud Rate Select
    
    // Очистить буфер
    uart_rx_head = 0;
    uart_rx_tail = 0;
}

/**
 * Отправить один символ через UART
 * @param ch - символ для отправки
 */
void uart_send_char(unsigned char ch) {
    while(!TRMT);  // Ждать, пока буфер передатчика освободится
    TXREG = ch;    // Отправить символ
}

/**
 * Отправить строку через UART
 * @param str - указатель на строку
 */
void uart_send_string(const char* str) {
    while(*str) {
        uart_send_char(*str++);
    }
}

/**
 * Отправить новую строку (\r\n)
 */
void uart_send_newline(void) {
    uart_send_char('\r');
    uart_send_char('\n');
}

/**
 * Отправить число в виде десятичной строки
 * @param num - число для отправки
 */
void uart_send_number(uint16_t num) {
    char buffer[6];  // Макс 5 цифр + нулевой терминатор
    sprintf(buffer, "%u", num);
    uart_send_string(buffer);
}

/**
 * Отправить сообщение при запуске
 */
void send_startup_message(void) {
    uart_send_newline();
    uart_send_string("=====================================\r\n");
    uart_send_string("PIC Welder Controller v1.0\r\n");
    uart_send_string("Inverter Welding Machine\r\n");
    uart_send_string("=====================================\r\n");
    uart_send_string("Ready for commands...\r\n");
    uart_send_newline();
}

/**
 * Прочитать один символ из UART (если доступен)
 * @return символ или 0 если буфер пуст
 */
unsigned char uart_receive_char(void) {
    if(uart_rx_head == uart_rx_tail) {
        return 0;  // Буфер пуст
    }
    
    unsigned char ch = uart_rx_buffer[uart_rx_tail];
    uart_rx_tail = (uart_rx_tail + 1) % UART_BUFSIZE;
    
    return ch;
}

/**
 * Проверить, доступны ли данные в UART
 * @return 1 если данные доступны, 0 если буфер пуст
 */
uint8_t uart_data_available(void) {
    // Проверить флаг RCIF (Receive Interrupt Flag)
    if(RCIF) {
        unsigned char ch = RCREG;
        uart_rx_buffer[uart_rx_head] = ch;
        uart_rx_head = (uart_rx_head + 1) % UART_BUFSIZE;
        RCIF = 0;  // Очистить флаг
        return 1;
    }
    
    return (uart_rx_head != uart_rx_tail);
}

/**
 * Получить количество доступных байтов в буфере
 * @return количество байтов
 */
uint8_t uart_available(void) {
    return (uart_rx_head - uart_rx_tail + UART_BUFSIZE) % UART_BUFSIZE;
}

#endif // UART_H
