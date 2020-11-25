#include "camera.h"
#include "sensors.h"
#include "actuators.h"
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>

int main()
{
    std::cout << "Raspberry car v01.00" << std::endl;

    int pid = fork();
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

    std::cout << "Tasks are running . . ." << std::endl;
    int canExit;
    wait(&canExit);
    wait(&canExit);
    wait(&canExit);
    wait(&canExit);

    return 0;
}
