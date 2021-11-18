#include <unistd.h>
#include "motors.h"
#include <wiringPi.h>
#include <softPwm.h>
#include <math.h>
#include <stdio.h>
#include <limits>

#define LEFT(speed)  ((uint8_t)(((float)(speed) * 1.0f)))
#define RIGHT(speed) ((uint8_t)(((float)(speed) * 0.85f)))

static uint8_t Speed_Magnitude = MIN_SPEED;
static bool invert_left_right = false;

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
    invert_left_right = false;
}

void setMotorBackward()
{
    digitalWrite(MOTOR_LEFT1, LOW);
    digitalWrite(MOTOR_LEFT2, HIGH);
    digitalWrite(MOTOR_RIGHT1, LOW);
    digitalWrite(MOTOR_RIGHT2, HIGH);
    invert_left_right = true;
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
    //printf("SoftPWMWrite %d\n", speed);
    softPwmWrite(ENABLE_LEFT, invert_left_right ? RIGHT(speed) : LEFT(speed));
    softPwmWrite(ENABLE_RIGHT, invert_left_right ? LEFT(speed) : RIGHT(speed));
    //usleep(1000);
    //double duty_left =  (double)(speed)/255.0;
    //double duty_right = (double)(speed)/255.0;
    //double v_out_left = duty_left * 5;
    //double v_out_right = duty_right * 5;
    //printf("v_out_left %f V\n", v_out_left);
    //printf("v_out_right %f V\n\n", v_out_right);
}

void applyMotorLRSpeed(uint8_t speed_left, uint8_t speed_right)
{
    softPwmWrite(ENABLE_LEFT, invert_left_right ? RIGHT(speed_left)  : LEFT(speed_left));
    softPwmWrite(ENABLE_RIGHT, invert_left_right ? LEFT(speed_right) : RIGHT(speed_right));

    //double duty_left =  (double)(speed_left)/255.0;
    //double duty_right = (double)(speed_left)/255.0;
    //double v_out_left = duty_left * 5;
    //double v_out_right = duty_right * 5;
    //printf("v_out_left %f V\n", v_out_left);
    //printf("v_out_right %f V\n\n", v_out_right);

}
