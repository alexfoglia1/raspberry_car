#include "camera.h"
#include "sensors.h"
#include "actuators.h"
#include "motors.h"
#include "defs.h"
#include "health.h"

#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <wait.h>
#include <wiringPi.h>
#include <stdio.h>
#include <fstream>

std::string PC_ADDRESS("192.168.1.15");
std::string PH_ADDRESS("192.168.1.52");

void on_sigterm(int pid)
{
    std::cout << "Emergency exit" << std::endl;
    motorPowerOff();
    kill(0, SIGKILL);
}


int main(int argc, char** argv)
{
    std::cout << "Raspberry car v01.00" << std::endl;

    if (argc > 2)
    {
        PC_ADDRESS = argv[1];
        PH_ADDRESS = argv[2];
    }
    else
    {
        std::string line[2];
        std::ifstream rfile;
        rfile.open("settings.ini");
        if (rfile.is_open())
        {
            int i = 0;
            while (std::getline(rfile, line[i]) && i < 1)
            {
                i++;
            }
            rfile.close();
        }
        if(line[0].length() > 0)
        {
                PC_ADDRESS = line[0];
        }
        std::cout << "PC ADDRESS(" << PC_ADDRESS << ")" << std::endl;

        if(line[1].length() > 0)
        {
            PH_ADDRESS = line[1];
        }
        std::cout << "PHONE ADDRESS(" << PH_ADDRESS << ")" << std::endl;

    }

    signal(SIGTERM, on_sigterm);

    int fpid = getpid();
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
        exit(EXIT_SUCCESS);
    }

    pid = fork();
    if(pid == 0)
    {
        //health_task(fpid);
        exit(EXIT_SUCCESS);
    }

    std::cout << "Tasks are running . . ." << std::endl;

    int canExit;
    wait(&canExit);
    wait(&canExit);
    wait(&canExit);
    wait(&canExit);
    wait(&canExit);
    wait(&canExit);
}
