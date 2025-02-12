# Controle de Servomotor com LED RGB usando Raspberry Pi Pico

Este programa foi desenvolvido para controlar um **servomotor** utilizando um **sinal PWM** gerado pelo Raspberry Pi Pico. Além disso, um **LED RGB** conectado à GPIO 12 é utilizado para indicar visualmente a posição do servomotor. O objetivo do código é garantir que o servo se mova entre **0° e 180° de forma suave**, enquanto o LED altera sua intensidade proporcionalmente à posição do servo.

# Funcionamento do Programa

O programa começa configurando o **PWM** na GPIO 22 para operar a **50Hz**, o que corresponde a um **período de 20ms**. Isso é necessário para controlar o servo, já que a maioria dos servomotores utilizam essa frequência para determinar o ângulo de posicionamento.

Após a configuração, o servo é posicionado sequencialmente nos seguintes ângulos:
1. **180 graus (2.400µs)** – Aguarda 5 segundos.
2. **90 graus (1.470µs)** – Aguarda 5 segundos.
3. **0 graus (500µs)** – Aguarda 5 segundos.

Depois dessa fase inicial, o programa entra em um **loop contínuo**, onde o servo oscila suavemente entre **0° e 180°**, utilizando **incrementos de 5µs a cada 10ms**. Durante essa movimentação, o LED RGB altera sua intensidade, servindo como um **indicador visual da posição do servo**.

---

# Explicação do Código

# Inclusão de Bibliotecas
O código inclui as bibliotecas essenciais para o funcionamento do Raspberry Pi Pico:
```c
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
```
A biblioteca `pico/stdlib.h` é necessária para a comunicação padrão e funções de temporização, enquanto `hardware/pwm.h` fornece suporte para a geração de sinais PWM.

# Definição de Constantes
O código define algumas constantes para garantir que os valores de controle do PWM sejam fáceis de ajustar e manter.
```c
#define SERVO_PIN 22      // Pino GPIO do servomotor
#define LED_PIN 12        // Pino GPIO do LED RGB
#define PWM_FREQ 50       // Frequência do PWM (50Hz, período de 20ms)

#define CLOCK_FREQ 125000000  // Clock do RP2040 (125MHz)
#define DIVIDER 64.0          // Divisor do clock
#define WRAP_VALUE (CLOCK_FREQ / (DIVIDER * PWM_FREQ))  // Valor máximo do contador PWM
```
O valor `WRAP_VALUE` é calculado com base na frequência do clock do Raspberry Pi Pico e no divisor de clock para garantir que a **frequência do PWM seja exatamente 50Hz**.

# Definição dos Tempos de Pulso para Controle do Servo
Os tempos de pulso determinam a posição do servo:
```c
#define PULSE_MIN 500   // 0 graus - 500µs
#define PULSE_90 1470   // 90 graus - 1.47ms
#define PULSE_MAX 2400  // 180 graus - 2.4ms
```
Esses valores representam os tempos de pulso que fazem o servomotor se mover para os ângulos desejados.

# Configuração do PWM
A função `setup_pwm()` inicializa os pinos PWM para o servo e para o LED RGB.
```c
void setup_pwm()
{
    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM);
    gpio_set_function(LED_PIN, GPIO_FUNC_PWM);

    uint slice_servo = pwm_gpio_to_slice_num(SERVO_PIN);
    uint slice_led = pwm_gpio_to_slice_num(LED_PIN);

    pwm_set_clkdiv(slice_servo, DIVIDER);
    pwm_set_wrap(slice_servo, WRAP_VALUE);

    pwm_set_clkdiv(slice_led, DIVIDER);
    pwm_set_wrap(slice_led, WRAP_VALUE);

    pwm_set_gpio_level(SERVO_PIN, PULSE_MIN * WRAP_VALUE / 20000);
    pwm_set_gpio_level(LED_PIN, 0);

    pwm_set_enabled(slice_servo, true);
    pwm_set_enabled(slice_led, true);
}
```
Aqui, os **slices de PWM** são obtidos e configurados com os parâmetros corretos. O PWM é ativado para o servo e o LED.

# Controle do Servo com Espera Temporizada
A função `set_servo_position()` define uma posição fixa para o servo e aguarda um tempo antes de continuar.
```c
void set_servo_position(uint16_t pulse_width, uint16_t wait_time)
{
    pwm_set_gpio_level(SERVO_PIN, pulse_width * WRAP_VALUE / 20000);
    uint16_t led_brightness = (pulse_width - PULSE_MIN) * WRAP_VALUE / (PULSE_MAX - PULSE_MIN);
    pwm_set_gpio_level(LED_PIN, led_brightness);

    printf("Posição ajustada para %d µs. Aguardando %d ms...\n", pulse_width, wait_time);
    sleep_ms(wait_time);
}
```
Essa função converte o tempo de pulso do servo para o **nível do PWM** e também ajusta o brilho do LED proporcionalmente à posição do servo.

# Movimentação Suave do Servo
A função `move_servo_with_led()` permite que o servo **se mova suavemente** entre dois ângulos, ajustando também o LED RGB.
```c
void move_servo_with_led(uint16_t start, uint16_t end)
{
    int step = (start < end) ? STEP_US : -STEP_US;
    uint16_t pos = start;

    while (pos != end)
    {
        pwm_set_gpio_level(SERVO_PIN, pos * WRAP_VALUE / 20000);
        uint16_t led_brightness = (pos - PULSE_MIN) * WRAP_VALUE / (PULSE_MAX - PULSE_MIN);
        pwm_set_gpio_level(LED_PIN, led_brightness);

        sleep_ms(DELAY_MS);
        pos += step;
    }
}
```
Essa função usa um **incremento de 5µs a cada 10ms**, garantindo um **movimento gradual** do servo.

### Função Principal `main()`
No `main()`, o código primeiro posiciona o servo nas posições fixas de 180°, 90° e 0° com esperas de 5 segundos.
```c
int main()
{
    stdio_init_all();
    setup_pwm();

    printf("Movendo servo para 180 graus (2400µs)...\n");
    set_servo_position(PULSE_MAX, 5000);

    printf("Movendo servo para 90 graus (1470µs)...\n");
    set_servo_position(PULSE_90, 5000);

    printf("Movendo servo para 0 graus (500µs)...\n");
    set_servo_position(PULSE_MIN, 5000);

    while (true)
    {
        printf("Movendo suavemente para 180 graus...\n");
        move_servo_with_led(PULSE_MIN, PULSE_MAX);

        printf("Movendo suavemente para 0 graus...\n");
        move_servo_with_led(PULSE_MAX, PULSE_MIN);
    }
}
```
Após a fase inicial, o loop faz o servo se movimentar continuamente entre **0° e 180°**.

---

Este programa fornece um **controle preciso e visual** do servo com um LED RGB.

# Link do Vídeo de Explicação:

https://www.dropbox.com/scl/fi/r4m4lxnx7tyx8o69bnluy/Video_explicacao2.mp4?rlkey=0usrpz3aljz7yoe92qu501igk&st=rha7cjqh&dl=0

# Observações do Experimento com a Ferramenta Educacional BitDogLab

Ao realizar o experimento utilizando a BitDogLab, observou-se que o LED RGB conectado à GPIO 12 variava sua intensidade de brilho de acordo com a posição do servomotor. Isso ocorre porque a intensidade do LED é ajustada proporcionalmente ao ciclo ativo do PWM aplicado ao servo. As principais observações foram:

1. Quando o servo estava na posição de 0 graus (500µs), o LED apagava, indicando o menor nível de duty cycle.

2. Ao movimentar o servo para a posição de 90 graus (1470µs), o LED aumentava seu brilho para um nível intermediário, representando a posição central do servo.

3. Quando o servo atingia a posição de 180 graus (2400µs), o LED apresentava brilho máximo, refletindo o duty cycle máximo configurado.

4. Durante a movimentação suave do servo entre 0° e 180°, foi perceptível que o LED aumentava e diminuía gradativamente sua intensidade, demonstrando um comportamento sincronizado com o deslocamento do servomotor.
