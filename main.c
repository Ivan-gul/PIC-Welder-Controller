#include <xc.h>
#include "config.h"
#include "pwm.h"
#include "adc.h"
#include "soft_start.h"
#include "protection.h"
#include "uart.h"

/**
 * Главный файл программы управления инверторным сварочным аппаратом
 * 
 * Функции:
 * - Управление ШИМ для полумоста инвертора
 * - Контроль сварочного тока через АЦП
 * - Контроль сетевого напряжения
 * - Мониторинг температуры силовых диодов
 * - Управление схемой мягкого старта
 * - Защита от перегрузок
 * - Диагностика через UART
 */

// Переменные состояния приложения
typedef struct {
    uint16_t current;           // Текущий сварочный ток (А)
    uint16_t grid_voltage;      // Сетевое напряжение (В)
    uint8_t diode_temperature;  // Температура диодов (°C)
    uint8_t pwm_duty;           // Текущее значение ШИМ (0-255)
    uint8_t system_state;       // Состояние системы
    uint16_t uptime_ms;         // Время работы (мс)
    uint16_t cycle_counter;     // Счетчик циклов
} SystemState;

// Глобальное состояние
SystemState system_state;

// Целевой ток для ШИМ управления
uint16_t target_current = 50;  // Начальный целевой ток 50 А

// Пропорциональный регулятор
typedef struct {
    int16_t error;              // Ошибка регулятора
    int16_t last_error;         // Предыдущая ошибка
    int16_t integral;           // Интегральная часть
    uint8_t kp;                 // Коэффициент пропорциональности
    uint8_t ki;                 // Коэффициент интеграции
    uint8_t kd;                 // Коэффициент дифференцирования
} PIDController;

// ПИД регулятор для управления сварочным током
PIDController pid_controller = {
    .error = 0,
    .last_error = 0,
    .integral = 0,
    .kp = 2,    // P коэффициент
    .ki = 0,    // I коэффициент
    .kd = 1     // D коэффициент
};

/**
 * Инициализация всех компонентов системы
 */
void system_init(void) {
    // Инициализация основных модулей
    init_adc();
    init_pwm();
    init_soft_start();
    init_protection();
    init_uart();
    
    // Инициализация состояния
    system_state.current = 0;
    system_state.grid_voltage = 0;
    system_state.diode_temperature = 0;
    system_state.pwm_duty = 0;
    system_state.system_state = 0;  // IDLE
    system_state.uptime_ms = 0;
    system_state.cycle_counter = 0;
    
    // Отправить приветственное сообщение
    send_startup_message();
    uart_send_string("System initialized successfully!");
    uart_send_newline();
}

/**
 * Простой ПИД регулятор для управления сварочным током
 * @param target - целевой ток (А)
 * @param actual - текущий ток (А)
 * @return новое значение ШИМ (0-255)
 */
uint8_t pid_control(uint16_t target, uint16_t actual) {
    static int16_t pwm_output = 128;
    
    // Вычислить ошибку
    pid_controller.error = target - actual;
    
    // P компонент
    int16_t p_term = (pid_controller.error * pid_controller.kp) / 10;
    
    // I компонент
    pid_controller.integral += pid_controller.error;
    if(pid_controller.integral > 200) pid_controller.integral = 200;
    if(pid_controller.integral < -200) pid_controller.integral = -200;
    int16_t i_term = (pid_controller.integral * pid_controller.ki) / 100;
    
    // D компонент
    int16_t d_term = ((pid_controller.error - pid_controller.last_error) * pid_controller.kd) / 10;
    pid_controller.last_error = pid_controller.error;
    
    // Общий выход регулятора
    pwm_output = 128 + p_term + i_term + d_term;
    
    // Ограничение значения
    if(pwm_output > 255) pwm_output = 255;
    if(pwm_output < 0) pwm_output = 0;
    
    return (uint8_t)pwm_output;
}

/**
 * Обновить состояние системы на основе показаний датчиков
 */
void update_system_state(void) {
    // Прочитать все датчики
    system_state.current = read_current();
    system_state.grid_voltage = read_grid_voltage();
    system_state.diode_temperature = read_diode_temperature();
    
    // Проверить защиту
    ErrorCode error = check_all_faults(system_state.current, 
                                       system_state.grid_voltage, 
                                       system_state.diode_temperature);
    
    // Обработка ошибок
    if(error != ERROR_NONE) {
        // Отключить ШИМ при ошибке
        disable_pwm();
        system_state.pwm_duty = 0;
        
        // Отправить сообщение об ошибке
        uart_send_string("[ERROR] ");
        uart_send_string(get_error_string(error));
        uart_send_string(" - I=");
        uart_send_number(system_state.current);
        uart_send_string("A U=");
        uart_send_number(system_state.grid_voltage);
        uart_send_string("V T=");
        uart_send_number(system_state.diode_temperature);
        uart_send_string("C");
        uart_send_newline();
        
        system_state.system_state = 2;  // ERROR state
    } else {
        // Все норма, используем ПИД регулятор
        if(system_state.system_state == 1) {  // RUNNING state
            system_state.pwm_duty = pid_control(target_current, system_state.current);
            set_pwm_duty(system_state.pwm_duty);
        }
    }
}

/**
 * Отправить диагностическую информацию
 */
void send_diagnostics(void) {
    // Формат вывода:
    // I=45A U=220V T=65C PWM=150 STATE=1
    
    uart_send_string("DATA: I=");
    uart_send_number(system_state.current);
    uart_send_string("A U=");
    uart_send_number(system_state.grid_voltage);
    uart_send_string("V T=");
    uart_send_number(system_state.diode_temperature);
    uart_send_string("C PWM=");
    uart_send_number(system_state.pwm_duty);
    uart_send_string(" STATE=");
    uart_send_number(system_state.system_state);
    uart_send_string(" CYCLES=");
    uart_send_number(system_state.cycle_counter);
    uart_send_newline();
}

/**
 * Включить сварку (запустить мягкий старт и включить ШИМ)
 */
void start_welding(void) {
    if(system_state.system_state == 0) {  // IDLE state
        // Запустить мягкий старт
        start_soft_start();
        system_state.system_state = 1;  // RUNNING state
        
        uart_send_string("Soft start initiated...");
        uart_send_newline();
    }
}

/**
 * Остановить сварку
 */
void stop_welding(void) {
    // Отключить ШИМ
    disable_pwm();
    system_state.pwm_duty = 0;
    
    // Остановить мягкий старт
    stop_soft_start();
    
    // Вернуться в режим IDLE
    system_state.system_state = 0;
    
    // Сбросить ПИД регулятор
    pid_controller.integral = 0;
    pid_controller.last_error = 0;
    
    uart_send_string("Welding stopped.");
    uart_send_newline();
}

/**
 * Главный цикл программы
 */
void main(void) {
    // Инициализация
    system_init();
    
    // Переменные для отслеживания времени
    uint16_t diag_timer = 0;
    uint16_t ss_timer = 0;
    
    // Основной цикл
    while(1) {
        // Обновить состояние системы (каждые 10 мс)
        update_system_state();
        
        // Обновить мягкий старт (если включен)
        if(system_state.system_state == 1) {
            ss_timer += 10;
            update_soft_start(ss_timer);
            
            // Проверить, завершен ли мягкий старт
            if(is_soft_start_completed()) {
                // Вывести сообщение
                uart_send_string("Soft start completed, welding active.");
                uart_send_newline();
                ss_timer = 0;
            }
        }
        
        // Отправить диагностику (каждые 500 мс)
        diag_timer += 10;
        if(diag_timer >= 500) {
            send_diagnostics();
            diag_timer = 0;
        }
        
        // Увеличить счетчики
        system_state.uptime_ms += 10;
        system_state.cycle_counter++;
        
        // Обработка команд из UART (опционально)
        if(uart_data_available()) {
            unsigned char cmd = uart_receive_char();
            
            switch(cmd) {
                case 'S':
                case 's':
                    // Start welding
                    start_welding();
                    break;
                    
                case 'P':
                case 'p':
                    // Stop welding
                    stop_welding();
                    break;
                    
                case '+':
                    // Увеличить целевой ток на 5 А
                    if(target_current < 100) {
                        target_current += 5;
                        uart_send_string("Target current: ");
                        uart_send_number(target_current);
                        uart_send_string("A");
                        uart_send_newline();
                    }
                    break;
                    
                case '-':
                    // Уменьшить целевой ток на 5 А
                    if(target_current > 5) {
                        target_current -= 5;
                        uart_send_string("Target current: ");
                        uart_send_number(target_current);
                        uart_send_string("A");
                        uart_send_newline();
                    }
                    break;
                    
                case '?':
                    // Отправить помощь
                    uart_send_string("Commands:");
                    uart_send_newline();
                    uart_send_string("S - Start welding");
                    uart_send_newline();
                    uart_send_string("P - Stop welding");
                    uart_send_newline();
                    uart_send_string("+ - Increase current");
                    uart_send_newline();
                    uart_send_string("- - Decrease current");
                    uart_send_newline();
                    break;
            }
        }
        
        // Задержка 10 мс перед следующей итерацией
        __delay_ms(10);
    }
}
