import time
from socket import *
from threading import Thread

s = socket(AF_INET, SOCK_STREAM)
s.bind(("0.0.0.0", 7777))
s.listen(5)


def handle_client(conn_socket, conn_nr):
    print(f"Client {conn_nr} just connected.")
    msg = conn_socket.recv(64)
    time.sleep(5)
    print(msg.decode("utf-8") + f" from client {conn_nr}.")
    conn_socket.send(str(conn_nr).encode("utf-8"))
    conn_socket.close()


def main():
    i = 0
    while True:
        conn_socket, addr = s.accept()
        i = i + 1
        t = Thread(target=handle_client, args=(conn_socket, i))
        t.start()


if __name__ == "__main__":
    main()
