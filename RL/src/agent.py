import socket
import logging
import sys
from enum import StrEnum
import json
from typing import Any


PORT = 8080
HOST = "127.0.0.1"
PACKET_SIZE = 65535

logger = logging.getLogger("agent")


class UDPClient:
    def __init__(self, port: int, host: str):
        self._port = port
        self._host = host
        self._socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self._running = True
        logger.info("Client connection opened")

    def receive_observations(self) -> dict[str, Any]|None:
        try:
            data, addr = self._socket.recvfrom(PACKET_SIZE)
            return json.loads(data.decode())
        except (OSError, ValueError) as e:
            logger.error(f"Error receiving observations: {e}")
            return None

    def send_action(self) -> None:
        try:
            self._socket.sendto(json.dumps({'action': 1}).encode(), (self._host, self._port))
        except OSError as e:
            logger.error(f"Error sending action: {e}")
            return

    def shutdown(self) -> None:
        try:
            self._socket.close()
        except OSError as e:
            logger.error(f"Error closing socket: {e}")
            return
        logger.info("Client connection closed")

    @property
    def running(self) -> bool:
        return self._running

    @running.setter
    def running(self, value: bool) -> None:
        self._running = value

    def __enter__(self) -> UDPClient:
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.shutdown()
        return False


if __name__ == '__main__':
    logging.basicConfig(level=logging.DEBUG,
                        stream=sys.stdout,
                        format="%(asctime)s [%(levelname)s] %(name)s: %(message)s",
                        datefmt="%Y-%m-%d %H:%M:%S")
    with UDPClient(PORT, HOST) as client:
        while client.running:
            try:
                client.send_action()
                observations = client.receive_observations()
                if observations is None:
                    break
                logger.debug(observations)
            except KeyboardInterrupt:
                client.running = False

