#ifndef MOTORS_H
#define MOTORS_H
#include <stdint.h>

#define ENABLE_LEFT    15
#define MOTOR_LEFT1    1
#define MOTOR_LEFT2    16
#define ENABLE_RIGHT     0
#define MOTOR_RIGHT1     2
#define MOTOR_RIGHT2     3

#define MAX_SPEED       0xFF
#define MIN_SPEED       0x00

void motorPowerOn();
void motorPowerOff();
void setMotorForward();
void setMotorBackward();
void setMotorLeft();
void setMotorRight();

void applyMotorSpeed(uint8_t speed);
void applyMotorLRSpeed(uint8_t speed_left, uint8_t speed_right);


#endif //MOTORS_H
