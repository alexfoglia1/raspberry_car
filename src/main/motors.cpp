#include <unistd.h>
#include "motors.h"
#include <wiringPi.h>
#include <softPwm.h>
#include <math.h>
#include <stdio.h>
#include <limits>

static bool invert_left_right = false;

void init_motors()
{
    wiringPiSetup();

    pinMode(ENABLE_LEFT, OUTPUT);
    pinMode(ENABLE_RIGHT, OUTPUT);
    pinMode(MOTOR_LEFT1, OUTPUT);
    pinMode(MOTOR_LEFT2, OUTPUT);
    pinMode(MOTOR_RIGHT1, OUTPUT);
    pinMode(MOTOR_RIGHT2, OUTPUT);

    softPwmCreate(ENABLE_LEFT, MIN_SPEED, MAX_SPEED);
    softPwmCreate(ENABLE_RIGHT, MIN_SPEED, MAX_SPEED);
}

void stop_motors()
{
    digitalWrite(MOTOR_LEFT1,  LOW);
    digitalWrite(MOTOR_LEFT2,  LOW);
    digitalWrite(MOTOR_RIGHT1, LOW);
    digitalWrite(MOTOR_RIGHT2, LOW);

    softPwmWrite(ENABLE_LEFT,  0x00);
    softPwmWrite(ENABLE_RIGHT, 0x00);
}

void motors_forward()
{
    digitalWrite(MOTOR_LEFT1, HIGH);
    digitalWrite(MOTOR_LEFT2, LOW);
    digitalWrite(MOTOR_RIGHT1, HIGH);
    digitalWrite(MOTOR_RIGHT2, LOW);

    invert_left_right = false;
}

void motors_backward()
{
    digitalWrite(MOTOR_LEFT1, LOW);
    digitalWrite(MOTOR_LEFT2, HIGH);
    digitalWrite(MOTOR_RIGHT1, LOW);
    digitalWrite(MOTOR_RIGHT2, HIGH);

    invert_left_right = true;
}

void set_motor_speed(uint8_t speed_left, uint8_t speed_right)
{
    softPwmWrite(ENABLE_LEFT, invert_left_right ? speed_left  : speed_left);
    softPwmWrite(ENABLE_RIGHT, invert_left_right ? speed_right : speed_right);
}
