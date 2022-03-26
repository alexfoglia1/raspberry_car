import melopero_lsm9ds1 as mp
import time
import struct

from socket import *

g = 9.81

def vector3_sub(x, y):
    return [x[0] - y[0], x[1] - y[1], x[2] - y[2]]

def vector3_sum(x, y):
    return [x[0] + y[0], x[1] + y[1], x[2] + y[2]]

def vector3_scal_mult(x, alpha):
    return [x[0]*alpha, x[1]*alpha, x[2]*alpha]

def vector3_round(x, n_digit):
    return [round(x[0], n_digit), round(x[1], n_digit), round(x[2], n_digit)]

def get_acc_g(sensor):
    acc = sensor.get_acc()
    return [acc[0]*g, acc[1]*g, acc[2]*g]


def read_imu(sensor, n, delay):
    gyro = [0, 0, 0]
    acc = [0, 0, 0]
    magn = [0, 0, 0]

    for i in range(n):
        gyro = vector3_sum(gyro, sensor.get_gyro())
        acc = vector3_sum(acc, sensor.get_acc())
        magn = vector3_sum(magn, sensor.get_mag())
        time.sleep(delay)

    gyro = vector3_scal_mult(gyro, 1.0/n)
    acc = vector3_scal_mult(acc, 1.0/n)
    magn = vector3_scal_mult(magn, 1.0/n)
    gyro = [gyro[1], gyro[0], gyro[2]]
    acc = [acc[1], acc[0], acc[2]]
    magn = [magn[1], magn[0], magn[2]]

    return gyro, acc, magn


imu_out = bytearray()

sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)
sensor = mp.LSM9DS1()
sensor.use_i2c()

sensor.set_gyro_odr(mp.LSM9DS1.ODR_119Hz)
sensor.set_acc_odr(mp.LSM9DS1.ODR_119Hz)
sensor.set_mag_odr(mp.LSM9DS1.MAG_ODR_80Hz)

print("Estimating IMU bias . . .")
gyro, acc, _ = read_imu(sensor, 100, 1.0/80.0)
gyro_bias_x = round(gyro[0], 6)
gyro_bias_y = round(gyro[1], 6)
gyro_bias_z = round(gyro[2], 6)
acc_bias_x = round(g*acc[0], 6)
acc_bias_y = round(g*acc[1], 6)
acc_bias_z = round(g - g*acc[2], 6)
print("Gyroscope bias: x({}), y({}), z({})".format(gyro_bias_x, gyro_bias_y, gyro_bias_z))
print("Accelerometer bias: x({}), y({}), z({})".format(acc_bias_x, acc_bias_y, acc_bias_z))
print("IMU driver running . . .")

for i in range(84):
    imu_out.append(0x00)

sock.sendto(imu_out, ("127.0.0.1",7777))

while True:
    try:
        gyro, acc, magn = read_imu(sensor, 1, 1.0/80.0)

        acc_measurements =  vector3_round([acc[0]*g,acc[1]*g,acc[2]*g], 6)
        gyro_measurements = vector3_round(gyro, 6)
        magn_measurements = vector3_round(magn, 6)

        imu_out =  struct.pack("i", 0x00) # imu_out[0:3]
        imu_out += struct.pack("d", time.time()) # imu_out[4:11]
        imu_out += struct.pack("d", acc_measurements[0] - acc_bias_x) # imu_out[12:19]
        imu_out += struct.pack("d", acc_measurements[1] - acc_bias_y) # imu_out[20:27]
        imu_out += struct.pack("d", acc_measurements[2] - acc_bias_z) # imu_out[28:35]
        imu_out += struct.pack("d", gyro_measurements[0] - gyro_bias_x) # imu_out[36:43]
        imu_out += struct.pack("d", gyro_measurements[1] - gyro_bias_y) # imu_out[44:51]
        imu_out += struct.pack("d", gyro_measurements[2] - gyro_bias_z) # imu_out[52:59]
        imu_out += struct.pack("d", magn_measurements[0]) # imu_out[60:67]
        imu_out += struct.pack("d", magn_measurements[1]) # imu_out[68:75]
        imu_out += struct.pack("d", magn_measurements[2]) # imu_out[76:83]

        sock.sendto(imu_out, ("127.0.0.1", 7777))
    except KeyboardInterrupt:
        break
    except Exception as e:
        print(e)


#Close the device correctly!
sensor.close()
