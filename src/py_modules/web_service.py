from http.server import BaseHTTPRequestHandler, HTTPServer
import time
import os

hostName = "0.0.0.0"
serverPort = 8080
html_page = [    "<!DOCTYPE html>\
                 <html lang=\"en\">\
                 <head>\
                 <meta charset=\"utf-8\"/>\
                 <title> Raspberry Car Web Service </title>\
                 </head>\
                 <body>\
                 <div>\
                 <p> Arduino Firmware upload </p>\
                 <form method=\"post\" action=\"/firmware\" enctype=\"multipart/form-data\">\
                 <input type=\"file\" name=\"file\" id=\"file\">\
                 <input type=\"submit\" value=\"Upload\">\
                 </form>\
                 </div>\
                 <div>\
                 <p> Settings </p>\
                 <form method=\"post\" action=\"/settings\">\
                 <input type=\"text\" id =\"addr\" name=\"addr\">\
                 <input type=\"submit\" value=\"Set controller address\">\
                 </form>\
                 </div>\
                 <div>\
                 <p> IMU </p>\
                 <form method=\"post\" action=\"/imu\" >\
                 <input type=\"submit\" value=\"Recalibrate inertial unit\">\
                 </form>\
                 </div>\
                 </body>\
                 </html>",
                 "<html> <head> <title> Raspberry Car Web Service </title> </head> <body> <p> Firmware uploaded </p> </body> </html>",
                 "<html> <head> <title> Raspberry Car Web Service </title> </head> <body> <p> Controller address updated </p> </body> </html>",
                 "<html> <head> <title> Raspberry Car Web Service </title> </head> <body> <p> IMU Recalibrated </p> </body> </html>",
                 "<html> <head> <title> Raspberry Car Web Service </title> </head> <body> <p> Cannot serve request </p> </body> </html>"
                 ]

class MyServer(BaseHTTPRequestHandler):
    
    def do_GET(self):
        self.send_response(200)
        self.send_header("Content-type", "text/html")
        self.end_headers()
        self.wfile.write(bytes(html_page[0], "utf-8"))
    
    def do_POST(self):
        
        if self.path == '/firmware':
            content_len = int(self.headers.get('Content-Length'))
            content = self.rfile.read(content_len)
            data_begin = content.index(b'\r\n\r\n')
            content = content[data_begin + 4:]
            content_tmp = content
            data_end = 0
            while b'\r\n' in content_tmp:
                data_end = content_tmp.index(b'\r\n')
                content_tmp = content_tmp[:data_end]
            content = content_tmp
            
            with open("../firmware/firmware.ino", "wb") as f:
                f.write(content)
            
            os.system("sudo systemctl stop raspberry-car")
            os.system("arduino-cli compile --upload ../firmware/firmware.ino --port /dev/ttyACM0 --fqbn arduino:megaavr:nona4809")

            self.send_response(200)
            self.send_header("Content-type", "text/html")
            self.end_headers()
            self.wfile.write(bytes(html_page[1], "utf-8"))
        
            os.system("sudo reboot now")
        elif self.path == '/settings':
            content_len = int(self.headers.get('Content-Length'))
            content = self.rfile.read(content_len)
            content = str(content)
            csplit = content.split('=')
            if len(csplit) > 1:
                content = csplit[1].replace("'", "")
                if len(content.split(".")) != 4:
                    content = "192.168.1.1"
            else:
                content = "192.168.1.1"
            
            with open("../../build/settings.ini", "wb") as f:
                f.write(content.encode("utf-8"))
            
            os.system("sudo systemctl restart raspberry-car")
            
            self.send_response(200)
            self.send_header("Content-type", "text/html")
            self.end_headers()
            self.wfile.write(bytes(html_page[2], "utf-8"))
        elif self.path == '/imu':
            os.system("sudo systemctl restart imu-service")
            
            self.send_response(200)
            self.send_header("Content-type", "text/html")
            self.end_headers()
            self.wfile.write(bytes(html_page[3], "utf-8"))
        else:
            self.send_response(300)
            self.send_header("Content-type", "text/html")
            self.end_headers()
            self.wfile.write(bytes(html_page[4], "utf-8"))
            
            
if __name__ == "__main__":        
    webServer = HTTPServer((hostName, serverPort), MyServer)
    print("Raspberry Car Web Service started http://%s:%s" % (hostName, serverPort))

    try:
        webServer.serve_forever()
    except KeyboardInterrupt:
        pass

    webServer.server_close()
