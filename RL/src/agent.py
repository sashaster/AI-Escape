import socket
import logging
import json
import math
from typing import Any, Generator
import torch
from torch.utils.tensorboard import SummaryWriter
from tqdm import tqdm
from ppo import PPO
import sys
from pathlib import Path
from collections import deque


PORT = 8080
HOST = "127.0.0.1"
PACKET_SIZE = 65535
DEVICE = torch.accelerator.current_accelerator() if torch.accelerator.is_available() else torch.device("cpu")
LOG = logging.getLogger("agent")

WALL_PENALTY = 0.1
EXIT_REWARD = 50.0
PROGRESS_REWARD = 0.5
NEW_CELL_REWARD = 0.5
STEP_PENALTY = 0.01
REVISIT_PENALTY = 0.1
MAX_REVISIT_PENALTY = 1.0


NO_ACTION = (0, 0)
ACTIONS = [(0, 1), (1, 0), (0, -1), (-1, 0)] #UP, RIGHT, DOWN, LEFT
MAX_EPISODES = 1000
EVAL_WINDOW = 100
BEST_MODEL_PATH = Path("best_model.pth")
LAST_MODEL_PATH = Path("model.pth")



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

def compute_reward(pos: tuple[float, float], prev_pos: tuple[float, float], cell: tuple[int, int], prev_cell: tuple[int, int], prev_hint: list[int], steps_left: int, visit_count: int, hit: float, done: bool) -> tuple[float, dict[str, float]]:
    stats = {'wall_penalty': 0.0, 'step_penalty': 0.0, 'progress_reward': 0.0, 'new_cell_reward': 0.0, 'revisit_penalty': 0.0, 'exit_reward': 0.0}
    if done:
        r = EXIT_REWARD * (1 + steps_left/MAX_STEPS)
        stats['exit_reward'] = r
        return r, stats

    reward = -STEP_PENALTY
    stats['step_penalty'] -= STEP_PENALTY
    if hit:
        reward -= WALL_PENALTY
        stats['wall_penalty'] -= WALL_PENALTY

    if 1 in prev_hint:
        dx, dy = ACTIONS[prev_hint.index(1)]
        target = (prev_cell[0] + dx, prev_cell[1] + dy)
        progress = math.dist(prev_pos, target) - math.dist(pos, target)
        progress_r = PROGRESS_REWARD * progress
        stats['progress_reward'] = progress_r
        reward += progress_r

    if cell != prev_cell:
        if visit_count == 1:
            stats['new_cell_reward'] = NEW_CELL_REWARD
            reward += NEW_CELL_REWARD
        else:
            revisit_p = min(REVISIT_PENALTY * (visit_count - 1), MAX_REVISIT_PENALTY)
            stats['revisit_penalty'] = -revisit_p
            reward -= revisit_p


    return reward, stats

def episode(client: UDPClient) -> Generator[dict[str, Any], None, None]:
    while client.running:
        obs = client.receive_observations()
        if obs is None:
            client.running = False
            return None
        if "state" not in obs or "done" not in obs or "cell" not in obs or "hit" not in obs:
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
    if len(sys.argv) < 2 or sys.argv[1] not in ("train", "test"):
        LOG.error("Run with flag: train or test")
        sys.exit(1)

    train = True if sys.argv[1] == "train" else False 
    MAX_STEPS = 2000 if train else 4000  
    agent = PPO(17, len(ACTIONS), device=DEVICE, writer=writer, training=train)
    if not train:
        agent.load(BEST_MODEL_PATH)
    with UDPClient(PORT, HOST) as client:
        client.send_action(NO_ACTION)
        ep = 0
        successes: list[int] = []
        recent_rewards: deque[float] = deque(maxlen=EVAL_WINDOW)
        best_avg_reward = float("-inf")
        while client.running:
            step = 0
            reward: float|None = None
            truncated = False
            stats = {'wall_penalty': 0.0, 'step_penalty': 0.0, 'progress_reward': 0.0, 'new_cell_reward': 0.0, 'revisit_penalty': 0.0, 'exit_reward': 0.0}
            visited: dict[tuple[int, int], int] = {}
            prev_action: int|None = None
            prev_cell: tuple[int, int] = (0, 0)
            prev_pos: tuple[float, float] = (0.0, 0.0)
            prev_hint: list[int] = []
            try:
                with tqdm(episode(client), f"Episode {ep + 1}", unit="step", total=MAX_STEPS, leave=False, file=sys.stdout, colour="green") as pbar:

                    for obs in pbar:

                        actions = [0.0] * len(ACTIONS)

                        full_state = obs['state']
                        sensors = full_state['sensors']
                        hint = full_state['hint']
                        exit_vec = full_state['exit']
                        offset = full_state['offset']
                        x, y = obs['cell']
                        cell = (int(x), int(y))
                        pos = (cell[0] + offset[0], cell[1] + offset[1])
                        if prev_action is None or cell != prev_cell:
                            visited[cell] = visited.get(cell, 0) + 1

                        done = obs['done']
                        hit = obs['hit']
                        if prev_action is not None:
                            actions[prev_action] = 1.0
                        state = sensors + offset + hint + exit_vec + actions + [hit]
                        if prev_action is not None:
                            steps_left = MAX_STEPS - step
                            reward, details = compute_reward(pos, prev_pos, cell, prev_cell, prev_hint, steps_left, visited[cell], hit, done)
                            for k,v in details.items():
                                stats[k] += v
                            truncated = not done and step >= MAX_STEPS
                            agent.record(reward, done, truncated, state)
                            pbar.set_postfix(ep_reward=sum(stats.values()))
                            

                        if done or truncated:
                            successes.append(1 if done else 0)
                            client.send_action(NO_ACTION, True)
                            ep_reward = sum(stats.values())
                            summary = ", ".join(f"{k}={v:.2f}" for k, v in stats.items())
                            tqdm.write(f"Episode: {ep + 1} {'done' if done else 'truncated'}, steps={step} {summary}, ep_reward={ep_reward:.2f}")
                            writer.add_scalar("Episode/rewards", ep_reward, ep + 1, new_style=True)
                            writer.add_scalar("Episode/visited_cells", len(visited) - 1, ep + 1, new_style=True)
                            writer.add_scalar("Episode/length", step, ep + 1, new_style=True)
                            for k, v in stats.items():
                                writer.add_scalar(f"Reward/{k}", v, ep + 1, new_style=True)
                            if (ep + 1) % EVAL_WINDOW == 0:
                                block = successes[-EVAL_WINDOW:]
                                writer.add_scalar("Exit/success_rate", sum(block)/len(block), ep + 1, new_style=True)

                            if train:    
                                recent_rewards.append(ep_reward)
                                if len(recent_rewards) == EVAL_WINDOW:
                                    avg_reward = sum(recent_rewards) / len(recent_rewards)
                                    if avg_reward > best_avg_reward:
                                        best_avg_reward = avg_reward
                                        agent.save(BEST_MODEL_PATH)
                                        tqdm.write(f"New best: avg_reward={avg_reward:.2f} (ep {ep + 1}), saved to {BEST_MODEL_PATH}")
                            break
                    
                        action = agent.act(state)
                        prev_action = action
                        prev_cell = cell
                        prev_pos = pos
                        prev_hint = hint
                        client.send_action(ACTIONS[action])
                        step += 1
                    ep += 1
                    if ep == MAX_EPISODES:
                        if train:
                            agent.save(LAST_MODEL_PATH)
                        client.running = False
            except KeyboardInterrupt:
                if train:
                    agent.save(LAST_MODEL_PATH)
                client.running = False
        writer.flush()
        writer.close()       