from socket import *

s = socket(AF_INET, SOCK_STREAM)
s.bind(("0.0.0.0", 7777))
s.listen(5)

cs, addr = s.accept()
b = cs.recv(10)
print(b.decode("utf-8"))
cs.send("Received".encode("utf-8"))
cs.close()
