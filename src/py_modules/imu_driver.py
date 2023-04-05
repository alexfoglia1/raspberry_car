from mpu6050 import *
from socket import *
import time
import struct


MPU_Init()
imu_out = bytearray()

sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)
print("IMU driver running . . .")

for i in range(84):
    imu_out.append(0x00)

sock.sendto(imu_out, ("127.0.0.1",7777))

while True:
    try:
        #Read Accelerometer raw value
        acc_x = read_raw_data(ACCEL_XOUT_H)
        acc_y = read_raw_data(ACCEL_YOUT_H)
        acc_z = read_raw_data(ACCEL_ZOUT_H)
	
        #Read Gyroscope raw value
        gyro_x = read_raw_data(GYRO_XOUT_H)
        gyro_y = read_raw_data(GYRO_YOUT_H)
        gyro_z = read_raw_data(GYRO_ZOUT_H)
	
        #Full scale range +/- 250 degree/C as per sensitivity scale factor
        Ax = acc_x/16384.0
        Ay = acc_y/16384.0
        Az = acc_z/16384.0
	
        Gx = gyro_x/131.0
        Gy = gyro_y/131.0
        Gz = gyro_z/131.0

        imu_out =  struct.pack("i", 0x00) # imu_out[0:3]
        imu_out += struct.pack("d", time.time()) # imu_out[4:11]
        imu_out += struct.pack("d", Ax) # imu_out[12:19]
        imu_out += struct.pack("d", Ay) # imu_out[20:27]
        imu_out += struct.pack("d", Az) # imu_out[28:35]
        imu_out += struct.pack("d", Gx) # imu_out[36:43]
        imu_out += struct.pack("d", Gy) # imu_out[44:51]
        imu_out += struct.pack("d", Gz) # imu_out[52:59]
        imu_out += struct.pack("d", 0.0) # imu_out[60:67]
        imu_out += struct.pack("d", 0.0) # imu_out[68:75]
        imu_out += struct.pack("d", 0.0) # imu_out[76:83]

        sock.sendto(imu_out, ("127.0.0.1", 7777))
    except KeyboardInterrupt:
        break
    except Exception as e:
        print(e)

