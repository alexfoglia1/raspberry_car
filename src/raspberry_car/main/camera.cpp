/*
 * Name: get_video_pixel
 * Description: Take a snapshot of a video and get the RGB value
 *	of any pixel in the snapshot.
 * Author: Najam Syed (github.com/nrsyed)
 * Created: 2018-Feb-12
 */

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <stdio.h>
#include <unistd.h>

#include "defs.h"
#include "lights.h"

void __attribute__((noreturn)) camera_task()
{
    cv::VideoCapture capture(0);
    if (!capture.isOpened())
    {
        light_camera_failure_sequence();
        exit(EXIT_FAILURE);
    }

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_in pcaddr;

    memset(&pcaddr, 0x00, sizeof(struct sockaddr_in));

    pcaddr.sin_family = AF_INET;
    pcaddr.sin_addr.s_addr = inet_addr(PC_ADDRESS.c_str());
    pcaddr.sin_port = htons(REMOTE_PORT_VIDEO_PC);

    cv::Mat frame;
    while (1)
    {
        if (!capture.read(frame))
        {
            light_camera_failure_sequence();
            exit(EXIT_FAILURE);
        }

        cv::flip(frame, frame, -1);
        cv::resize(frame, frame, cv::Size(IMAGE_COLS, IMAGE_ROWS));

        std::vector<int> params;
        std::vector<uint8_t> buffer;
        
        params.push_back(cv::IMWRITE_JPEG_QUALITY);
        params.push_back(60);
        cv::imencode(".jpg", frame, buffer, params);

        image_msg outmsg;
        outmsg.len = static_cast<uint16_t>(buffer.size());

        for (uint16_t i = 0; i < outmsg.len; i++)
        {
            outmsg.data[i] = buffer[i];
        }

        if (sendto(sock, reinterpret_cast<char*>(&outmsg), sizeof(outmsg), 0, reinterpret_cast<struct sockaddr*>(&pcaddr), sizeof(pcaddr)) <= 0)
        {
            light_camera_failure_sequence();
        }
    }
}
