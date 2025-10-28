#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define LED0_PIN 22
#define LED1_PIN 21
#define LED2_PIN 20
#define SW0_PIN 9
#define SW1_PIN 8
#define SW2_PIN 7
#define TOP 999

//3 LEDs on/off/dim together. Need to release. Holding doesn't cause keep lighting.
//Sw1 is main switch. Sw0 increase brightness; SW2 decrease brightness.
    //holding to shift; if leds are off, no effect;
//if leds brightness have been changed, it should remain when next time toggle them.
    //leds-ON, brightness=0, sw1==0, brightness to 50%.(instead of turn to off)
    //leds-on, brightness=0, sw0==0, brightness increase(brightness is lowest and press sw0 to brigher)
//divider 125, PWM-frequency 1kHz, Top=999?

static uint pwm_gpio_to_slice_num(uint gpio);
static uint pwm_gpio_to_channel (uint gpio);
static void pwm_set_enabled (uint slice_num, bool enabled);
static pwm_config pwm_get_default_config (void);
static void pwm_config_set_clkdiv_int (pwm_config * c, uint32_t div_int);
static void pwm_config_set_wrap (pwm_config * c, uint16_t wrap);
static void pwm_init (uint slice_num, pwm_config * c, bool start);
static void pwm_set_chan_level (uint slice_num, uint chan, uint16_t level);
void gpio_set_function (uint gpio, gpio_function_t fn);
static bool gpio_get (uint gpio);


int main() {
    stdio_init_all();
    gpio_init(SW0_PIN);
    gpio_set_dir(SW0_PIN, GPIO_IN);
    gpio_pull_up(SW0_PIN);

    gpio_init(SW1_PIN);
    gpio_set_dir(SW1_PIN, GPIO_IN);
    gpio_pull_up(SW1_PIN);

    gpio_init(SW2_PIN);
    gpio_set_dir(SW2_PIN, GPIO_IN);
    gpio_pull_up(SW2_PIN);

    // get slice and channel of your GPIO pin
    const uint led0_pin = LED0_PIN;
    const uint led1_pin = LED1_PIN;
    const uint led2_pin = LED2_PIN;

    const uint slice0=pwm_gpio_to_slice_num(led0_pin);
    const uint slice1=pwm_gpio_to_slice_num(led1_pin);
    const uint slice2=pwm_gpio_to_slice_num(led2_pin);

    const uint channel0=pwm_gpio_to_channel (led0_pin);
    const uint channel1=pwm_gpio_to_channel (led1_pin);
    const uint channel2=pwm_gpio_to_channel (led2_pin);

    //stop PWM
    pwm_set_enabled (slice0, false);
    pwm_set_enabled (slice1, false);
    pwm_set_enabled (slice2, false);

    //default PWM configuration
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv_int(&config, 125);
    pwm_config_set_wrap(&config,TOP);

    //the config you created with start set to false
    pwm_init(slice0, &config, false);
    pwm_init(slice1, &config, false);
    pwm_init(slice2, &config, false);
    //set level (CC) → duty cycle
    pwm_set_chan_level(slice0,channel0,0);
    pwm_set_chan_level(slice1,channel1,0);
    pwm_set_chan_level(slice2,channel2,0);

    //select PWM mode
    gpio_set_function(LED0_PIN, GPIO_FUNC_PWM);
    gpio_set_function(LED1_PIN, GPIO_FUNC_PWM);
    gpio_set_function(LED2_PIN, GPIO_FUNC_PWM);

    //start PWM
    pwm_set_enabled (slice0, true);
    pwm_set_enabled (slice1, true);
    pwm_set_enabled (slice2, true);

    //set default state of leds.
    bool leds_on=false;
    int brightness=500;
    bool sw1_released=true;

    while (1) {
        //pullup, press=false
        bool sw1_pressed=!gpio_get(SW1_PIN);
        bool sw2_pressed=!gpio_get(SW2_PIN);
        bool sw0_pressed=!gpio_get(SW0_PIN);
        if (sw1_pressed&&sw1_released) {
            sw1_released=false;

            if (leds_on&&brightness==0) {
                brightness=500;
            }else {
                leds_on=!leds_on;
                //test whether when sw1_press works.
                printf("Leds toggled\n");
            }
        }else if (!sw1_pressed){
            sw1_released=true;
        }

        int current_pwm=0;
        // only leds are on could be dimmed.
        if (leds_on) {
            if (sw0_pressed) {
                if (brightness<TOP) {
                    //brightness++ is too slow.
                    brightness=brightness+10;
                }
            }
            if (sw2_pressed) {
                if (brightness>0) {
                    brightness=brightness-10;
                }
            }
            current_pwm=brightness;
        }
        pwm_set_chan_level(slice0,channel0,current_pwm);
        pwm_set_chan_level(slice1,channel1,current_pwm);
        pwm_set_chan_level(slice2,channel2,current_pwm);

        sleep_ms(10);
    }
}
