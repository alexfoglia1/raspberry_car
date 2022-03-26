from http.server import BaseHTTPRequestHandler, HTTPServer
import time
import os

hostName = "0.0.0.0"
serverPort = 8080
html_page = [    "<!DOCTYPE html>\
                 <html lang=\"en\">\
                 <head>\
                 <meta charset=\"utf-8\"/>\
                 <title> Arduino FW Upload </title>\
                 </head>\
                 <body>\
                 <form method=\"post\" action=\".\" enctype=\"multipart/form-data\">\
                 <input type=\"file\" name=\"file\" id=\"file\">\
                 <input type=\"submit\" value=\"Upload\">\
                 </form>\
                 </body>\
                 </html>",
                 "<html> <head> <title> Arduino FW Upload </title> </head> <body> <p> Firmware uploaded </p> </body> </html>"
                 ]
class MyServer(BaseHTTPRequestHandler):
    
    def do_GET(self):
        self.send_response(200)
        self.send_header("Content-type", "text/html")
        self.end_headers()
        self.wfile.write(bytes(html_page[0], "utf-8"))
    
    def do_POST(self):
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
            
        os.system("systemctl stop raspberry_car")
        os.system("arduino-cli compile -b arduino:megaavr:nona4809 ../firmware/firmware.ino")
        os.system("arduino-cli upload -b arduino:megaavr:nona4809 -p /dev/ttyACM0 ../firmware/firmware.ino")
        os.system("systemctl restart raspberry_car")

        self.send_response(200)
        self.send_header("Content-type", "text/html")
        self.end_headers()
        self.wfile.write(bytes(html_page[1], "utf-8"))

if __name__ == "__main__":        
    webServer = HTTPServer((hostName, serverPort), MyServer)
    print("Firmware uploader started http://%s:%s" % (hostName, serverPort))

    try:
        webServer.serve_forever()
    except KeyboardInterrupt:
        pass

    webServer.server_close()
