#include "mbed.h"
#include "board_freedom.h"
#include "adc.h"
#include "oled_ssd1322.h"
#include <cstdint>
#include <cstdio> 
#include <cstring> 
#include <string>
 
//smart-home SYS code
// main() runs in its own thread in the OS
#define MESSAGE_MAX_SIZE 50
#define BR 500ms

int main()
{
    board_init();

    u8g2_ClearBuffer(&oled);
    u8g2_SetFont(&oled, u8g2_font_6x12_mr);
    u8g2_SendBuffer(&oled);

    DigitalOut Garden_Lamp(PTB18);
    DigitalOut Garage_Door (PTB11);  // PTE24 IS DOOR
    DigitalIn  Car_Sensor(PTE25);
    DigitalOut Servo_Frodo(PTE24);
    DigitalIn Button(PTB7);
    DigitalOut Servo_Curtain(PTC7);
    DigitalOut door_closed_led(PTB2);
    DigitalOut door_open_led(PTB3);
    DigitalOut In_Mux_A(PTA8);
    DigitalOut In_Mux_B(PTA9);
    DigitalOut Fan (PTC12);  // fan


    door_closed_led = 1;
    door_open_led = 0;

    Servo_Frodo = 1;
    Servo_Curtain = 0;

    // for indoor temperature sensor

    In_Mux_A = 0;
    In_Mux_B = 0;

    // bool 
    char status_of_fan[] = "OFF";
    std:string s = "OFF";

    Garden_Lamp = 0;
    char message[MESSAGE_MAX_SIZE + 1];
    message[MESSAGE_MAX_SIZE] = '\0';

    pwm3.write(0.5);  // turn heater on

    while (true)
    { 
        if (!Button) Servo_Frodo = 0;  // door closes when it is 1
        else Servo_Frodo = 1;

        ThisThread::sleep_for(100ms);
        uint16_t optosensor_value = adc_read(1);  // read the optosensor value from pin adc1_se16

        // read indoor temperature
        uint16_t indoor_temp_val = adc_read(0);
        // convert to temp
        float voltage = (indoor_temp_val / 65535.0) * 3.0;
        if (voltage < 0.25 || voltage > 1.7) {  // Assuming sensor normally outputs between 0.2V to 2.8V
            if (voltage < 0.25) {
                snprintf(message, MESSAGE_MAX_SIZE, "Sensor error! Check connection for indoor");
                printf("%s\n", message);
                u8g2_ClearBuffer(&oled);
                u8g2_DrawUTF8(&oled, 10, 10, message);
                u8g2_DrawUTF8(&oled, 10, 20, "tempereature sensor");
                u8g2_SendBuffer(&oled);
            } else if (voltage > 1.7) {
                snprintf(message, MESSAGE_MAX_SIZE, "System overheating.");
                printf("%s\n", message);
            }
        }

        float temperature = (voltage * 1000 - 400) / 19.5;

        
        if (temperature <= 22) {
            pwm3.write(0.5);  // turn the heater on
            Fan = 0;
            s = "OFF";
        } else if (temperature >= 24.7) {  // account for delays between sensor readings
            Fan = 1;
            pwm3.write(0.0);  // turn heater off
            s = "ON";
        }  else if (temperature >= 23 && temperature  <= 23.5) {
            Fan = 0;
            s = "OFF";
        }

        // snprintf(message, MESSAGE_MAX_SIZE, "Indoor Temperature: %5.02f, Fan Status: %s", temperature, s.c_str());
        snprintf(message, MESSAGE_MAX_SIZE, "Indoor Temperature: %5.02f", temperature);
        printf("%s\n", message);
        if (voltage > 0.25) {
            u8g2_ClearBuffer(&oled);
            u8g2_DrawUTF8(&oled, 10, 10, message);
        }

        snprintf(message, MESSAGE_MAX_SIZE, "Fan Status: %s", s.c_str());
        printf("%s\n", message);
        u8g2_DrawUTF8(&oled, 10, 30, message);

        // display on lcd
        u8g2_SendBuffer(&oled);

        // indoor temperature sensor failure
        if (indoor_temp_val == 0xFFFF) {
            printf("ADC read error: Component failure detected\n");
        } 

        // to obtain illuminance values in lux
        double multiplier = 400.0/11000.0;
        double lux = optosensor_value * multiplier;
        // printf("%5f\n", lux);

        // if too dark (based on a reference value and adjusted)
        if (lux < 400) {
            Garden_Lamp = 1;
            Servo_Curtain=1;
        } else {
            Garden_Lamp = 0;
            Servo_Curtain = 0;
        }
       
        // if car not detected
        if (Car_Sensor == 0) {
            ThisThread::sleep_for(1s);
            Garage_Door = 1;  // open garage door
            ThisThread::sleep_for(4s); // door opening delay
            door_open_led = 1;
            door_closed_led = 0;
            pwm2.write (0.5);
            pwm2.period_ms(10);  // turn on lamp garage door
            ThisThread::sleep_for(10s); // delay to keep the door open for a limited time
            Garage_Door = 0; 
            door_open_led = 0;
            door_closed_led = 1;
        }
       
    }
}





