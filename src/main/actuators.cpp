#include "actuators.h"
#include "defs.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <iostream>
#include <wiringPi.h>

static uint8_t throttle_state;

void __attribute__((noreturn)) actuators_task()
{
    int serversock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    int clisock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    wiringPiSetup();
    pinMode(15, OUTPUT);
    digitalWrite(15, LOW);

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
    while (1)
    {
        command_msg cmd_in;
        throttle_msg cmd_resp;
        socklen_t socklen;
        int recvok = recvfrom(serversock, &cmd_in, sizeof(command_msg), 0, reinterpret_cast<struct sockaddr*>(&saddr), &socklen);
        if (recvok)
        {
            int32_t n_throttle_state_s32 = static_cast<int32_t>(throttle_state) + static_cast<int32_t>(cmd_in.throttle_add);
            throttle_state = cmd_in.throttle_add == 0x70 ? 0 :
                             cmd_in.throttle_add == 0x7F ? 0xFF :
                             n_throttle_state_s32 > 0xFF ? 0xFF :
                             n_throttle_state_s32 < 0x00 ? 0x00 :
                             static_cast<uint8_t>(n_throttle_state_s32 & 0xFF);

            digitalWrite(15, throttle_state == 0xFF);
            cmd_resp.header.msg_id = COMMAND_MSG_ID;
            cmd_resp.throttle_state = throttle_state;
            if (sendto(clisock, &cmd_resp, sizeof(cmd_resp), 0, reinterpret_cast<struct sockaddr*>(&daddr), sizeof(daddr)) > 0)
            {
                std::cout << "Throttle state out: Success" << std::endl;
            }

        }

    }
}
