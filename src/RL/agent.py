import socket

PORT = 8080
HOST = "127.0.0.1"

S = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)


def send(message: str) -> None:
    S.sendto(message.encode(), (HOST, PORT))


if __name__ == '__main__':
    send("Hello, from python!")
    S.close()