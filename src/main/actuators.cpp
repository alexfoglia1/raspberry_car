#include "actuators.h"
#include "motors.h"
#include "defs.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <limits>
#include <math.h>

#define _min_(a,b)(a<b?a:b)

static uint8_t throttle_state = 0x00;
static dir_t direction_state = DIR_FWD;
static dir_t last_direction_state = DIR_FWD;
static int8_t x_axis = 0;
static bool is_brake = false;

void distribute_speed(uint8_t speed_magnitude)
{
    double left_percentage = 1.0 - (double) (-std::numeric_limits<int8_t>::min() + x_axis) /
                                   (double) (std::numeric_limits<int8_t>::max() - std::numeric_limits<int8_t>::min());

    double right_percentage = 1.0 - left_percentage;


    double right_speed = _min_(MAX_SPEED, speed_magnitude * 2 * left_percentage);
    double left_speed =  _min_(MAX_SPEED, speed_magnitude * 2 * right_percentage);

    uint8_t ils = static_cast<uint8_t>(left_speed);
    uint8_t irs = static_cast<uint8_t>(right_speed);

    //printf("speed magnitude(%d), left motor(%d), right motor(%d)\n", speed_magnitude, ils, irs);
    applyMotorLRSpeed(ils, irs);
}

void set_motors()
{
    if (is_brake)
    {
         is_brake = false;
         if (last_direction_state == DIR_FWD)
         {
             setMotorBackward();
             applyMotorSpeed(0xFF);
             usleep(100000);
             setMotorForward();
         }
         else if (last_direction_state == DIR_BWD)
         {
            setMotorForward();
            applyMotorSpeed(0xFF);
            usleep(100000);
            setMotorBackward();
         }
         else if (last_direction_state == DIR_LFT)
         {
            setMotorRight();
            applyMotorSpeed(0xFF);
            usleep(100000);
            setMotorLeft();
         }
         else if (last_direction_state == DIR_RGT)
         {
            setMotorLeft();
            applyMotorSpeed(0xFF);
            usleep(100000);
            setMotorRight();
         }
    }

    if (direction_state == DIR_FWD)
    {
        setMotorForward();
    }
    else if (direction_state == DIR_BWD)
    {
        setMotorBackward();
    }
    else if (direction_state == DIR_LFT)
    {
        setMotorLeft();
    }
    else if (direction_state == DIR_RGT)
    {
        setMotorRight();
    }
    
    applyMotorSpeed(throttle_state);
}

void __attribute__((noreturn)) actuators_task()
{
    int serversock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    int clisock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_in saddr;
    struct sockaddr_in daddr;

    memset(&saddr, 0x00, sizeof(struct sockaddr_in));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port = htons(THRPORT);
    
    memset(&daddr, 0x00, sizeof(struct sockaddr_in));
    daddr.sin_family = AF_INET;
    daddr.sin_addr.s_addr = inet_addr(PC_ADDRESS.c_str());
    daddr.sin_port = htons(THRPORT);

    int res = bind(serversock, reinterpret_cast<struct sockaddr*>(&saddr), sizeof(struct sockaddr_in));
    
    motorPowerOn();
    set_motors();
    
    bool wait_ad = true;
    while (1)
    {
        command_msg cmd_in;
        joystick_xy_msg js_xy_in;
        joystick_throttle_msg js_th_in;
        joystick_break_msg js_br_in;
        throttle_msg cmd_resp;
        socklen_t socklen;
        size_t maxrecvlen = sizeof(command_msg);
	
        int recvok = recv(serversock, &cmd_in, maxrecvlen, 0);//, reinterpret_cast<struct sockaddr*>(&saddr), &socklen);
        
        if (recvok)
        {
            
            if (cmd_in.header.msg_id == COMMAND_MSG_ID)
            {
                printf("rx command msg 0x%X!\n", (cmd_in.throttle_add & 0xFF));
                printf("wait ad: %s\n", wait_ad ? "true" : "false");
                if(wait_ad == true && ((cmd_in.throttle_add & 0xFF) == 0xAD))
                {
                    printf("Controller is alive, start actuating!\n");
                    wait_ad = false;
                }
                if (wait_ad == false && ((cmd_in.throttle_add & 0xFF) == 0xDE))
                {
                    printf("Controller is quitting, wait for 0xAD to continue actuating.\n");
                    wait_ad = true;
                }
                
                if (wait_ad)
                {
                    continue;
                }
                bool is_straight = (last_direction_state == DIR_BWD || last_direction_state == DIR_FWD);
                bool is_lateral  = (last_direction_state == DIR_LFT || last_direction_state == DIR_RGT);
                is_brake = (throttle_state > BREAK_THRESHOLD_DIR_STRAIGHT && is_straight && cmd_in.throttle_add == 0x70) ||
                           (throttle_state > BREAK_THRESHOLD_DIR_LATERAL && is_lateral  && cmd_in.throttle_add == 0x70) ;

                int32_t n_throttle_state_s32 = static_cast<int32_t>(throttle_state) + static_cast<int32_t>(cmd_in.throttle_add);
                throttle_state = cmd_in.throttle_add == THROTTLE_MIN ? 0x00 :
                                 cmd_in.throttle_add == THROTTLE_MAX ? 0xFF :
                                 n_throttle_state_s32 > 0xFF ? 0xFF :
                                 n_throttle_state_s32 < 0x00 ? 0x00 :
                                 static_cast<uint8_t>(n_throttle_state_s32 & 0xFF);

                direction_state = cmd_in.direction;
                if (direction_state != DIR_NONE)
                {
                    last_direction_state = direction_state;
                }
                set_motors();
            }

            else if (cmd_in.header.msg_id == JS_XY_MSG_ID && !wait_ad)
            {
                memcpy(&js_xy_in, &cmd_in, sizeof(joystick_xy_msg));
                //printf("x axis(%d)\n", js_xy_in.x_axis);
                x_axis = js_xy_in.x_axis;
                last_direction_state = DIR_FWD;
                distribute_speed(throttle_state);

            }
            else if(cmd_in.header.msg_id == JS_BR_MSG_ID && !wait_ad)
            {
                memcpy(&js_br_in, &cmd_in, sizeof(joystick_break_msg));
                last_direction_state = DIR_BWD;
                setMotorBackward();
		//printf("received backward(%d)\n", js_br_in.backward);
                distribute_speed(js_br_in.backward);
                throttle_state = js_br_in.backward;
            }
            else if (cmd_in.header.msg_id == JS_TH_MSG_ID && !wait_ad)
            {
                memcpy(&js_th_in, &cmd_in, sizeof(joystick_throttle_msg));
                last_direction_state = DIR_FWD;
                setMotorForward();
                distribute_speed(js_th_in.throttle_state);
           	throttle_state = js_th_in.throttle_state; 
	   }

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
            else printf("tx throttle state out to %s\n", PC_ADDRESS.c_str());
        }
	else
	{
		perror("RECV");
	}
    }

}
