#include "motors.h"
#include <wiringPi.h>
#include <softPwm.h>
#include <math.h>
#include <stdio.h>
#include <limits>

static uint8_t Speed_Magnitude = MIN_SPEED;

void motorPowerOn()
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

void motorPowerOff()
{
    digitalWrite(MOTOR_LEFT1,  LOW);
    digitalWrite(MOTOR_LEFT2,  LOW);
    digitalWrite(MOTOR_RIGHT1, LOW);
    digitalWrite(MOTOR_RIGHT2, LOW);

    softPwmWrite(ENABLE_LEFT,  0x00);
    softPwmWrite(ENABLE_RIGHT, 0x00);
}

void setMotorForward()
{
    digitalWrite(MOTOR_LEFT1, HIGH);
    digitalWrite(MOTOR_LEFT2, LOW);
    digitalWrite(MOTOR_RIGHT1, HIGH);
    digitalWrite(MOTOR_RIGHT2, LOW);
}

void setMotorBackward()
{
    digitalWrite(MOTOR_LEFT1, LOW);
    digitalWrite(MOTOR_LEFT2, HIGH);
    digitalWrite(MOTOR_RIGHT1, LOW);
    digitalWrite(MOTOR_RIGHT2, HIGH);
}

void setMotorLeft()
{
    digitalWrite(MOTOR_LEFT1, LOW);
    digitalWrite(MOTOR_LEFT2, HIGH);
    digitalWrite(MOTOR_RIGHT1, HIGH);
    digitalWrite(MOTOR_RIGHT2, LOW);
}

void setMotorRight()
{
    digitalWrite(MOTOR_LEFT1, HIGH);
    digitalWrite(MOTOR_LEFT2, LOW);
    digitalWrite(MOTOR_RIGHT1, LOW);
    digitalWrite(MOTOR_RIGHT2, HIGH);
}

void applyMotorSpeed(uint8_t speed)
{
    softPwmWrite(ENABLE_LEFT, speed);
    softPwmWrite(ENABLE_RIGHT, speed);
}

void applyMotorLRSpeed(uint8_t speed_left, uint8_t speed_right)
{
    softPwmWrite(ENABLE_LEFT, speed_left);
    softPwmWrite(ENABLE_RIGHT, speed_right);
}
