#include "mbed.h"
#include "board_freedom.h"
#include "adc.h"
#include "oled_ssd1322.h"
#include <cstdint>
#include <cstdio>

#define BR 500ms
#define MESSAGE_MAX_SIZE 50

int main() {
    board_init();
    u8g2_ClearBuffer(&oled);
    u8g2_SetFont(&oled, u8g2_font_6x12_mr);
    u8g2_SendBuffer(&oled);
    PwmOut heater_power(PTC2);
    heater_power = 1;

    char message[MESSAGE_MAX_SIZE + 1];
    message[MESSAGE_MAX_SIZE] = '\0'; 
    DigitalOut led(LED1);
    DigitalOut greenLed(PTB3);
    greenLed = 1;

    while (true) {
        uint16_t analog_in_value = adc_read(1);
        float voltage = (analog_in_value / 65535.0) * 3.0;
        float temperature = (voltage * 1000 - 400) / 19.5;

        // Detect sensor malfunction or disconnection
        if (voltage < 0.25 || voltage > 1.7) {  // Assuming sensor normally outputs between 0.2V to 2.8V
            if (voltage < 0.25) {
                snprintf(message, MESSAGE_MAX_SIZE, "Sensor error! Check connection.");
            } else if (voltage > 1.7) {
                snprintf(message, MESSAGE_MAX_SIZE, "System overheating.");
            }
            heater_power = 0;  // turn off heater if sensor fails
            led = 1;  // stop led glows
            greenLed=0;  // stop led glows
        } else {
            if (temperature >= 30 && temperature < 35) {
                greenLed = 1;
            } else {
                greenLed = 0;
            }

            if (temperature <= 30.1) {
                heater_power = 1;
                led = 0;
                ThisThread::sleep_for(BR);
            }
            if (temperature >= 34.9) {
                heater_power = 0;
                led = 1;
            }

            snprintf(message, MESSAGE_MAX_SIZE, "Value is %-5d, temperature is %5.02f", analog_in_value , temperature);
        }

        u8g2_ClearBuffer(&oled);
        u8g2_DrawUTF8(&oled, 10, 10, message);
        u8g2_SendBuffer(&oled);
        printf("%s\n", message);
        ThisThread::sleep_for(100ms);
    }
}
