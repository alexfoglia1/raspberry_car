#ifndef ACTUATORS_H
#define ACTUATORS_H
#include <stdint.h>

void idle_to_running();
void running_to_idle();
void joystick_handler();

typedef enum
{
    idle = 0,
    running = 1
} sys_state_t;

typedef enum
{
    received_start = 0,
    received_stop = 1,
    received_joystick = 2
} sys_event_t;

typedef void (*sys_action_t)();

typedef struct
{
    sys_state_t actual_state;
    sys_event_t event;
    sys_state_t next_state;
    sys_action_t action;
} sys_state_machine_entry_t;

typedef struct
{
    uint8_t throttle_state = 0x00;
    int8_t x_axis = 0;
    bool is_brake = false;
} motor_state_t;


void actuators_task(int millis);



#endif // ACTUATORS_H
