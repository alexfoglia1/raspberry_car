#include "health.h"
#include "defs.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <string.h>

void health_task(int pid)
{
    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct sockaddr_in saddr;

    memset(&saddr, 0x00, sizeof(struct sockaddr_in));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(HLTPORT);
    saddr.sin_addr.s_addr = INADDR_ANY;

    health_status_msg in;
    memset(&in, 0x00, sizeof(health_status_msg));
    while (true)
    {
        socklen_t len;
        ssize_t recv = recvfrom(s, reinterpret_cast<char*>(&in), sizeof(health_status_msg), 0, reinterpret_cast<struct sockaddr*>(&saddr), &len);
        
        if(recv <= 0 || false == in.running) /* TODO CHECK ADDRFROM */
        {
            kill(SIGTERM, pid);
            exit(EXIT_FAILURE);
        }
    }
}
