#ifndef LIGHTS_H
#define LIGHTS_H

#define LEFT_LIGHT  29
#define RIGHT_LIGHT 21
#define CENTRAL_LIGHT 27

void init_lights();
void left_light_on();
void left_light_off();
void right_light_on();
void right_light_off();
void central_light_on();
void central_light_off();
void shutdown_lights();

void light_boot_sequence();
void light_motors_on_sequence();
void light_motors_off_sequence();
void light_imu_failure_sequence();
void light_arduino_failure_sequence();
void light_camera_failure_sequence();
void light_no_network_sequence();


#endif
