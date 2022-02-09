#include "camera.h"
#include "sensors.h"
#include "actuators.h"
#include "motors.h"
#include "defs.h"

#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <wait.h>
#include <wiringPi.h>
#include <stdio.h>
#include <fstream>

std::string PC_ADDRESS("192.168.1.19");
std::string TEGRA_ADDRESS("192.168.1.51");


void on_sigterm(int pid)
{
    __UNUSED__(&pid);

    std::cout << "Emergency exit" << std::endl;
    stop_motors();
    kill(0, SIGKILL);
}


int main(int argc, char** argv)
{
    std::cout << PROJNAME << std::endl;

    if (argc >= 3)
    {
        PC_ADDRESS = argv[1];
        TEGRA_ADDRESS = argv[2];
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
        if(line[1].length() > 0)
        {
            TEGRA_ADDRESS = line[1];
        }

        std::cout << "PC ADDRESS(" << PC_ADDRESS << ")" << std::endl;
        std::cout << "TEGRA ADDRESS(" << TEGRA_ADDRESS << ")" << std::endl;
    }

    signal(SIGTERM, on_sigterm);

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
        voltage_task();
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
        actuators_task(10);
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
