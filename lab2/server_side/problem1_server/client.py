from socket import *

s = socket(AF_INET, SOCK_STREAM)
s.connect(("127.0.0.1", 7777))
s.send("Read me".encode("utf-8"))
print(s.recv(64).decode("utf-8"))
s.close()
