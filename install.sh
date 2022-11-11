#!/bin/bash

echo "**** INSTALLING RASPBERRY CAR ****"
echo ""
echo "Compiling..."
echo ""

builddir=$PWD/build
if test -d "$builddir"; then
    rm -fr "$builddir"
fi

mkdir build
cd build
cmake ..
make all
echo "192.168.1.4" > settings.ini
cd ../src/scripts/

echo ""
echo "Installing services..."
echo ""

webserv=/etc/systemd/system/web-service.service
imuserv=/etc/systemd/system/imu-service.service
carserv=/etc/systemd/system/raspberry-car.service

if test -f "$webserv"; then
    sudo systemctl stop web-service
    echo "$webserv exists: removing old web service installation"
    sudo systemctl disable web-service
    sudo rm -fr "$webserv"
    sudo systemctl daemon-reload
fi

if test -f "$imuserv"; then
    sudo systemctl stop imu-service
    echo "$imuserv exists: removing old imu service installation"
    sudo systemctl disable imu-service
    sudo rm -fr "$imuserv"
    sudo systemctl daemon-reload
fi


if test -f "$carserv"; then
    sudo systemctl stop raspberry-car
    echo "$carserv exists: removing old raspberry-car service installation"
    sudo systemctl disable raspberry-car
    sudo rm -fr "$carserv"
    sudo systemctl daemon-reload
fi

sudo sh install-services.sh

cd ../../

echo "**** DONE ****"
echo ""
echo ""
echo "You need to know the controller address."
echo "Type it inside build/settings.ini"
echo "Or change it via the web service."
echo ""
echo ""
echo "To run the application, type:"
echo "sudo systemctl start imu-service"
echo "sudo systemctl start web-service"
echo "sudo systemctl start raspberry-car"
echo "or reboot the system."
