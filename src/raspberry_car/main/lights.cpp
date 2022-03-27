#include "lights.h"
#include <unistd.h>
#include <wiringPi.h>
#include <stdio.h>

void init_lights()
{
    wiringPiSetup();
    pinMode(LEFT_LIGHT, OUTPUT);
    pinMode(RIGHT_LIGHT, OUTPUT);
    pinMode(CENTRAL_LIGHT, OUTPUT);
    digitalWrite(LEFT_LIGHT, LOW);
    digitalWrite(RIGHT_LIGHT, LOW);
    digitalWrite(CENTRAL_LIGHT, LOW);
}

void left_light_on()
{
    digitalWrite(LEFT_LIGHT, HIGH);
}

void right_light_on()
{
    digitalWrite(RIGHT_LIGHT, HIGH);
}

void left_light_off()
{
    digitalWrite(LEFT_LIGHT, LOW);
}

void right_light_off()
{
    digitalWrite(RIGHT_LIGHT, LOW);
}

void central_light_on()
{
    digitalWrite(CENTRAL_LIGHT, HIGH);
}

void central_light_off()
{
    digitalWrite(CENTRAL_LIGHT, LOW);
}

void shutdown_lights()
{
    left_light_off();
    right_light_off();
    central_light_off();
}

void light_boot_sequence()
{
    for(int i = 0; i < 3; i++)
    {
        left_light_on();
        usleep(100 * 1000);
        left_light_off();
        central_light_on();
        usleep(100 * 1000);
        central_light_off();
        right_light_on();
        usleep(100 * 1000);
        right_light_off();
        usleep(500 * 1000);
    }
    for (int i = 0; i < 2; i++)
    {
        left_light_on();
        central_light_on();
        right_light_on();
        usleep(250 * 1000);
        left_light_off();
        central_light_off();
        right_light_off();
        usleep(250 * 1000);
    }

}

void light_motors_on_sequence()
{
    for (int i = 0; i < 2; i++)
    {
        left_light_on();
        central_light_on();
        right_light_on();
   
        usleep(250 * 1000);

        left_light_off();
        central_light_off();
        right_light_off();
       
        usleep(250 * 1000);
    }

}

void light_motors_off_sequence()
{
    for (int i = 0; i < 5; i++)
    {
         left_light_on();
         usleep(100 * 1000);
         left_light_off();
         central_light_on();
         usleep(100 * 1000);
         central_light_off();
         right_light_on();
         usleep(100 * 1000);
         right_light_off();
    }

}

void light_imu_failure_sequence()
{
    right_light_on();
    sleep(1);
    right_light_off();
}

void light_arduino_failure_sequence()
{
    left_light_on();
    sleep(1);
    left_light_off();
}

void light_camera_failure_sequence()
{
    central_light_on();
    sleep(1);
    central_light_off();
    
}

void light_no_network_sequence()
{
    left_light_on();
    right_light_on();
    sleep(1);
    left_light_off();
    right_light_off();
}

