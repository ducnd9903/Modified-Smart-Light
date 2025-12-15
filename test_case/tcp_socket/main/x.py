# # ip là server
import socket

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind(('0.0.0.0', 3333))
server.listen(1)
print("Server listening on port 3333...")

conn, addr = server.accept()
print("Connected by", addr)
data = conn.recv(1024)
print("Received:", data.decode())
conn.close()


# # ip là client
# import socket
# ESP_IP = "192.168.1.18"
# PORT = 3333
# s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
# s.connect((ESP_IP, PORT))
# print("Connected to ESP32 TCP server")
# while True:
#     msg = input("Enter message: ")
#     s.send(msg.encode())
#     data = s.recv(1024)
#     print("Echo from ESP:", data.decode())


