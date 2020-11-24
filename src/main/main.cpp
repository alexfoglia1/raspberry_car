#include "camera.h"
#include "sensors.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>

int main()
{
    printf("Raspberry Car\n");

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
    }

    printf("Tasks are running . . .\n");
    int canExit;
    wait(&canExit);
    wait(&canExit);
    wait(&canExit);

    return 0;
}
