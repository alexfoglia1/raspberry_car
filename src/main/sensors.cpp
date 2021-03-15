#include "sensors.h"
#include "defs.h"

#include <stdio.h>
#include <errno.h>
#include <termios.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <stdbool.h>
#include <iostream>
#include <pcf8591.h>
#include <sys/time.h>
#include <math.h>


#define CPM_2_USV(cpm) (cpm/220.f)

bool strends(char* str, const char* tok, ssize_t lenstr, ssize_t lentok)
{
    for(int i = 0; i < lentok; i++)
    {
        bool equality = str[lenstr - i - 1] == tok[lentok - i - 1];
        if(!equality)
        {
            return false;
        }
    }

    return true;
}

int read_cpm(int fd)
{
    char buf[256];

    memset(buf, 0x00, 256);

    ssize_t lenread = read(fd, buf, 1);
    if(lenread == -1)
    {
        perror("Geiger task");
        exit(EXIT_FAILURE);
    }
    while(lenread >= 0 && !strends(buf, "\n\n", lenread, 2))
    {
        lenread += read(fd, buf + lenread, 1);
    }

    return atoi(buf);
}

void init_serial(int fd, uint8_t vmin)
{
    struct termios serialPortSettings;

    tcgetattr(fd, &serialPortSettings);
    cfsetispeed(&serialPortSettings, B9600);

    serialPortSettings.c_cflag &= ~PARENB;   /* Disables the Parity Enable bit(PARENB),So No Parity   */
    serialPortSettings.c_cflag &= ~CSTOPB;   /* CSTOPB = 2 Stop bits,here it is cleared so 1 Stop bit */
    serialPortSettings.c_cflag &= ~CSIZE;	 /* Clears the mask for setting the data size             */
    serialPortSettings.c_cflag |=  CS8;      /* Set the data bits = 8                                 */
    serialPortSettings.c_cflag &= ~CRTSCTS;       /* No Hardware flow Control                         */
    serialPortSettings.c_cflag |= CREAD | CLOCAL; /* Enable receiver,Ignore Modem Control lines       */
    serialPortSettings.c_iflag &= ~(IXON | IXOFF | IXANY);          /* Disable XON/XOFF flow control both i/p and o/p */
    serialPortSettings.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG);  /* Non Cannonical mode                            */
    serialPortSettings.c_oflag &= ~OPOST;/*No Output Processing*/
    serialPortSettings.c_cc[VMIN] = vmin;

    if(tcsetattr(fd, TCSANOW, &serialPortSettings)!=0)
    {
        perror("tcsetattr");
    }
    tcflush(fd, TCIFLUSH);
}

void __attribute__((noreturn)) imu_task()
{
    int melopero_interface = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct sockaddr_in saddr;
    memset(&saddr, 0x00, sizeof(struct sockaddr_in));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port = htons(IMUPORT);

    std::cout << "Port to local imu driver: " << IMUPORT << std::endl;

    bind(melopero_interface, reinterpret_cast<struct sockaddr*>(&saddr), sizeof(struct sockaddr));

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct sockaddr_in daddr;
    memset(&daddr, 0x00, sizeof(struct sockaddr_in));
    daddr.sin_family = AF_INET;
    daddr.sin_addr.s_addr = inet_addr(PC_ADDRESS);
    daddr.sin_port = htons(DATPORT);

    imu_msg imu_m1, imu_out;
    speed_msg speed_out;
    attitude_msg att_out;
    memset(&imu_out, 0x00, sizeof(imu_msg));
    memset(&speed_out, 0x00, sizeof(speed_msg));
    memset(&att_out, 0x00, sizeof(attitude_msg));

    double vx, vy, vz, yaw, pitch, roll;
    vx = vy = vz = 0.0;
    yaw = pitch = roll = 0.0;

    double t0 = -1;
    while(1)
    {
        memset(&imu_out, 0x00, sizeof(imu_out));
        ssize_t lenread;
        if((lenread = recv(melopero_interface, &imu_out, sizeof(imu_out), 0)) > 0)
        {
            if(t0 < 0)
            {
                t0 = imu_out.timestamp;
                imu_m1 = imu_out;
            }
            else
            {
                double dt_s = imu_out.timestamp - t0;
                t0 = imu_out.timestamp;
		
                imu_out.header.msg_id = IMU_MSG_ID;
                vx += dt_s * imu_m1.ax;
                vy += dt_s * imu_m1.ay;
                vz += dt_s * imu_m1.az;
                
                yaw 	+= dt_s * imu_m1.gyroz;
                pitch	+= dt_s * imu_m1.gyroy;
                roll	+= dt_s * imu_m1.gyrox;

                imu_m1 = imu_out;

                speed_out.header.msg_id = SPEED_MSG_ID;
                speed_out.vx = vx;
                speed_out.vy = vy;
                speed_out.vz = vz;

                att_out.header.msg_id = ATTITUDE_MSG_ID;
                att_out.yaw = yaw;
                att_out.pitch = pitch;
                att_out.roll = roll;

                sendto(sock, reinterpret_cast<char*>(&imu_out), sizeof(imu_msg), 0, reinterpret_cast<struct sockaddr*>(&daddr), sizeof(struct sockaddr));
                sendto(sock, reinterpret_cast<char*>(&speed_out), sizeof(speed_msg), 0, reinterpret_cast<struct sockaddr*>(&daddr), sizeof(struct sockaddr));
                sendto(sock, reinterpret_cast<char*>(&att_out), sizeof(attitude_msg), 0, reinterpret_cast<struct sockaddr*>(&daddr), sizeof(struct sockaddr));
            }
        }
        else if(lenread < 0)
        {
            perror("IMU task");
            exit(EXIT_FAILURE);
        }

    };

}
void __attribute__((noreturn)) geiger_task()
{
    int fd = open("/dev/ttyUSB0", O_RDONLY);
    init_serial(fd, 3);

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct sockaddr_in daddr;
    memset(&daddr, 0x00, sizeof(struct sockaddr_in));
    daddr.sin_family = AF_INET;
    daddr.sin_addr.s_addr = inet_addr(PC_ADDRESS);
    daddr.sin_port = htons(DATPORT);

    while(1)
    {
        int cpm = static_cast<int>(read_cpm(fd));

        radiation_msg out;
        out.header.msg_id = RADIATION_MSG_ID;
        out.CPM = cpm;
        out.uSv_h = CPM_2_USV(out.CPM);

        if (sendto(sock, &out, sizeof(radiation_msg), 0, reinterpret_cast<struct sockaddr*>(&daddr), sizeof(struct sockaddr_in)) > 0)
        {
            std::cout << "Geiger out: Success" << std::endl;
        }

    }
}
