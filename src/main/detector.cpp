#include "detector.h"
#include "defs.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>

cv::CascadeClassifier* fClassifier;
cv::CascadeClassifier* cClassifier;
cv::CascadeClassifier* pClassifier;

void to_target_data(target_data* tracks, std::vector<cv::Rect>& objects, const char* desc)
{
    for(uint32_t i = 0; i < objects.size(); i++)
    {
        tracks[i].x_pos = objects.at(i).x;
        tracks[i].y_pos = objects.at(i).y;
        tracks[i].width = objects.at(i).width;
        tracks[i].height = objects.at(i).height;
        sprintf(tracks[i].description, "%s", desc);
    }
}


target_msg search_targets(cv::Mat &frame)
{
    std::vector<cv::Rect>  tFaces;
    std::vector<cv::Rect>  tObjects;
    std::vector<cv::Rect>  tPeople;

    fClassifier->detectMultiScale(frame, tFaces);
    cClassifier->detectMultiScale(frame, tObjects);
    pClassifier->detectMultiScale(frame, tPeople);
    int nFaces = tFaces.size();
    int nObjects = tObjects.size();
    int nPeople = tPeople.size();
    //printf("found %d faces, %d obstacles, %d people\n", nFaces, nObjects, nPeople);
    
    int nTargets = nFaces + nObjects + nPeople;

    target_msg tgt;
    memset(&tgt, 0x00, sizeof(target_msg));
    tgt.header.msg_id = TARGET_MSG_ID;
    tgt.n_targets = std::min<uint8_t>(MAX_TARGETS, nTargets);

    if (nFaces <= MAX_TARGETS)
        to_target_data(&tgt.data[0], tFaces, "HUMAN FACE");
    if (nFaces + nObjects <= MAX_TARGETS)
        to_target_data(&tgt.data[nFaces], tObjects, "OBSTACLE");
    if (nTargets <= MAX_TARGETS)
        to_target_data(&tgt.data[nFaces + nObjects], tPeople,"PEDESTRIAN");

    return tgt;
}


void __attribute__((noreturn)) detect_task()
{
    fClassifier = new cv::CascadeClassifier("data/haarcascade_default.xml");
    cClassifier = new cv::CascadeClassifier("data/haarcascade_car.xml");
    pClassifier = new cv::CascadeClassifier("data/haarcascade_fullbody.xml");

    int clisock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    int txsock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_in saddr, txaddr;
    memset(&saddr, 0x00, sizeof(struct sockaddr_in));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(DETPORT);
    saddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(clisock, (struct sockaddr*)&saddr, sizeof(struct sockaddr_in)) < 0)
    {
        perror("Cannot bind detector socket\n");
        exit(EXIT_FAILURE);
    }

    txaddr.sin_family = AF_INET;
    txaddr.sin_addr.s_addr = inet_addr(PC_ADDRESS.c_str());
    txaddr.sin_port = htons(TGTPORT);

    while (1)
    {
        char buffer[MAX_IMAGESIZE];
        recv(clisock, buffer, MAX_IMAGESIZE, 0);
        image_msg rx = *reinterpret_cast<image_msg*>(buffer);
        cv::Mat matrx;
        memset(matrx.data, 0x00 , matrx.dataend - matrx.data);
        std::vector<char> data(rx.data, rx.data + rx.len);
        matrx = cv::imdecode(cv::Mat(data), 1);

        target_msg e = search_targets(matrx);

        sendto(txsock, reinterpret_cast<char*>(&e), sizeof(target_msg), 0, reinterpret_cast<struct sockaddr*>(&txaddr), sizeof(txaddr));

    }


}
