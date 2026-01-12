# R-Type Editor Suite

Python-based level and asset editor for the R-Type game.

## Requirements

- Python 3.8+
- Pillow (for sprite viewing)
- pygame (for audio playback)

## Installation

```bash
cd tools/editor
pip install -r requirements.txt
```

## Usage

### Run the full editor suite:
```bash
python main.py
```

### Run individual editors:
```bash
# Level editor only
python level_editor.py

# Asset editor only
python asset_editor.py
```

## Features

### Level Editor
- Create and edit level JSON files compatible with LevelLoader
- Visual wave timeline editor
- Drag-and-drop enemy placement on a scaled game canvas
- Edit enemy properties: type, position, velocity, health, score
- Wave settings: delay, spawn interval, simultaneous mode
- Import/export JSON files from `config/levels/`

### Asset Editor
- **Sprite Viewer**: Open sprite sheets, adjust frame size, click to select frames, copy coordinates
- **Audio Player**: Browse and play sound/music files from `assets/sound/`
- **Asset Browser**: Browse all game assets with preview

## Level JSON Format

```json
{
  "name": "Level Name",
  "difficulty": 1,
  "waveDelay": 2.0,
  "waves": [
    {
      "delayBefore": 1.0,
      "spawnInterval": 0.5,
      "simultaneous": false,
      "enemies": [
        {
          "type": "basic",
          "x": 1300,
          "y": 360,
          "vx": -100,
          "vy": 0,
          "health": 1,
          "scoreValue": 100
        }
      ]
    }
  ]
}
```

## Enemy Types

| Type | Description |
|------|-------------|
| basic | Simple straight-line enemy |
| shooter | Fires projectiles at player |
| chaser | Follows/tracks player |
| boss | Boss enemy with high health |
| turret | Stationary shooter |
| sine_wave | Moves in sine wave pattern |
| fast | High-speed enemy |
| tank | Slow, high health enemy |
| zigzag | Sharp angular movement |
| circular | Circular/orbital motion |
| follow | Follows player position |
| dive_bomber | Dives toward player |
