#include "health.h"
#include "defs.h"
#include "motors.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <string.h>

const int COUNT_TO_TIMEOUT = 6;

bool back = false;

void handle_timeout()
{
    printf("Health stauts TIMEOUT: Force motor speed to zero\n");
    applyMotorSpeed(0x00);
    motorPowerOff();
back=true;
}

void health_task(int pid)
{
    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct timeval read_timeout;
    read_timeout.tv_sec = 1;
    read_timeout.tv_usec = 0;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof(read_timeout));
    
    struct sockaddr_in saddr;
    memset(&saddr, 0x00, sizeof(struct sockaddr_in));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(HLTPORT);
    saddr.sin_addr.s_addr = INADDR_ANY;

    bind(s, reinterpret_cast<struct sockaddr*>(&saddr), sizeof(struct sockaddr));
    health_status_msg in;
    memset(&in, 0x00, sizeof(health_status_msg));

    while (true)
    {
        socklen_t len;
	    struct sockaddr_in client;
        ssize_t recv = recvfrom(s, reinterpret_cast<char*>(&in), sizeof(health_status_msg), 0, reinterpret_cast<struct sockaddr*>(&client), &len);
        bool haveHealth = recv > 0;
        bool havePhone = false;
        bool havePc = false;
	    if(haveHealth)
        {
            havePhone = in.whoami    == WHOAMI_PH;
	        havePc    = in.whoami    == WHOAMI_PC;
	        if(!haveHealth && !havePhone)
	        {
	            handle_timeout();
	        }
		else
		{
			if(back){
motorPowerOn();back=false;//motorsPowerOn();back=false;
}
}
	    }
	    else
	    {
	        handle_timeout();
	    }
    }
}
