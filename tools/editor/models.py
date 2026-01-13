"""
R-Type Level Editor - Data Models
Dataclasses matching the C++ level configuration structures
"""

from dataclasses import dataclass, field
from typing import List, Optional
import json


@dataclass
class EnemySpawnConfig:
    """Configuration for spawning a single enemy"""
    type: str = "basic"
    x: float = 1300.0
    y: float = 360.0
    vx: float = -100.0
    vy: float = 0.0
    health: int = 1
    scoreValue: int = 100

    def to_dict(self) -> dict:
        return {
            "type": self.type,
            "x": self.x,
            "y": self.y,
            "vx": self.vx,
            "vy": self.vy,
            "health": self.health,
            "scoreValue": self.scoreValue
        }

    @classmethod
    def from_dict(cls, data: dict) -> 'EnemySpawnConfig':
        return cls(
            type=data.get("type", "basic"),
            x=float(data.get("x", 1300.0)),
            y=float(data.get("y", 360.0)),
            vx=float(data.get("vx", -100.0)),
            vy=float(data.get("vy", 0.0)),
            health=int(data.get("health", 1)),
            scoreValue=int(data.get("scoreValue", 100))
        )


@dataclass
class WaveConfig:
    """Configuration for a wave of enemies"""
    delayBefore: float = 0.0
    spawnInterval: float = 0.5
    simultaneous: bool = False
    enemies: List[EnemySpawnConfig] = field(default_factory=list)

    def to_dict(self) -> dict:
        return {
            "delayBefore": self.delayBefore,
            "spawnInterval": self.spawnInterval,
            "simultaneous": self.simultaneous,
            "enemies": [e.to_dict() for e in self.enemies]
        }

    @classmethod
    def from_dict(cls, data: dict) -> 'WaveConfig':
        enemies = [EnemySpawnConfig.from_dict(e) for e in data.get("enemies", [])]
        return cls(
            delayBefore=float(data.get("delayBefore", 0.0)),
            spawnInterval=float(data.get("spawnInterval", 0.5)),
            simultaneous=bool(data.get("simultaneous", False)),
            enemies=enemies
        )


@dataclass
class LevelConfig:
    """Configuration for a complete level"""
    name: str = "New Level"
    difficulty: int = 1
    waveDelay: float = 2.0
    waves: List[WaveConfig] = field(default_factory=list)
    # Level assets
    background: str = ""
    stageMusic: str = ""
    bossMusic: str = ""

    def to_dict(self) -> dict:
        result = {
            "name": self.name,
            "difficulty": self.difficulty,
            "waveDelay": self.waveDelay,
            "waves": [w.to_dict() for w in self.waves]
        }
        # Only include asset fields if they have values
        if self.background:
            result["background"] = self.background
        if self.stageMusic:
            result["stageMusic"] = self.stageMusic
        if self.bossMusic:
            result["bossMusic"] = self.bossMusic
        return result

    @classmethod
    def from_dict(cls, data: dict) -> 'LevelConfig':
        waves = [WaveConfig.from_dict(w) for w in data.get("waves", [])]
        return cls(
            name=data.get("name", "New Level"),
            difficulty=int(data.get("difficulty", 1)),
            waveDelay=float(data.get("waveDelay", 2.0)),
            waves=waves,
            background=data.get("background", ""),
            stageMusic=data.get("stageMusic", ""),
            bossMusic=data.get("bossMusic", "")
        )

    def to_json(self, indent: int = 2) -> str:
        return json.dumps(self.to_dict(), indent=indent)

    @classmethod
    def from_json(cls, json_str: str) -> 'LevelConfig':
        data = json.loads(json_str)
        return cls.from_dict(data)

    def save(self, filepath: str) -> None:
        with open(filepath, 'w') as f:
            f.write(self.to_json())

    @classmethod
    def load(cls, filepath: str) -> 'LevelConfig':
        with open(filepath, 'r') as f:
            return cls.from_json(f.read())


# Enemy type definitions with colors for visualization
ENEMY_TYPES = {
    "basic": {"color": "#4444FF", "name": "Basic", "desc": "Simple straight-line enemy"},
    "shooter": {"color": "#FF4444", "name": "Shooter", "desc": "Fires projectiles at player"},
    "chaser": {"color": "#FF44FF", "name": "Chaser", "desc": "Follows/tracks player"},
    "boss": {"color": "#FFD700", "name": "Boss", "desc": "Boss enemy with high health"},
    "turret": {"color": "#888888", "name": "Turret", "desc": "Stationary shooter"},
    "sine_wave": {"color": "#44FF44", "name": "Sine Wave", "desc": "Moves in sine wave pattern"},
    "fast": {"color": "#00FFFF", "name": "Fast", "desc": "High-speed enemy"},
    "tank": {"color": "#8B4513", "name": "Tank", "desc": "Slow, high health enemy"},
    "zigzag": {"color": "#FF8800", "name": "Zigzag", "desc": "Sharp angular movement"},
    "circular": {"color": "#AA44AA", "name": "Circular", "desc": "Circular/orbital motion"},
    "follow": {"color": "#FF66FF", "name": "Follow", "desc": "Follows player position"},
    "dive_bomber": {"color": "#FF0000", "name": "Dive Bomber", "desc": "Dives toward player"},
}

# Powerup types
POWERUP_TYPES = {
    "SPREAD_SHOT": {"color": "#00FFFF", "value": 0},
    "SPEED_BOOST": {"color": "#FFFF00", "value": 1},
    "HEALTH_UP": {"color": "#00FF00", "value": 2},
    "SHIELD": {"color": "#6496FF", "value": 3},
    "WEAPON_UPGRADE": {"color": "#FF9600", "value": 4},
    "FORCE_ORB": {"color": "#64B4FF", "value": 5},
    "BOMB": {"color": "#FF6432", "value": 6},
}

# Game constants
SCREEN_WIDTH = 1280
SCREEN_HEIGHT = 720
SPAWN_X = 1300  # Default spawn position (off-screen right)
