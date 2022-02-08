#include "actuators.h"
#include "motors.h"
#include "defs.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <limits>
#include <math.h>
#include <vector>
#include <queue>


static std::vector<sys_state_machine_entry_t> state_machine =
{
    /* Actual state */      /* Event */                     /* New state */         /* Action */
    {sys_state_t::idle,     sys_event_t::received_start,    sys_state_t::running,   &idle_to_running},
    {sys_state_t::idle,     sys_event_t::received_stop,     sys_state_t::idle,      nullptr},
    {sys_state_t::idle,     sys_event_t::received_joystick, sys_state_t::idle,      nullptr},

    {sys_state_t::running,  sys_event_t::received_start,    sys_state_t::running,   nullptr},
    {sys_state_t::running,  sys_event_t::received_stop,     sys_state_t::idle,      &running_to_idle},
    {sys_state_t::running,  sys_event_t::received_joystick, sys_state_t::running,   &joystick_handler}
};

static sys_state_t  system_state = sys_state_t::idle;
static joystick_msg js_state;
static std::queue<sys_event_t> event_queue;

void post_event(sys_event_t event)
{
    event_queue.push(event);
}

sys_event_t pop_event()
{
    sys_event_t event = event_queue.front();
    event_queue.pop();
    return event;
}

void idle_to_running()
{
    init_motors();
    set_motor_speed(0, 0);
}

void running_to_idle()
{
    set_motor_speed(0, 0);
    stop_motors();
}

void joystick_handler()
{
    double left_percentage = 1.0 - (double) (-std::numeric_limits<int8_t>::min() + js_state.x_axis) /
                                   (double) (std::numeric_limits<int8_t>::max() - std::numeric_limits<int8_t>::min());
    double right_percentage = 1.0 - left_percentage;

    double right_speed = std::min<double>(MAX_SPEED, js_state.throttle_state * 2 * left_percentage);
    double left_speed =  std::min<double>(MAX_SPEED, js_state.throttle_state * 2 * right_percentage);
    uint8_t ils = static_cast<uint8_t>(left_speed);
    uint8_t irs = static_cast<uint8_t>(right_speed);

    if (js_state.header.msg_id == JS_ACC_MSG_ID)
    {
        motors_forward();
    }
    else
    {
        motors_backward();
    }

    set_motor_speed(ils, irs);
}

void* listener(void*)
{
    size_t maxrecvlen = 65536;
    int serversock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_in saddr;

    memset(&saddr, 0x00, sizeof(struct sockaddr_in));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port = htons(JOYPORT);

    int res = bind(serversock, reinterpret_cast<struct sockaddr*>(&saddr), sizeof(struct sockaddr_in));
    if (!res)
    {
        perror("Bind actuator socket");
        exit(EXIT_FAILURE);
    }

    while (true)
    {
        joystick_msg js_in;
        recv(serversock, &js_in, maxrecvlen, 0);

        if (js_in.header.msg_id == JS_ACC_MSG_ID || js_in.header.msg_id == JS_BRK_MSG_ID)
        {
            sys_event_t decoded_event = js_in.start_flag && !js_in.stop_flag  ? sys_event_t::received_start :
                                        js_in.stop_flag  && !js_in.start_flag ? sys_event_t::received_stop  :
                                        sys_event_t::received_joystick;

            if (sys_event_t::received_joystick == decoded_event)
            {
                js_state = js_in;
            }
            else
            {
                js_state.start_flag = false;
                js_state.stop_flag = false;
                js_state.throttle_state = 0x00;
                js_state.x_axis = 0x00;
                js_state.y_axis = 0x00;
            }

            post_event(decoded_event);
        }
    }
}

void __attribute__((noreturn)) actuators_task(int millis)
{
    int clisock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct sockaddr_in daddr;
    
    memset(&daddr, 0x00, sizeof(struct sockaddr_in));
    daddr.sin_family = AF_INET;
    daddr.sin_addr.s_addr = inet_addr(PC_ADDRESS.c_str());
    daddr.sin_port = htons(JOYPORT);
    
    set_motor_speed(0, 0);
    memset(&js_state, 0x00, sizeof(joystick_msg));
    pthread_t thread;
    pthread_create(&thread, NULL, listener, NULL);

    while (1)
    {
        usleep(millis * 1000);
        if (!event_queue.empty())
        {
            sys_event_t next_event = pop_event();
            for(auto& entry : state_machine)
            {
                if (entry.actual_state == system_state && entry.event == next_event)
                {
                    if (entry.action != nullptr)
                    {
                        entry.action();
                    }
                    system_state = entry.next_state;

                    break;
                }
            }
        }

        actuators_state_msg state_out;
        state_out.header.msg_id = ACTUATORS_STATE_MSG_ID;
        state_out.throttle_state = js_state.throttle_state;
        state_out.system_state = int8_t(system_state);
        sendto(clisock, &state_out, sizeof(actuators_state_msg), 0, (struct sockaddr*)&daddr, sizeof(struct sockaddr_in));
    }

}
