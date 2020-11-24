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
    int fd = open("/dev/ttyACM0", O_RDONLY | O_NOCTTY);
    init_serial(fd, sizeof(arduino_out));

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct sockaddr_in daddr;
    memset(&daddr, 0x00, sizeof(struct sockaddr_in));
    daddr.sin_family = AF_INET;
    daddr.sin_addr.s_addr = inet_addr("192.168.1.14");
    daddr.sin_port = htons(DATPORT);

    imu_msg imu_out;
    speed_msg speed_out;
    attitude_msg att_out;
    memset(&imu_out, 0x00, sizeof(imu_msg));
    memset(&speed_out, 0x00, sizeof(speed_msg));
    memset(&att_out, 0x00, sizeof(attitude_msg));
    while(1)
    {
        arduino_out from_arduino;
        memset(&from_arduino, 0x00, sizeof(from_arduino));
        if(read(fd, &from_arduino, sizeof(from_arduino)) == sizeof(from_arduino))
        {
            imu_out.header.msg_id = IMU_MSG_ID;
            imu_out.ax = from_arduino.accelerationX;
            imu_out.ay = from_arduino.accelerationY;
            imu_out.az = from_arduino.accelerationZ;
            imu_out.gyrox = from_arduino.angularSpeedX;
            imu_out.gyroy = from_arduino.angularSpeedY;
            imu_out.magnx = imu_out.magny = imu_out.magnz = 0.0;

            speed_out.header.msg_id = SPEED_MSG_ID;
            speed_out.vx = from_arduino.speedX;
            speed_out.vy = from_arduino.speedY;
            speed_out.vz = from_arduino.speedZ;

            att_out.header.msg_id = ATTITUDE_MSG_ID;
            att_out.yaw = 0;
            att_out.pitch = from_arduino.pitchAngle;
            att_out.roll = from_arduino.rollAngle;

            sendto(sock, reinterpret_cast<char*>(&imu_out), sizeof(imu_msg), 0, reinterpret_cast<struct sockaddr*>(&daddr), sizeof(struct sockaddr));
            perror("Imu out");
            sendto(sock, reinterpret_cast<char*>(&speed_out), sizeof(speed_msg), 0, reinterpret_cast<struct sockaddr*>(&daddr), sizeof(struct sockaddr));
            perror("Speed out");
            sendto(sock, reinterpret_cast<char*>(&att_out), sizeof(attitude_msg), 0, reinterpret_cast<struct sockaddr*>(&daddr), sizeof(struct sockaddr));
            perror("Attitude out");
            std::cout << "Angular speed: " << imu_out.gyrox << "\t" << imu_out.gyroy << std::endl;
            std::cout << "Acc: " << imu_out.ax << "\t" << imu_out.ay << "\t" << imu_out.az << std::endl;
            std::cout << "Speed: " << speed_out.vx << "\t" << speed_out.vy << "\t" << speed_out.vz << std::endl;
            std::cout << "Attitude: " << att_out.roll << "\t" << att_out.pitch << std::endl << std::endl;
        }

        tcflush(fd, TCIFLUSH);
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
    daddr.sin_addr.s_addr = inet_addr("192.168.1.14");
    daddr.sin_port = htons(DATPORT);

    while(1)
    {
        int cpm = static_cast<int>(read_cpm(fd));

        radiation_msg out;
        out.header.msg_id = RADIATION_MSG_ID;
        out.CPM = cpm;
        out.uSv_h = CPM_2_USV(out.CPM);

        sendto(sock, &out, sizeof(radiation_msg), 0, reinterpret_cast<struct sockaddr*>(&daddr), sizeof(struct sockaddr_in));
        perror("Geiger out");
    }
}
