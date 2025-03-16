#include <stdio.h>
#include <string.h>  
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"

const int TRIGGER_PIN = 16;
const int ECHO_PIN = 17;  

volatile int alarmFlag = 0;

long long int alarm_callback(long int id, void *user_data) {
    alarmFlag = 1;
    return 0;
}

float timeToDistance(int time) {
    return (time * 0.0343) / 2;
}

int main() {
    stdio_init_all();
    gpio_init(TRIGGER_PIN);
    gpio_set_dir(TRIGGER_PIN, GPIO_OUT);
    gpio_init(ECHO_PIN);
    gpio_set_dir(ECHO_PIN, GPIO_IN);

    printf("Enter 's' to start measuring, 'k' to stop.\n");

    while (true) {
        int ch = getchar_timeout_us(0);
        
        if (ch == 's') {
            printf("Starting measurements. Type 'k' to stop.\n");

            while (true) {
                ch = getchar_timeout_us(0);
                if (ch != PICO_ERROR_TIMEOUT && ch == 'k') {
                    printf("Measurement halted.\n");
                    break;
                }

                alarmFlag = 0;
                gpio_put(TRIGGER_PIN, 1);
                sleep_us(10);
                gpio_put(TRIGGER_PIN, 0);

                alarm_id_t alarmId = add_alarm_in_ms(1000, alarm_callback, NULL, false);

                while (alarmFlag == 0) {
                    while (gpio_get(ECHO_PIN) == 0);
                    absolute_time_t startTime = get_absolute_time();
                    while (gpio_get(ECHO_PIN) == 1);
                    absolute_time_t endTime = get_absolute_time();

                    if (alarmFlag) {
                        printf("Error during measurement, retrying...\n");
                        break;
                    }

                    int echoTime = absolute_time_diff_us(startTime, endTime);
                    if (echoTime > 0) {
                        cancel_alarm(alarmId);
                        float distanceCm = timeToDistance(echoTime);
                        printf("Measured distance in cm: %.2f\n", (double)distanceCm);
                        break;
                    }
                }
                sleep_ms(300);  
            }
        }
    }

    return 0;
}
