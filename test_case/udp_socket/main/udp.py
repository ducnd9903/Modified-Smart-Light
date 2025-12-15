# # ip là client
import socket

UDP_IP = "192.168.1.18"  # IP của ESP32-C3
UDP_PORT = 3333
MESSAGE = "Turn on light"

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.sendto(MESSAGE.encode(), (UDP_IP, UDP_PORT))
print("Sent:", MESSAGE)


# ip là server
# import socket

# sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
# sock.bind(("0.0.0.0", 3333))
# print("Listening on UDP port 3333...")

# while True:
#     data, addr = sock.recvfrom(1024)
#     print("Received:", data.decode(), "from", addr)

