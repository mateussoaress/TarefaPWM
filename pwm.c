#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define SERVO_PIN 22      // GPIO do servomotor
#define LED_PIN 12        // GPIO do LED RGB (BitDogLab)
#define PWM_FREQ 50       // Frequência de 50Hz (período de 20ms)

// Parâmetros do PWM
#define CLOCK_FREQ 125000000  // Frequência do clock base do RP2040 (125MHz)
#define DIVIDER 64.0          // Divisor de clock para ajuste de frequência
#define WRAP_VALUE (CLOCK_FREQ / (DIVIDER * PWM_FREQ))  // Cálculo do wrap

// Posições do servo
#define PULSE_MIN 500   // 0 graus - 500µs
#define PULSE_90 1470   // 90 graus - 1.47ms
#define PULSE_MAX 2400  // 180 graus - 2.4ms
#define STEP_US 5       // Incremento/decremento de 5µs
#define DELAY_MS 10     // Atraso de 10ms por passo

void setup_pwm();
void set_servo_position(uint16_t pulse_width, uint16_t wait_time);
void move_servo_with_led(uint16_t start, uint16_t end);

// Protótipos das funções
int main()
{
    stdio_init_all();
    setup_pwm();

    // Ajusta posição inicial do servo e espera 5s
    printf("Movendo servo para 180 graus (2400µs)...\n");
    set_servo_position(PULSE_MAX, 5000); // 180 graus

    printf("Movendo servo para 90 graus (1470µs)...\n");
    set_servo_position(PULSE_90, 5000); // 90 graus

    printf("Movendo servo para 0 graus (500µs)...\n");
    set_servo_position(PULSE_MIN, 5000); // 0 graus

    // Agora inicia a movimentação suave entre 0° e 180°
    while (true)
    {
        printf("Movendo suavemente para 180 graus...\n");
        move_servo_with_led(PULSE_MIN, PULSE_MAX);
        

        printf("Movendo suavemente para 0 graus...\n");
        move_servo_with_led(PULSE_MAX, PULSE_MIN);
        
    }
}

void setup_pwm()
{
    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM); // Configura GPIO do servo para PWM
    gpio_set_function(LED_PIN, GPIO_FUNC_PWM);   // Configura GPIO do LED RGB para PWM

    uint slice_servo = pwm_gpio_to_slice_num(SERVO_PIN); // Obtém slice do PWM do servo
    uint slice_led = pwm_gpio_to_slice_num(LED_PIN);     // Obtém slice do PWM do LED

    pwm_set_clkdiv(slice_servo, DIVIDER);
    pwm_set_wrap(slice_servo, WRAP_VALUE);

    pwm_set_clkdiv(slice_led, DIVIDER);
    pwm_set_wrap(slice_led, WRAP_VALUE);

    pwm_set_gpio_level(SERVO_PIN, PULSE_MIN * WRAP_VALUE / 20000);
    pwm_set_gpio_level(LED_PIN, 0); // Começa com o LED apagado

    pwm_set_enabled(slice_servo, true);
    pwm_set_enabled(slice_led, true);
}

void set_servo_position(uint16_t pulse_width, uint16_t wait_time)
{
    pwm_set_gpio_level(SERVO_PIN, pulse_width * WRAP_VALUE / 20000); // Ajusta duty cycle do servo
    uint16_t led_brightness = (pulse_width - PULSE_MIN) * WRAP_VALUE / (PULSE_MAX - PULSE_MIN); 
    pwm_set_gpio_level(LED_PIN, led_brightness); // Ajusta brilho do LED com base na posição

    printf("Posição ajustada para %d µs. Aguardando %d ms...\n", pulse_width, wait_time);
    sleep_ms(wait_time);
}

void move_servo_with_led(uint16_t start, uint16_t end)
{
    int step = (start < end) ? STEP_US : -STEP_US;
    uint16_t pos = start;

    while (pos != end)
    {
        pwm_set_gpio_level(SERVO_PIN, pos * WRAP_VALUE / 20000);

        // Ajusta brilho do LED conforme o servo se movimenta
        uint16_t led_brightness = (pos - PULSE_MIN) * WRAP_VALUE / (PULSE_MAX - PULSE_MIN);
        pwm_set_gpio_level(LED_PIN, led_brightness);

        sleep_ms(DELAY_MS);
        pos += step;
    }
}
