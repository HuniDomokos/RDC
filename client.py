import socket
import sys

s=socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
msg = "Hello, Server!"
s.sendto(str.encode(msg),(sys.argv[1], int(sys.argv[2])))
msg,addr = s.recvfrom(1024)
print("Received message: ", msg.decode(), "from", addr)