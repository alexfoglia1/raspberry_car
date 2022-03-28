#ifndef DEFS_H
#define DEFS_H

#define PROJNAME "Raspberry PI 4 Car 01.00-R"

#define LOCAL_PORT_IMU		    7777
#define REMOTE_PORT_DATA            8888
#define REMOTE_PORT_VIDEO_PC        2222

#define VOLTAGE_MSG_ID          1
#define ATTITUDE_MSG_ID         2
#define ACTUATORS_STATE_MSG_ID  3
#define JS_ACC_MSG_ID           7
#define JS_BRK_MSG_ID           8
#define TARGET_MSG_ID           9

#define MAX_IMAGESIZE 60000
#define IMAGE_ROWS    650
#define IMAGE_COLS    1200

#define __UNUSED__(x) (*x)

#include <stdint.h>
#include <string>

extern std::string PC_ADDRESS;

typedef struct
{
    uint32_t msg_id;
} msg_header;

typedef struct __attribute__((packed))
{
    uint32_t msg_id;
    float motor_voltage;
} voltage_msg;

typedef struct __attribute__((packed))
{
    msg_header header;
    double pitch;
    double roll;
    double yaw;
} attitude_msg;

typedef struct __attribute__((packed))
{
    uint16_t len;
    uint8_t data[MAX_IMAGESIZE];
} image_msg;

typedef struct __attribute__((packed))
{
    msg_header header;
    uint8_t throttle_state;
    int8_t x_axis;
    int8_t y_axis;
    bool start_flag;
    bool stop_flag;
    bool left_light_on;
    bool left_light_off;
    bool right_light_on;
    bool right_light_off;
    bool central_light_on;
    bool central_light_off;
}  joystick_msg;

typedef struct __attribute__((packed))
{
    msg_header header;
    uint8_t throttle_state;
    uint8_t system_state;
} actuators_state_msg;

#endif //DEFS_H
