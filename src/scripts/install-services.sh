cp /home/pi/git/raspberry_car/src/scripts/web-service.service /etc/systemd/system
systemctl daemon-reload 
systemctl enable web-service

cp /home/pi/git/raspberry_car/src/scripts/imu-service.service /etc/systemd/system
systemctl daemon-reload
systemctl enable imu-service

cp /home/pi/git/raspberry_car/src/scripts/raspberry-car.service /etc/systemd/system
systemctl daemon-reload
systemctl enable raspberry-car
