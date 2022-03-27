#include "web_service.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

enum session_state
{
    WAIT_FILE,
    FILE_RECEIVED,
    SESSION_ENUM_SIZE
};

void service_task()
{
    const char* HTTP_RESP_HEADER_FORMAT =  "HTTP/1.1 200 OK\r\n"
                                           "Content-Length: %d\r\n"         // <--- qui va la dimensione della pagina
                                           "Content-Type: text/html\r\n\r\n"
                                           "%s";                            // <--- qui va la pagina

    const char* html_page[SESSION_ENUM_SIZE] =
            {"<html>"
            "<head>"
            "<script type=\"text/javascript\">"
            ""
            "function Upload()"
            "{"
            ""
            "var filename = document.getElementById(\"filename\").value;"
            ""
            "var storepath = \"HOSTURL/Foldername\";"
            ""
            "}"
            "</script>"
            "</head>"
            "<body>"
            "<form action=\"\" method=\"post\" enctype=\"multipart/form-data\" >"
            "    <input type=\"file\" name=\"filename\" />"
            "    <input type=\"submit\" value=\"Upload\" onclick=\"Upload\" />"
            "</form>"
            "</body>"
            "</html>",
            "<html> File received </html>"};

    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in saddr;
    memset(&saddr, 0x00, sizeof(struct sockaddr_in));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port = htons(8080);

    bind(s, (struct sockaddr*) &saddr, sizeof(struct sockaddr_in));
    listen(s, 50);

    session_state state = WAIT_FILE;
    char out_buf[4096];
    while (1)
    {
        unsigned int len;
        int clisock = accept(s, (struct sockaddr*)&saddr, &len);

        switch (state)
        {
            case WAIT_FILE:
            {
                char buf[1024];
                recv(clisock, buf, 1024, 0);

                size_t html_page_len = strlen(html_page[WAIT_FILE]);
                sprintf(out_buf, HTTP_RESP_HEADER_FORMAT, html_page_len, html_page[WAIT_FILE]);
                send(clisock, out_buf, strlen(out_buf), 0);



                char filebuf[6553500];
                recv(clisock, filebuf, 6553500, 0);

                for (int i = 0; i < 2048; i++)
                {
                    printf("byte[%d]: 0x%X\n", i, filebuf[i]);
                }

                state = FILE_RECEIVED;
                break;
            }
            case FILE_RECEIVED:
            {
                size_t html_page_len = strlen(html_page[FILE_RECEIVED]);
                sprintf(out_buf, HTTP_RESP_HEADER_FORMAT, html_page_len, html_page[FILE_RECEIVED]);
                send(clisock, out_buf, strlen(out_buf), 0);
                state = WAIT_FILE;
                break;
            }
            default: break;

        }
    }

}
