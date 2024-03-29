#include "sensors.h"
#include "defs.h"
#include "attitude_estimator.h"
#include "lights.h"

#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/time.h>

#define M_PI 3.141592653589793
#define RAD_2_DEG(rad) (rad*180.0/M_PI)
#define DEG_2_RAD(deg) (deg*M_PI/180.0)
#define MAX_READABLE_VOLTAGE_V 10.f //todo: se questa cosa cambia rivedi il partitore di tensione perchè attualmente divide per 2 ed è bene rimanga cosi

#define WAIT_ARDUINO_SYNC    0
#define WAIT_ARDUINO_BYTE_0  1
#define WAIT_ARDUINO_BYTE_1  2
#define WAIT_ARDUINO_BYTE_2  3
#define WAIT_ARDUINO_BYTE_3  4

nano_msg_in voltage_in;
int read_voltage_status;

bool read_voltage(int fd)
{
    bool res = false;
    uint8_t byteIn;
    ssize_t lenread = read(fd, &byteIn, 1);
    if (lenread > 0)
    {
        switch (read_voltage_status)
        {
            case WAIT_ARDUINO_SYNC:
                voltage_in.sync = byteIn;
                if (voltage_in.sync == 0xff)
                {
                    read_voltage_status = WAIT_ARDUINO_BYTE_0;
                }
                break;
           case WAIT_ARDUINO_BYTE_0:
                voltage_in.voltage_data.bytes.byte_1 = byteIn;
                read_voltage_status = WAIT_ARDUINO_BYTE_1;
                break;
           case WAIT_ARDUINO_BYTE_1:
                voltage_in.voltage_data.bytes.byte_2 = byteIn;
                read_voltage_status = WAIT_ARDUINO_BYTE_2;
                break;
           case WAIT_ARDUINO_BYTE_2:
                voltage_in.voltage_data.bytes.byte_3 = byteIn;
                read_voltage_status = WAIT_ARDUINO_BYTE_3;
                break;
           case WAIT_ARDUINO_BYTE_3:
                voltage_in.voltage_data.bytes.byte_4 = byteIn;
                res = true;
                read_voltage_status = WAIT_ARDUINO_SYNC;
                break;
           default: break;
        }
    }


    return res;
}

void init_serial(int fd)
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

    if(tcsetattr(fd, TCSANOW, &serialPortSettings)!=0)
    {
        perror("tcsetattr");
    }
    tcflush(fd, TCIFLUSH);
}

void __attribute__((noreturn)) voltage_task()
{
    voltage_in.sync = 0x00;
    voltage_in.voltage_data.fdata = 0.0f;
    read_voltage_status = WAIT_ARDUINO_SYNC;

    int fd = open("/dev/ttyUSB0", O_RDONLY);
    init_serial(fd);

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct sockaddr_in daddr;
    memset(&daddr, 0x00, sizeof(struct sockaddr_in));
    daddr.sin_family = AF_INET;
    daddr.sin_addr.s_addr = inet_addr(PC_ADDRESS.c_str());
    daddr.sin_port = htons(REMOTE_PORT_DATA);

    voltage_msg voltage_out;
    memset(&voltage_out, 0x00, sizeof(voltage_msg));
    while(1)
    {
         if (read_voltage(fd))
         {
             voltage_out.msg_id = VOLTAGE_MSG_ID;
             voltage_out.motor_voltage = voltage_in.voltage_data.fdata * MAX_READABLE_VOLTAGE_V;
         
             sendto(sock, reinterpret_cast<char*>(&voltage_out), sizeof(voltage_msg), 0, reinterpret_cast<struct sockaddr*>(&daddr), sizeof(struct sockaddr));
         }
    }
}

double timestamp()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)(tv.tv_sec) + (double)(tv.tv_usec) * 1e-6;
}


void __attribute__((noreturn)) imu_task()
{
    int mpu_interface = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct sockaddr_in saddr;
    memset(&saddr, 0x00, sizeof(struct sockaddr_in));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port = htons(LOCAL_PORT_IMU);

    bind(mpu_interface, reinterpret_cast<struct sockaddr*>(&saddr), sizeof(struct sockaddr));

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct sockaddr_in daddr;
    memset(&daddr, 0x00, sizeof(struct sockaddr_in));
    daddr.sin_family = AF_INET;
    daddr.sin_addr.s_addr = inet_addr(PC_ADDRESS.c_str());
    daddr.sin_port = htons(REMOTE_PORT_DATA);

    attitude_msg att_out;
    memset(&att_out, 0x00, sizeof(attitude_msg));

    const int imu_msg_dim = 84;
    char imu_out[imu_msg_dim];
    
    stateestimation::AttitudeEstimator* Est = new stateestimation::AttitudeEstimator();
    Est->setMagCalib(0.68, -1.32, 0.0);
    Est->setPIGains(2.2, 2.65, 10, 1.25);

    double yaw = 0.0;
    double t0 = -1;
    double dt_s = 0;
	double vel_x = 0;
	double vel_y = 0;
    while (1)
    {
        memset(&imu_out, 0x00, imu_msg_dim);
        ssize_t lenread = recv(mpu_interface, &imu_out, imu_msg_dim, 0);
        if (lenread  > 0)
        {
            double msg_timestamp = *reinterpret_cast<double*>(&imu_out[4]);
            if (msg_timestamp == 0)
            {
                printf("IMU alive, resetting estimator...\n");
                yaw = 0;

                if (Est) delete Est;

                Est = new stateestimation::AttitudeEstimator();
                Est->setMagCalib(0.68, -1.32, 0.0);
                Est->setPIGains(2.2, 2.65, 10, 1.25);
            }

            double accx  = *reinterpret_cast<double*>(&imu_out[12]);
            double accy  = *reinterpret_cast<double*>(&imu_out[20]);
            double accz  = *reinterpret_cast<double*>(&imu_out[28]);
            double gyrox = DEG_2_RAD(*reinterpret_cast<double*>(&imu_out[36]));
            double gyroy = DEG_2_RAD(*reinterpret_cast<double*>(&imu_out[44]));
            double gyroz = DEG_2_RAD(*reinterpret_cast<double*>(&imu_out[52]));
            double magnx = *reinterpret_cast<double*>(&imu_out[60]);
            double magny = *reinterpret_cast<double*>(&imu_out[68]);
            double magnz = *reinterpret_cast<double*>(&imu_out[76]);
            
            //printf("acc x(%f), acc y(%f) acc z(%f)\n", accx, accy, accz);

            double act_t = timestamp();
            dt_s = t0 < 0 ? 1.0/80.0 : act_t - t0;

            Est->update(dt_s, gyrox, gyroy, gyroz, accx, accy, accz, magnx, magny, magnz);
            yaw += gyroz * dt_s;
            t0 = act_t;
            
            double q[4];
	    Est->getAttitude(q);
	    
	    att_out.header.msg_id = ATTITUDE_MSG_ID;
            att_out.yaw = yaw;
            att_out.pitch = Est->fusedPitch();
            att_out.roll = Est->fusedRoll();
            
            sendto(sock, reinterpret_cast<char*>(&att_out), sizeof(attitude_msg), 0, reinterpret_cast<struct sockaddr*>(&daddr), sizeof(struct sockaddr));


		vel_x += (accx * dt_s);
		vel_y += (accy * dt_s);
  	        //printf("x'(%f) y'(%f)\n", vel_x, vel_y);
		
        }
        else if(lenread < 0)
        {
            light_imu_failure_sequence();
            exit(EXIT_FAILURE);
        }

    }

}
