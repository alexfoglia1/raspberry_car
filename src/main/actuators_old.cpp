#include "actuators.h"
#include "defs.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <iostream>
#include <wiringPi.h>
#include <softPwm.h>

#define ENABLE_RIGHT 15
#define MOTOR_RIGHT1 1
#define MOTOR_RIGHT2 16
#define ENABLE_LEFT 0
#define MOTOR_LEFT1 2
#define MOTOR_LEFT2 3


static uint8_t throttle_state = 0x00;
static dir_t direction_state = DIR_FWD;
static bool is_brake = false;

void set_motors()
{
    if (direction_state == DIR_FWD)
    {
        digitalWrite(MOTOR_LEFT1, HIGH);
        digitalWrite(MOTOR_LEFT2, LOW);
        digitalWrite(MOTOR_RIGHT1, HIGH);
        digitalWrite(MOTOR_RIGHT2, LOW);

    }
    else if (direction_state == DIR_BWD)
    {
        digitalWrite(MOTOR_LEFT1, LOW);
        digitalWrite(MOTOR_LEFT2, HIGH);
        digitalWrite(MOTOR_RIGHT1, LOW);
        digitalWrite(MOTOR_RIGHT2, HIGH);
    }
    else if (direction_state == DIR_LFT)
    {
        digitalWrite(MOTOR_LEFT1, LOW);
        digitalWrite(MOTOR_LEFT2, HIGH);
        digitalWrite(MOTOR_RIGHT1, HIGH);
        digitalWrite(MOTOR_RIGHT2, LOW);
    }
    else if (direction_state == DIR_RGT)
    {
        digitalWrite(MOTOR_LEFT1, HIGH);
        digitalWrite(MOTOR_LEFT2, LOW);
        digitalWrite(MOTOR_RIGHT1, LOW);
        digitalWrite(MOTOR_RIGHT2, HIGH);
    }
    
    if (is_brake)
    {
         is_brake = false;
         if (direction_state == DIR_FWD)
         {
             digitalWrite(MOTOR_LEFT1, LOW);
             digitalWrite(MOTOR_LEFT2, HIGH);
             digitalWrite(MOTOR_RIGHT1, LOW);
             digitalWrite(MOTOR_RIGHT2, HIGH);
             
         }
    }
    softPwmWrite(ENABLE_LEFT, throttle_state);
    softPwmWrite(ENABLE_RIGHT, throttle_state);

}

void __attribute__((noreturn)) actuators_task()
{
    int serversock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    int clisock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    wiringPiSetup();

    pinMode(ENABLE_LEFT, OUTPUT);
    pinMode(ENABLE_RIGHT, OUTPUT);
    pinMode(MOTOR_LEFT1, OUTPUT);
    pinMode(MOTOR_LEFT2, OUTPUT);
    pinMode(MOTOR_RIGHT1, OUTPUT);
    pinMode(MOTOR_RIGHT2, OUTPUT);

    softPwmCreate(ENABLE_LEFT, 0x00, 0xFF);
    softPwmCreate(ENABLE_RIGHT, 0x00, 0xFF);

    struct sockaddr_in saddr;
    struct sockaddr_in daddr;

    memset(&saddr, 0x00, sizeof(struct sockaddr_in));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port = htons(DATPORT);

    memset(&daddr, 0x00, sizeof(struct sockaddr_in));
    daddr.sin_family = AF_INET;
    daddr.sin_addr.s_addr = inet_addr(PC_ADDRESS);
    daddr.sin_port = htons(DATPORT);

    bind(serversock, reinterpret_cast<struct sockaddr*>(&saddr), sizeof(struct sockaddr_in));
    set_motors();
    while (1)
    {
        command_msg cmd_in;
        throttle_msg cmd_resp;
        socklen_t socklen;
        int recvok = recvfrom(serversock, &cmd_in, sizeof(command_msg), 0, reinterpret_cast<struct sockaddr*>(&saddr), &socklen);
        if (recvok)
        {
	    is_brake = (throttle_state != 0x00 && cmd_in.thottle_add == 0x70);
            int32_t n_throttle_state_s32 = static_cast<int32_t>(throttle_state) + static_cast<int32_t>(cmd_in.throttle_add);
            throttle_state = cmd_in.throttle_add == 0x70 ? 0 :
                             cmd_in.throttle_add == 0x7F ? 0xFF :
                             n_throttle_state_s32 > 0xFF ? 0xFF :
                             n_throttle_state_s32 < 0x00 ? 0x00 :
                             static_cast<uint8_t>(n_throttle_state_s32 & 0xFF);

            direction_state = cmd_in.direction;
	    
            set_motors();

            cmd_resp.header.msg_id = COMMAND_MSG_ID;
            cmd_resp.throttle_state = throttle_state;
            if (sendto (
                clisock, &cmd_resp, sizeof(cmd_resp),
                0,
                reinterpret_cast<struct sockaddr*>(&daddr),
                sizeof(daddr)
                ) <= 0)
            {
                std::cout << "Throttle state out: Failure" << std::endl;
            }
        }



    }
}
