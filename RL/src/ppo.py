import torch.nn as nn
import torch
import torch.distributions as distributions
import torch.optim as optim
from pathlib import Path
from torch.utils.tensorboard import SummaryWriter
import numpy as np


class PolicyNet(nn.Module):
    """The policy: outputs probability of each action."""

    def __init__(self, n_obs: int, n_actions: int):
        super().__init__()
        self.net = nn.Sequential(
            nn.Linear(n_obs, 128), nn.Tanh(),
            nn.Linear(128, 128), nn.Tanh(),
            nn.Linear(128, 128), nn.Tanh(),
            nn.Linear(128, n_actions)
        )

    def forward(self, obs: torch.Tensor) -> distributions.Categorical:
        return distributions.Categorical(logits=self.net(obs))


class ValueNet(nn.Module):
    """The value network: estimates how good the current state is."""

    def __init__(self, n_obs: int):
        super().__init__()
        self.net = nn.Sequential(
            nn.Linear(n_obs, 128), nn.Tanh(),
            nn.Linear(128, 128), nn.Tanh(),
            nn.Linear(128, 128), nn.Tanh(),
            nn.Linear(128, 1)
        )

    def forward(self, obs: torch.Tensor) -> torch.Tensor:
        return self.net(obs).squeeze(-1)


class PPO:
    """Proximal Policy Optimization.

            Args:
                n_obs: Number of observations.
                n_actions: Number of possible actions.
                n_steps: Number of steps to collect before learning.
                pl_lr: Policy learning rate.
                vf_lr: Value function learning rate.
                gamma: How much the agent cares about the future.
                lam: Trust the critic or the rewards. Low: relies more on critic's guesses, high: relies more on rewards.
                clip_ratio: How much the policy can change in one update.
                update_epochs: How many times the agent studies the same batch.
                batch_size: Samples per gradient step.
                ent_coef: Bonus for random actions.
            """
    def __init__(self, n_obs: int,
                 n_actions: int,
                 n_steps: int=1024,
                 pl_lr: float=0.0001,
                 vf_lr: float=0.001,
                 gamma: float=0.99,
                 lam: float=0.95,
                 clip_ratio: float=0.2,
                 update_epochs: int=10,
                 batch_size: int=64,
                 ent_coef: float=0.01,
                 device: torch.device = torch.device("cpu"),
                 writer: SummaryWriter|None = None,
                 training: bool=True):
        self.__device = device
        self.__policy = PolicyNet(n_obs, n_actions).to(self.__device)
        self.__value_net = ValueNet(n_obs).to(self.__device)
        self.__optim = optim.Adam([
            {"params": self.__policy.parameters(), "lr": pl_lr, "name": "policy"},
            {"params": self.__value_net.parameters(), "lr": vf_lr, "name": "value"}
        ])

        self.__n_steps = n_steps
        self.__gamma = gamma
        self.__lam = lam
        self.__clip_ratio = clip_ratio
        self.__update_epochs = update_epochs
        self.__batch_size = batch_size
        self.__ent_coef = ent_coef
        self.__training = training
        self.__writer = writer
        self.__steps = 0

        # The state the agent saw
        self.__states: list[torch.Tensor] = []

        #The chosen action
        self.__actions: list[int] = []

        #The log probability of the chosen action
        self.__log_probs: list[float] = []

        #The critic's guess
        self.__values: list[float] = []

        #The reward received
        self.__rewards: list[float] = []

        #Episode end(1: ended, 0: no)
        self.__dones: list[float] = []

        self.__old_logits: list[torch.Tensor] = []

    def act(self, state: list[float]) -> int:
        curr_state = torch.tensor(state, dtype=torch.float32, device=self.__device)

        if self.__training and len(self.__rewards) >= self.__n_steps: #Every n_steps learn
            with torch.no_grad():
                last_value = 0.0 if self.__dones[-1] else self.__value_net(curr_state).item()
            self._update(last_value)

        with torch.no_grad():
            dist = self.__policy(curr_state)
            # action = dist.sample() if self.__training else dist.probs.argmax(-1)
            action = dist.sample()

        if self.__training:
            self.__states.append(curr_state)
            self.__actions.append(int(action.item()))
            self.__log_probs.append(dist.log_prob(action).item())
            self.__old_logits.append(dist.logits)
            with torch.no_grad():
                self.__values.append(self.__value_net(curr_state).item())

        return int(action.item())

    def record(self, reward: float, done: bool, truncated: bool = False, state: list[float]|None = None) -> None:
        if not self.__training:
            return
        if truncated and not done and state is not None:
            with torch.no_grad():
                next_state = torch.tensor(state, dtype=torch.float32, device=self.__device).unsqueeze(0)
                reward += self.__gamma * self.__value_net(next_state).item()
        self.__rewards.append(reward)
        self.__dones.append(float(done or truncated))
        self.__steps += 1

    def save(self, path: Path):
        torch.save({"policy": self.__policy.state_dict(), "value": self.__value_net.state_dict()}, path)

    def load(self, path: Path) -> None:
        m = torch.load(path, map_location=self.__device, weights_only=True)
        self.__policy.load_state_dict(m["policy"])
        self.__value_net.load_state_dict(m["value"])

    def _clear(self) -> None:
        self.__states.clear()
        self.__actions.clear()
        self.__log_probs.clear()
        self.__values.clear()
        self.__rewards.clear()
        self.__dones.clear()
        self.__old_logits.clear()

    def _compute_gae(self, last_value: float) -> tuple[torch.Tensor, torch.Tensor]:
        """Compute advantages - how much better was this action than then value network expected"""
        n = len(self.__rewards)
        rewards = torch.tensor(self.__rewards, device=self.__device)
        dones = torch.tensor(self.__dones, device=self.__device)
        values = torch.tensor(self.__values, device=self.__device)

        advantages = torch.zeros(n, device=self.__device)
        gae = 0.0
        for t in reversed(range(n)):
            next_value = last_value if t == n - 1 else self.__values[t + 1]
            not_done = 1.0 - dones[t]
            delta = rewards[t] + self.__gamma * next_value * not_done - values[t]
            gae = delta + self.__gamma * self.__lam * not_done * gae
            advantages[t] = gae

        return advantages, advantages + values

    def _update(self, last_value: float) -> None:
        """Improve both networks"""
        advantages, returns = self._compute_gae(last_value)
        advantages = (advantages - advantages.mean()) / (advantages.std() + 1e-8) # 1e-8 prevents division by zero
        obs = torch.stack(self.__states)
        actions = torch.tensor(self.__actions, device=self.__device)
        old_log_probs = torch.tensor(self.__log_probs, device=self.__device)
        old_logits = torch.stack(self.__old_logits)
        n = len(self.__rewards)
        policy_losses, value_losses, entropies, kls = [], [], [], []
        for _ in range(self.__update_epochs):
            for idx in torch.randperm(n, device=self.__device).split(self.__batch_size):
                dist = self.__policy(obs[idx])
                value = self.__value_net(obs[idx])
                new_log_probs = dist.log_prob(actions[idx])

                adv = advantages[idx]

                ratio = torch.exp(new_log_probs - old_log_probs[idx]) #How much policy changed
                policy_loss = -torch.min(
                    ratio * adv,
                    torch.clamp(ratio, 1 - self.__clip_ratio, 1 + self.__clip_ratio) * adv
                ).mean() #PPO clip

                value_loss = 0.5 * ((value - returns[idx]) ** 2).mean()
                entropy = dist.entropy().mean()

                with torch.no_grad():
                    old_dist = distributions.Categorical(logits=old_logits[idx])
                    kl = distributions.kl_divergence(old_dist, dist).mean().item()
                kls.append(kl)    

                policy_losses.append(policy_loss.item())
                value_losses.append(value_loss.item())
                entropies.append(entropy.item())

                loss = policy_loss + value_loss - self.__ent_coef * entropy
                self.__optim.zero_grad()
                loss.backward()
                self.__optim.step()

        if self.__writer is not None:
            self.__writer.add_scalar("Loss/policy", np.mean(policy_losses), self.__steps, new_style=True)
            self.__writer.add_scalar("Loss/value", np.mean(value_losses), self.__steps, new_style=True)
            self.__writer.add_scalar("Policy/entropy", np.mean(entropies), self.__steps, new_style=True)
            self.__writer.add_scalar("Policy/kl_mean", np.mean(kls), self.__steps, new_style=True)
            

        self._clear()