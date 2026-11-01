import torch.nn as nn
import torch
import torch.distributions as distributions
import torch.optim as optim
from pathlib import Path


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
                 training: bool=True):
        self._device = device
        self._policy = PolicyNet(n_obs, n_actions).to(self._device)
        self._value_net = ValueNet(n_obs).to(self._device)
        self._optim = optim.Adam([
            {"params": self._policy.parameters(), "lr": pl_lr, "name": "policy"},
            {"params": self._value_net.parameters(), "lr": vf_lr, "name": "value"}
        ])

        self._n_steps = n_steps
        self._gamma = gamma
        self._lam = lam
        self._clip_ratio = clip_ratio
        self._update_epochs = update_epochs
        self._batch_size = batch_size
        self._ent_coef = ent_coef
        self._training = training

        # The state the agent saw
        self._states: list[torch.Tensor] = []

        #The chosen action
        self._actions: list[int] = []

        #The log probability of the chosen action
        self._log_probs: list[float] = []

        #The critic's guess
        self._values: list[float] = []

        #The reward received
        self._rewards: list[float] = []

        #Episode end(1: ended, 0: no)
        self._dones: list[float] = []

    def act(self, state: list[float]) -> int:
        curr_state = torch.tensor(state, dtype=torch.float32, device=self._device)

        if self._training and len(self._rewards) >= self._n_steps: #Every n_steps learn
            with torch.no_grad():
                last_value = 0.0 if self._dones[-1] else self._value_net(curr_state).item()
            self._update(last_value)

        with torch.no_grad():
            dist = self._policy(curr_state)
            action = dist.sample() if self._training else dist.probs.argmax(-1)

        if self._training:
            self._states.append(curr_state.squeeze(0))
            self._actions.append(int(action.item()))
            self._log_probs.append(dist.log_prob(action).item())
            with torch.no_grad():
                self._values.append(self._value_net(curr_state).item())

        return int(action.item())

    def record(self, reward: float, done: bool, truncated: bool = False, state: list[float]|None = None) -> None:
        if not self._training:
            return
        if truncated and not done and state is not None:
            with torch.no_grad():
                next_state = torch.tensor(state, dtype=torch.float32, device=self._device).unsqueeze(0)
                reward += self._gamma * self._value_net(next_state).item()
        self._rewards.append(reward)
        self._dones.append(float(done or truncated))

    def save(self, path: Path):
        torch.save({"policy": self._policy.state_dict(), "value": self._value_net.state_dict()}, path)

    def load(self, path: Path) -> None:
        m = torch.load(path, map_location=self._device)
        self._policy.load_state_dict(m["policy"])
        self._value_net.load_state_dict(m["value"])

    def _clear(self) -> None:
        self._states.clear()
        self._actions.clear()
        self._log_probs.clear()
        self._values.clear()
        self._rewards.clear()
        self._dones.clear()

    def _compute_gae(self, last_value: float) -> tuple[torch.Tensor, torch.Tensor]:
        """Compute advantages - how much better was this action than then value network expected"""
        n = len(self._rewards)
        rewards = torch.tensor(self._rewards, device=self._device)
        dones = torch.tensor(self._dones, device=self._device)
        values = torch.tensor(self._values, device=self._device)

        advantages = torch.zeros(n, device=self._device)
        gae = 0.0
        for t in reversed(range(n)):
            next_value = last_value if t == n - 1 else self._values[t + 1]
            not_done = 1.0 - dones[t]
            delta = rewards[t] + self._gamma * next_value * not_done - values[t]
            gae = delta + self._gamma * self._lam * not_done * gae
            advantages[t] = gae

        return advantages, advantages + values

    def _update(self, last_value: float) -> None:
        """Improve both networks"""
        advantages, returns = self._compute_gae(last_value)

        obs = torch.stack(self._states)
        actions = torch.tensor(self._actions, device=self._device)
        old_log_probs = torch.tensor(self._log_probs, device=self._device)
        n = len(self._rewards)

        for _ in range(self._update_epochs):
            for idx in torch.randperm(n, device=self._device).split(self._batch_size):
                dist = self._policy(obs[idx])
                value = self._value_net(obs[idx])
                new_log_probs = dist.log_prob(actions[idx])

                adv = advantages[idx]
                adv = (adv - adv.mean()) / (adv.std() + 1e-8) # 1e-8 prevents division by zero

                ratio = torch.exp(new_log_probs - old_log_probs[idx]) #How much policy changed
                policy_loss = -torch.min(
                    ratio * adv,
                    torch.clamp(ratio, 1 - self._clip_ratio, 1 + self._clip_ratio) * adv
                ).mean() #PPO clip

                value_loss = 0.5 * ((value - returns[idx]) ** 2).mean()
                entropy = dist.entropy().mean()

                loss = policy_loss + value_loss - self._ent_coef * entropy
                self._optim.zero_grad()
                loss.backward()
                self._optim.step()

        self._clear()