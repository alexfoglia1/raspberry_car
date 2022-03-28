#!/bin/bash

# RUN RASPBERRY CAR EXEC
/home/pi/git/raspberry_car/build/raspberry_car

# MOTORS PIN
gpio mode 0 out
gpio mode 1 out
gpio mode 3 out
gpio mode 15 out
gpio mode 16 out

# LIGHTS PIN
gpio mode 29 out
gpio mode 21 out
gpio mode 27 out

echo "Powering off motors and lights"
# POWER OFF MOTORS AND LIGHTS
gpio write 0 0
gpio write 1 0
gpio write 3 0
gpio write 15 0
gpio write 16 0
gpio write 29 0
gpio write 21 0
gpio write 27 0
