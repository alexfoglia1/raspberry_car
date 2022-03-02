#include "lights.h"
#include <wiringPi.h>

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
