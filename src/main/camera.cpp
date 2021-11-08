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

void __attribute__((noreturn)) camera_task()
{
	
    cv::VideoCapture capture(0);
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    int sock_to_detector = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_in pcaddr, detaddr;

    memset(&pcaddr, 0x00, sizeof(struct sockaddr_in));
    memset(&detaddr, 0x00, sizeof(struct sockaddr_in));

    pcaddr.sin_family = AF_INET;
    pcaddr.sin_addr.s_addr = inet_addr(PC_ADDRESS.c_str());
    pcaddr.sin_port = htons(RENPORT);
    
    detaddr.sin_family = AF_INET;
    detaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    detaddr.sin_port = htons(DETPORT);


    if (!capture.isOpened())
    {
	
        exit(EXIT_FAILURE);
    }
	

    cv::Mat frame;
    while (1)
    {
        if (!capture.read(frame))
        {
		
            exit(EXIT_FAILURE);
        }

        cv::flip(frame, frame, -1);
        cv::resize(frame, frame, cv::Size(IMAGE_COLS, IMAGE_ROWS));

        cv::Mat gray;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

        std::vector<int> params;
        std::vector<uint8_t> buffer;
        std::vector<uint8_t> detbuffer;
        params.push_back(cv::IMWRITE_JPEG_QUALITY);
        params.push_back(60);
        cv::imencode(".jpg", frame, buffer, params);
       cv::imencode(".jpg", gray, detbuffer, params);

        image_msg outmsg, detmsg;
        outmsg.len = static_cast<uint16_t>(buffer.size());
        detmsg.len = static_cast<uint16_t>(detbuffer.size());

        for(uint16_t i = 0; i < outmsg.len; i++)
        {
            outmsg.data[i] = buffer[i];
        }

        for(uint16_t i = 0; i < detmsg.len; i++)
        {
            detmsg.data[i] = detbuffer[i];
        }

        if (sendto(sock_to_detector, reinterpret_cast<char*>(&detmsg), detmsg.len, 0, reinterpret_cast<struct sockaddr*>(&detaddr), sizeof(detaddr)) > 0)
        {
            //printf("OK sendto DETECTOR\n");
        }
        //else
        //{
        //    perror("Camera task to DETECTOR\n");
        //}

        if (sendto(sock, reinterpret_cast<char*>(&outmsg), sizeof(outmsg), 0, reinterpret_cast<struct sockaddr*>(&pcaddr), sizeof(pcaddr)) > 0)
        {
             //printf("Ok sendto PC\n");
        }
        else
        {
             perror("Camera task to PC");
        }
        

    }
}
