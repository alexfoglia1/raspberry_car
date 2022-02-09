#ifndef DEFS_H
#define DEFS_H

#define PROJNAME "Raspberry PI 4 Car v01.01"

#define ERR_CREATESOCKDATA "Could not create data socket"
#define ERR_BINDSOCKDATA "Could not bind data socket"
#define ERR_RECVSOCKDATA "Could not receive from data socket"
#define ERR_CREATESOCKVIDEO "Could not create video socket"
#define ERR_BINDSOCKVIDEO "Could not bind video socket"
#define ERR_RECVSOCKVIDEO "Could not receive from video socket"
#define ERR_UNINITIALIZED_SOCKET "Cannot receive: socket uninitialized"

#define OK_CREATESOCKDATA "Data server socket was successfully created"
#define OK_BINDSOCKDATA "Data server socket was successfully binded"
#define OK_CREATESOCKVIDEO "Video server socket was successfully created"
#define OK_BINDSOCKVIDEO "Video server socket was successfully binded"
#define OK_CAN_RECEIVE_DATA "Waiting for data"
#define OK_CAN_RECEIVE_VIDEO "Waiting for video"
#define OK_RECVSOCKDATA "Received data from data socket"
#define OK_RECVSOCKVIDEO "Received data from video socket"

#define ERR_UNKNOWN_SOURCE "Received message from unknown source"
#define OK_IMU "Received IMU msg"
#define OK_SPEED "Received SPEED msg"
#define OK_ATTITUDE "Received ATTITUDE msg"
#define OK_RADIATION "Received RADIATION msg"
#define OK_IMAGE "Received IMAGE msg"

#define ATTPORT 1111
#define RENPORT 2222
#define JOYPORT 3333
#define TGTPORT 4444
#define VLTPORT 5555
#define DETPORT 9999 //tx to tegra

#define VOLTAGE_MSG_ID          1
#define ATTITUDE_MSG_ID         2
#define ACTUATORS_STATE_MSG_ID  3
#define JS_ACC_MSG_ID           7
#define JS_BRK_MSG_ID           8
#define TARGET_MSG_ID           9

#define MAX_IMAGESIZE 60000
#define IMAGE_ROWS    650
#define IMAGE_COLS    1200

#define BREAK_THRESHOLD_DIR_STRAIGHT 0x20
#define BREAK_THRESHOLD_DIR_LATERAL  0x40
#define THROTTLE_MAX 0x7F
#define THROTTLE_MIN 0x70
#define MAX_TARGETS 10

#define __UNUSED__(x) (*x)

#include <stdint.h>
#include <string>

extern std::string PC_ADDRESS;
extern std::string TEGRA_ADDRESS;

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
}  joystick_msg;

typedef struct __attribute__((packed))
{
    msg_header header;
    uint8_t throttle_state;
    uint8_t system_state;
} actuators_state_msg;

#endif //DEFS_H
