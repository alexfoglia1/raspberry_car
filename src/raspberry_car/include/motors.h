#ifndef MOTORS_H
#define MOTORS_H
#include <stdint.h>

#define ENABLE_RIGHT    4
#define MOTOR_RIGHT1    1
#define MOTOR_RIGHT2    16
#define ENABLE_LEFT     0
#define MOTOR_LEFT1     2
#define MOTOR_LEFT2     3

#define MAX_SPEED       0xFF
#define MIN_SPEED       0x00

void init_motors();
void stop_motors();
void motors_forward();
void motors_backward();
void set_motor_speed(uint8_t speed_left, uint8_t speed_right);


#endif //MOTORS_H
