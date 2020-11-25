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

#define COLOR_ROWS 80
#define COLOR_COLS 250
#define MAX_IMAGESIZE 40090

typedef struct
{
    uint16_t len;
    uint8_t data[MAX_IMAGESIZE];
} image_msg;


void __attribute__((noreturn)) camera_task()
{
	cv::VideoCapture capture(0);
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct sockaddr_in daddr, vaddr;
    memset(&daddr, 0x00, sizeof(struct sockaddr_in));
    memset(&vaddr, 0x00, sizeof(struct sockaddr_in));

    vaddr.sin_family = AF_INET;
    vaddr.sin_addr.s_addr = inet_addr("192.168.1.14");
    vaddr.sin_port = htons(4321);
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

		std::vector<int> params;
        std::vector<uint8_t> buffer;

        params.push_back(cv::IMWRITE_JPEG_QUALITY);
        params.push_back(60);
        cv::imencode(".jpg", frame, buffer, params);
        
        image_msg outmsg;
        outmsg.len = static_cast<uint16_t>(buffer.size());
        for(uint16_t i = 0; i < outmsg.len; i++)
        {
            outmsg.data[i] = buffer[i];
        }
        if(sendto(sock, reinterpret_cast<char*>(&outmsg), sizeof(outmsg), 0, reinterpret_cast<struct sockaddr*>(&vaddr), sizeof(vaddr)) > 0)
        {
            std::cout << "Video out: Success" << std::endl;
        }

	}
}
