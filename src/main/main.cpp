#include "camera.h"
#include "sensors.h"
#include "actuators.h"
#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <wait.h>
#include <wiringPi.h>
#include "motors.h"

void demo_turn(uint8_t speed, uint64_t seconds)
{
	applyMotorSpeed(speed);
	setMotorRight();
	sleep(seconds);
}

void demo_straight(uint8_t speed, uint64_t seconds)
{
	applyMotorSpeed(speed);
	setMotorForward();
	sleep(seconds);
}

void demo()
{
	printf("START DEMO\n");
	motorPowerOn();
	while(1)
	{
	    demo_straight(30, 2);
	    demo_turn(40, 1);
	    demo_straight(30, 2);
	    demo_turn(40, 1);
	    demo_straight(30, 2);
	    demo_turn(40, 1);
	    demo_straight(30, 2);
            demo_turn(40, 1);
        }
}

void on_sigterm()
{
    std::cout << "Emergency exit" << std::endl;
    motorPowerOff();
    exit(EXIT_FAILURE);
}

int main()
{
    std::cout << "Raspberry car v01.00" << std::endl;
    
    int pid = fork();
    if(pid == 0)
    {
        system("python3 /home/pi/git/imu_driver/imu_driver.py");
        exit(EXIT_SUCCESS);
    }

    pid = fork();
    if(pid == 0)
    {
        camera_task();
        exit(EXIT_SUCCESS);
    }

    pid = fork();
    if(pid == 0)
    {
        geiger_task();
        exit(EXIT_SUCCESS);
    }

    pid = fork();
    if(pid == 0)
    {
       imu_task();
       exit(EXIT_SUCCESS);
    }

    pid = fork();
    if(pid == 0)
    {
        actuators_task();
        //demo();
        exit(EXIT_SUCCESS);
    }

    std::cout << "Tasks are running . . ." << std::endl;

    int canExit;
    wait(&canExit);
    wait(&canExit);
    wait(&canExit);
    wait(&canExit);
    wait(&canExit);
}
