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
    detaddr.sin_addr.s_addr = inet_addr(TEGRA_ADDRESS.c_str());
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

        std::vector<int> params;
        std::vector<uint8_t> buffer;
        
        params.push_back(cv::IMWRITE_JPEG_QUALITY);
        params.push_back(40);
        cv::imencode(".jpg", frame, buffer, params);

        image_msg outmsg;
        outmsg.len = static_cast<uint16_t>(buffer.size());

        for(uint16_t i = 0; i < outmsg.len; i++)
        {
            outmsg.data[i] = buffer[i];
        }
        static int tx_to_detector = 0;
        tx_to_detector +=1;
        if (tx_to_detector == 10)
        {
        if (sendto(sock_to_detector, reinterpret_cast<char*>(&outmsg), outmsg.len, 0, reinterpret_cast<struct sockaddr*>(&detaddr), sizeof(detaddr)) > 0)
        {
            //printf("OK sendto DETECTOR\n");
        }
        else
        {
            perror("Camera task to DETECTOR\n");
        }
        tx_to_detector = 0;
        }

        if (sendto(sock, reinterpret_cast<char*>(&outmsg), sizeof(outmsg), 0, reinterpret_cast<struct sockaddr*>(&pcaddr), sizeof(pcaddr)) > 0)
        {
             //printf("Ok sendto PC\n");
        }
        //else
        //{
        //     perror("Camera task to PC");
        //}
        

    }
}
