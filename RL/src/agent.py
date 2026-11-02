import socket
import logging
import json
from typing import Any, Generator
import torch
from torch.utils.tensorboard import SummaryWriter
from tqdm import tqdm
from ppo import PPO
import sys
from pathlib import Path


PORT = 8080
HOST = "127.0.0.1"
PACKET_SIZE = 65535
DEVICE = torch.accelerator.current_accelerator() if torch.accelerator.is_available() else torch.device("cpu")
LOG = logging.getLogger("agent")

WALL_PENALTY = 0.05
EXIT_REWARD = 10.0
NEW_CELL_REWARD = 0.5

NO_ACTION = (0, 0)
ACTIONS = [(0, 1), (1, 0), (0, -1), (-1, 0)] #UP, RIGHT, DOWN, LEFT
MAX_STEPS = 2000
MAX_EPISODES = 100


class UDPClient:
    def __init__(self, port: int, host: str):
        self.__port = port
        self.__host = host
        self.__socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.__running = True
        LOG.info("Client socket opened")

    def receive_observations(self) -> dict[str, Any]|None:
        try:
            data, addr = self.__socket.recvfrom(PACKET_SIZE)
            return json.loads(data.decode())
        except (OSError, ValueError) as e:
            LOG.error(f"Error receiving observations: {e}")
            return None

    def send_action(self, action: tuple[int, int], reset: bool=False) -> None:
        try:
            self.__socket.sendto(json.dumps({'action': action, 'reset': reset}).encode(), (self.__host, self.__port))
        except OSError as e:
            LOG.error(f"Error sending action: {e}")
            return

    def shutdown(self) -> None:
        try:
            self.__socket.close()
        except OSError as e:
            LOG.error(f"Error closing socket: {e}")
            return
        LOG.info("Client socket closed")

    @property
    def running(self) -> bool:
        return self.__running

    @running.setter
    def running(self, value: bool) -> None:
        self.__running = value

    def __enter__(self) -> UDPClient:
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.shutdown()
        return False

def compute_reward(sensors: list[float], cell: tuple[int, int], visited: set[tuple[int, int]], done: bool) -> tuple[float, dict[str, float]]:
    stats = {'wall_penalty': 0.0, 'new_cell_reward': 0.0, 'exit_reward': 0.0}
    if done:
        stats['exit_reward'] = EXIT_REWARD
        return EXIT_REWARD, stats

    reward = 0.0
    if min(sensors) == 0.0:
        reward -= WALL_PENALTY
        stats['wall_penalty'] -= WALL_PENALTY

    if cell not in visited:
        reward += NEW_CELL_REWARD
        stats['new_cell_reward'] = NEW_CELL_REWARD
        visited.add(cell)

    return reward, stats

def episode(client: UDPClient) -> Generator[dict[str, Any], None, None]:
    while client.running:
        obs = client.receive_observations()
        if obs is None:
            client.running = False
            return None
        if "state" not in obs or "done" not in obs or "cell" not in obs:
            LOG.warning("Invalid observation")
            continue

        yield obs

    return None


if __name__ == '__main__':
    logging.basicConfig(level=logging.DEBUG,
                        stream=sys.stdout,
                        format="%(asctime)s [%(levelname)s] %(name)s: %(message)s",
                        datefmt="%Y-%m-%d %H:%M:%S")
    LOG.info(f"Device: {DEVICE}")
    writer = SummaryWriter()
    agent = PPO(12, 4, device=DEVICE, writer=writer)
    with UDPClient(PORT, HOST) as client:
        client.send_action(NO_ACTION)
        ep = 0
        while client.running:
            step = 0
            prev_state: list[float] | None = None
            truncated = False
            stats = {'wall_penalty': 0.0, 'new_cell_reward': 0.0, 'exit_reward': 0.0}
            visited = {(0, 0)}
            try:
                with tqdm(episode(client), f"Episode {ep + 1}", unit="step", total=MAX_STEPS, leave=False, file=sys.stdout, colour="green") as pbar:

                    for obs in pbar:

                        full_state = obs['state']
                        sensors = full_state['sensors']
                        neighbors = full_state['neighbors']
                        x, y = obs['cell']
                        cell = (int(x), int(y))
                        state = sensors + neighbors

                        done = obs['done']

                        if prev_state is not None:
                            reward, details = compute_reward(sensors, cell, visited, done)
                            for k,v in details.items():
                                stats[k] += v
                        
                            truncated = not done and step >= MAX_STEPS
                            agent.record(reward, done, truncated, state)
                            pbar.set_postfix(ep_reward=sum(stats.values()))

                        if done or truncated:
                            client.send_action(NO_ACTION, True)
                            ep_reward = sum(stats.values())
                            summary = ", ".join(f"{k}={v:.2f}" for k, v in stats.items())
                            tqdm.write(f"Episode: {ep + 1} {'done' if done else 'truncated'} {summary}, ep_reward={ep_reward:.2f}")
                            writer.add_scalar("Rewards/train", ep_reward, ep, new_style=True)
                            break

                        action = agent.act(state)
                        prev_state = state
                        client.send_action(ACTIONS[action])
                        step += 1
                    ep += 1
                    if ep == MAX_EPISODES:
                        agent.save(Path("model.pth"))
                        client.running = False
            except KeyboardInterrupt:
                client.running = False
        writer.flush()
        writer.close()       