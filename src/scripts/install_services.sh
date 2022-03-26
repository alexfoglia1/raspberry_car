cp /home/pi/git/raspberry_car/src/scripts/web_service.service /etc/systemd/system
systemctl daemon-reload 
systemctl enable web_service

cp /home/pi/git/raspberry_car/src/scripts/imu_service.service /etc/systemd/system
systemctl daemon-reload
systemctl enable imu_service

cp /home/pi/git/raspberry_car/src/scripts/raspberry_car.service /etc/systemd/system
systemctl daemon-reload
systemctl enable raspberry_car
