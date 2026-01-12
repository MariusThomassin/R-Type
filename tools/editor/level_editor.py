"""
R-Type Level Editor
Visual editor for creating and editing level JSON files
"""

import tkinter as tk
from tkinter import ttk, filedialog, messagebox, simpledialog
from typing import Optional, Callable
import os

from models import (
    LevelConfig, WaveConfig, EnemySpawnConfig,
    ENEMY_TYPES, SCREEN_WIDTH, SCREEN_HEIGHT, SPAWN_X
)


class EnemyPropertyEditor(ttk.Frame):
    """Panel for editing enemy properties"""
    
    def __init__(self, parent, on_change: Callable = None):
        super().__init__(parent)
        self.on_change = on_change
        self.enemy: Optional[EnemySpawnConfig] = None
        self._building = False
        self._build_ui()
    
    def _build_ui(self):
        self._building = True
        
        # Type selector
        ttk.Label(self, text="Type:").grid(row=0, column=0, sticky="w", padx=5, pady=2)
        self.type_var = tk.StringVar()
        self.type_combo = ttk.Combobox(self, textvariable=self.type_var, 
                                        values=list(ENEMY_TYPES.keys()), width=12)
        self.type_combo.grid(row=0, column=1, sticky="ew", padx=5, pady=2)
        self.type_combo.bind("<<ComboboxSelected>>", self._on_type_change)
        
        # Position
        ttk.Label(self, text="X:").grid(row=1, column=0, sticky="w", padx=5, pady=2)
        self.x_var = tk.DoubleVar()
        self.x_spin = ttk.Spinbox(self, from_=0, to=1500, textvariable=self.x_var, width=10)
        self.x_spin.grid(row=1, column=1, sticky="ew", padx=5, pady=2)
        self.x_spin.bind("<FocusOut>", self._on_value_change)
        self.x_spin.bind("<Return>", self._on_value_change)
        
        ttk.Label(self, text="Y:").grid(row=2, column=0, sticky="w", padx=5, pady=2)
        self.y_var = tk.DoubleVar()
        self.y_spin = ttk.Spinbox(self, from_=0, to=720, textvariable=self.y_var, width=10)
        self.y_spin.grid(row=2, column=1, sticky="ew", padx=5, pady=2)
        self.y_spin.bind("<FocusOut>", self._on_value_change)
        self.y_spin.bind("<Return>", self._on_value_change)
        
        # Velocity
        ttk.Label(self, text="VX:").grid(row=3, column=0, sticky="w", padx=5, pady=2)
        self.vx_var = tk.DoubleVar()
        self.vx_spin = ttk.Spinbox(self, from_=-500, to=500, textvariable=self.vx_var, width=10)
        self.vx_spin.grid(row=3, column=1, sticky="ew", padx=5, pady=2)
        self.vx_spin.bind("<FocusOut>", self._on_value_change)
        self.vx_spin.bind("<Return>", self._on_value_change)
        
        ttk.Label(self, text="VY:").grid(row=4, column=0, sticky="w", padx=5, pady=2)
        self.vy_var = tk.DoubleVar()
        self.vy_spin = ttk.Spinbox(self, from_=-500, to=500, textvariable=self.vy_var, width=10)
        self.vy_spin.grid(row=4, column=1, sticky="ew", padx=5, pady=2)
        self.vy_spin.bind("<FocusOut>", self._on_value_change)
        self.vy_spin.bind("<Return>", self._on_value_change)
        
        # Health
        ttk.Label(self, text="Health:").grid(row=5, column=0, sticky="w", padx=5, pady=2)
        self.health_var = tk.IntVar()
        self.health_spin = ttk.Spinbox(self, from_=1, to=100, textvariable=self.health_var, width=10)
        self.health_spin.grid(row=5, column=1, sticky="ew", padx=5, pady=2)
        self.health_spin.bind("<FocusOut>", self._on_value_change)
        self.health_spin.bind("<Return>", self._on_value_change)
        
        # Score
        ttk.Label(self, text="Score:").grid(row=6, column=0, sticky="w", padx=5, pady=2)
        self.score_var = tk.IntVar()
        self.score_spin = ttk.Spinbox(self, from_=0, to=10000, increment=50, 
                                       textvariable=self.score_var, width=10)
        self.score_spin.grid(row=6, column=1, sticky="ew", padx=5, pady=2)
        self.score_spin.bind("<FocusOut>", self._on_value_change)
        self.score_spin.bind("<Return>", self._on_value_change)
        
        self.columnconfigure(1, weight=1)
        self._building = False
    
    def set_enemy(self, enemy: Optional[EnemySpawnConfig]):
        self._building = True
        self.enemy = enemy
        if enemy:
            self.type_var.set(enemy.type)
            self.x_var.set(enemy.x)
            self.y_var.set(enemy.y)
            self.vx_var.set(enemy.vx)
            self.vy_var.set(enemy.vy)
            self.health_var.set(enemy.health)
            self.score_var.set(enemy.scoreValue)
            self._set_enabled(True)
        else:
            self._set_enabled(False)
        self._building = False
    
    def _set_enabled(self, enabled: bool):
        state = "normal" if enabled else "disabled"
        for widget in [self.type_combo, self.x_spin, self.y_spin, 
                       self.vx_spin, self.vy_spin, self.health_spin, self.score_spin]:
            widget.configure(state=state)
    
    def _on_type_change(self, event=None):
        if self._building or not self.enemy:
            return
        self.enemy.type = self.type_var.get()
        if self.on_change:
            self.on_change()
    
    def _on_value_change(self, event=None):
        if self._building or not self.enemy:
            return
        try:
            self.enemy.x = self.x_var.get()
            self.enemy.y = self.y_var.get()
            self.enemy.vx = self.vx_var.get()
            self.enemy.vy = self.vy_var.get()
            self.enemy.health = self.health_var.get()
            self.enemy.scoreValue = self.score_var.get()
            if self.on_change:
                self.on_change()
        except tk.TclError:
            pass


class WavePropertyEditor(ttk.Frame):
    """Panel for editing wave properties"""
    
    def __init__(self, parent, on_change: Callable = None):
        super().__init__(parent)
        self.on_change = on_change
        self.wave: Optional[WaveConfig] = None
        self._building = False
        self._build_ui()
    
    def _build_ui(self):
        self._building = True
        
        ttk.Label(self, text="Delay Before:").grid(row=0, column=0, sticky="w", padx=5, pady=2)
        self.delay_var = tk.DoubleVar()
        self.delay_spin = ttk.Spinbox(self, from_=0, to=30, increment=0.5, 
                                       textvariable=self.delay_var, width=10)
        self.delay_spin.grid(row=0, column=1, sticky="ew", padx=5, pady=2)
        self.delay_spin.bind("<FocusOut>", self._on_value_change)
        self.delay_spin.bind("<Return>", self._on_value_change)
        
        ttk.Label(self, text="Spawn Interval:").grid(row=1, column=0, sticky="w", padx=5, pady=2)
        self.interval_var = tk.DoubleVar()
        self.interval_spin = ttk.Spinbox(self, from_=0.1, to=10, increment=0.1, 
                                          textvariable=self.interval_var, width=10)
        self.interval_spin.grid(row=1, column=1, sticky="ew", padx=5, pady=2)
        self.interval_spin.bind("<FocusOut>", self._on_value_change)
        self.interval_spin.bind("<Return>", self._on_value_change)
        
        self.simultaneous_var = tk.BooleanVar()
        self.simultaneous_check = ttk.Checkbutton(self, text="Simultaneous Spawn", 
                                                   variable=self.simultaneous_var,
                                                   command=self._on_value_change)
        self.simultaneous_check.grid(row=2, column=0, columnspan=2, sticky="w", padx=5, pady=5)
        
        self.columnconfigure(1, weight=1)
        self._building = False
    
    def set_wave(self, wave: Optional[WaveConfig]):
        self._building = True
        self.wave = wave
        if wave:
            self.delay_var.set(wave.delayBefore)
            self.interval_var.set(wave.spawnInterval)
            self.simultaneous_var.set(wave.simultaneous)
            self._set_enabled(True)
        else:
            self._set_enabled(False)
        self._building = False
    
    def _set_enabled(self, enabled: bool):
        state = "normal" if enabled else "disabled"
        for widget in [self.delay_spin, self.interval_spin, self.simultaneous_check]:
            widget.configure(state=state)
    
    def _on_value_change(self, event=None):
        if self._building or not self.wave:
            return
        try:
            self.wave.delayBefore = self.delay_var.get()
            self.wave.spawnInterval = self.interval_var.get()
            self.wave.simultaneous = self.simultaneous_var.get()
            if self.on_change:
                self.on_change()
        except tk.TclError:
            pass


class GameCanvas(tk.Canvas):
    """Canvas for visualizing enemy positions"""
    
    SCALE = 0.5  # Scale factor for display
    
    def __init__(self, parent, on_select: Callable = None, on_move: Callable = None):
        width = int(SCREEN_WIDTH * self.SCALE)
        height = int(SCREEN_HEIGHT * self.SCALE)
        super().__init__(parent, width=width, height=height, bg="#1a1a2e", 
                         highlightthickness=1, highlightbackground="#444")
        
        self.on_select = on_select
        self.on_move = on_move
        self.wave: Optional[WaveConfig] = None
        self.selected_enemy: Optional[EnemySpawnConfig] = None
        self.enemy_items = {}  # enemy -> canvas item id
        self.dragging = False
        self.drag_start = None
        
        self.bind("<Button-1>", self._on_click)
        self.bind("<B1-Motion>", self._on_drag)
        self.bind("<ButtonRelease-1>", self._on_release)
        
        self._draw_grid()
    
    def _draw_grid(self):
        """Draw background grid"""
        self.delete("grid")
        w = int(SCREEN_WIDTH * self.SCALE)
        h = int(SCREEN_HEIGHT * self.SCALE)
        
        # Grid lines every 100 game units
        grid_spacing = int(100 * self.SCALE)
        for x in range(0, w, grid_spacing):
            self.create_line(x, 0, x, h, fill="#2a2a4e", tags="grid")
        for y in range(0, h, grid_spacing):
            self.create_line(0, y, w, y, fill="#2a2a4e", tags="grid")
        
        # Player spawn zone indicator
        player_x = int(150 * self.SCALE)
        self.create_line(player_x, 0, player_x, h, fill="#446644", dash=(4, 4), tags="grid")
        self.create_text(player_x + 5, 10, text="Player", fill="#668866", 
                         anchor="nw", font=("Arial", 8), tags="grid")
    
    def set_wave(self, wave: Optional[WaveConfig]):
        self.wave = wave
        self.selected_enemy = None
        self.refresh()
    
    def refresh(self):
        """Redraw all enemies"""
        self.delete("enemy")
        self.enemy_items.clear()
        
        if not self.wave:
            return
        
        for enemy in self.wave.enemies:
            self._draw_enemy(enemy)
    
    def _draw_enemy(self, enemy: EnemySpawnConfig):
        """Draw a single enemy"""
        x = int(min(enemy.x, SCREEN_WIDTH) * self.SCALE)
        y = int(enemy.y * self.SCALE)
        r = 8  # radius
        
        color = ENEMY_TYPES.get(enemy.type, {}).get("color", "#FFFFFF")
        outline = "#FFFFFF" if enemy == self.selected_enemy else "#888888"
        width = 2 if enemy == self.selected_enemy else 1
        
        item = self.create_oval(x - r, y - r, x + r, y + r, 
                                fill=color, outline=outline, width=width, tags="enemy")
        self.enemy_items[enemy] = item
        
        # Draw velocity arrow
        arrow_scale = 0.3
        ax = x + int(enemy.vx * arrow_scale * self.SCALE)
        ay = y + int(enemy.vy * arrow_scale * self.SCALE)
        self.create_line(x, y, ax, ay, fill=color, arrow=tk.LAST, 
                         arrowshape=(6, 8, 3), tags="enemy")
    
    def select_enemy(self, enemy: Optional[EnemySpawnConfig]):
        self.selected_enemy = enemy
        self.refresh()
    
    def _on_click(self, event):
        if not self.wave:
            return
        
        # Find clicked enemy
        clicked = None
        for enemy, item in self.enemy_items.items():
            coords = self.coords(item)
            if coords:
                x1, y1, x2, y2 = coords
                if x1 <= event.x <= x2 and y1 <= event.y <= y2:
                    clicked = enemy
                    break
        
        if clicked:
            self.selected_enemy = clicked
            self.dragging = True
            self.drag_start = (event.x, event.y)
            if self.on_select:
                self.on_select(clicked)
        else:
            self.selected_enemy = None
            if self.on_select:
                self.on_select(None)
        
        self.refresh()
    
    def _on_drag(self, event):
        if not self.dragging or not self.selected_enemy:
            return
        
        # Update enemy position
        self.selected_enemy.x = event.x / self.SCALE
        self.selected_enemy.y = event.y / self.SCALE
        
        # Clamp to screen
        self.selected_enemy.y = max(20, min(SCREEN_HEIGHT - 20, self.selected_enemy.y))
        
        self.refresh()
        if self.on_move:
            self.on_move()
    
    def _on_release(self, event):
        self.dragging = False
        self.drag_start = None


class LevelEditor(ttk.Frame):
    """Main level editor panel"""
    
    def __init__(self, parent):
        super().__init__(parent)
        self.level = LevelConfig()
        self.current_wave_idx = -1
        self.current_file = None
        self._build_ui()
        self._update_wave_list()
    
    def _build_ui(self):
        # Top toolbar
        toolbar = ttk.Frame(self)
        toolbar.pack(fill="x", padx=5, pady=5)
        
        ttk.Button(toolbar, text="New", command=self._new_level).pack(side="left", padx=2)
        ttk.Button(toolbar, text="Open", command=self._open_level).pack(side="left", padx=2)
        ttk.Button(toolbar, text="Save", command=self._save_level).pack(side="left", padx=2)
        ttk.Button(toolbar, text="Save As", command=self._save_level_as).pack(side="left", padx=2)
        
        ttk.Separator(toolbar, orient="vertical").pack(side="left", fill="y", padx=10)
        
        # Level properties
        ttk.Label(toolbar, text="Name:").pack(side="left", padx=2)
        self.name_var = tk.StringVar(value=self.level.name)
        self.name_entry = ttk.Entry(toolbar, textvariable=self.name_var, width=20)
        self.name_entry.pack(side="left", padx=2)
        self.name_entry.bind("<FocusOut>", self._on_name_change)
        
        ttk.Label(toolbar, text="Difficulty:").pack(side="left", padx=2)
        self.difficulty_var = tk.IntVar(value=self.level.difficulty)
        self.difficulty_spin = ttk.Spinbox(toolbar, from_=1, to=10, 
                                            textvariable=self.difficulty_var, width=5)
        self.difficulty_spin.pack(side="left", padx=2)
        self.difficulty_spin.bind("<FocusOut>", self._on_difficulty_change)
        
        ttk.Label(toolbar, text="Wave Delay:").pack(side="left", padx=2)
        self.wave_delay_var = tk.DoubleVar(value=self.level.waveDelay)
        self.wave_delay_spin = ttk.Spinbox(toolbar, from_=0.5, to=10, increment=0.5,
                                            textvariable=self.wave_delay_var, width=5)
        self.wave_delay_spin.pack(side="left", padx=2)
        self.wave_delay_spin.bind("<FocusOut>", self._on_wave_delay_change)
        
        # Main content
        main_pane = ttk.PanedWindow(self, orient="horizontal")
        main_pane.pack(fill="both", expand=True, padx=5, pady=5)
        
        # Left panel - Wave list
        left_frame = ttk.Frame(main_pane)
        main_pane.add(left_frame, weight=1)
        
        ttk.Label(left_frame, text="Waves", font=("Arial", 10, "bold")).pack(anchor="w", pady=(0, 5))
        
        wave_btn_frame = ttk.Frame(left_frame)
        wave_btn_frame.pack(fill="x", pady=(0, 5))
        ttk.Button(wave_btn_frame, text="+", width=3, command=self._add_wave).pack(side="left")
        ttk.Button(wave_btn_frame, text="-", width=3, command=self._remove_wave).pack(side="left")
        ttk.Button(wave_btn_frame, text="▲", width=3, command=self._move_wave_up).pack(side="left")
        ttk.Button(wave_btn_frame, text="▼", width=3, command=self._move_wave_down).pack(side="left")
        
        self.wave_listbox = tk.Listbox(left_frame, height=15, exportselection=False)
        self.wave_listbox.pack(fill="both", expand=True)
        self.wave_listbox.bind("<<ListboxSelect>>", self._on_wave_select)
        
        # Wave properties
        ttk.Label(left_frame, text="Wave Settings", font=("Arial", 10, "bold")).pack(anchor="w", pady=(10, 5))
        self.wave_editor = WavePropertyEditor(left_frame, on_change=self._on_wave_change)
        self.wave_editor.pack(fill="x")
        
        # Center panel - Canvas
        center_frame = ttk.Frame(main_pane)
        main_pane.add(center_frame, weight=3)
        
        ttk.Label(center_frame, text="Enemy Positions (drag to move)", 
                  font=("Arial", 10, "bold")).pack(anchor="w", pady=(0, 5))
        
        self.canvas = GameCanvas(center_frame, 
                                  on_select=self._on_enemy_select,
                                  on_move=self._on_enemy_move)
        self.canvas.pack()
        
        # Enemy buttons
        enemy_btn_frame = ttk.Frame(center_frame)
        enemy_btn_frame.pack(fill="x", pady=5)
        ttk.Button(enemy_btn_frame, text="Add Enemy", command=self._add_enemy).pack(side="left")
        ttk.Button(enemy_btn_frame, text="Remove Enemy", command=self._remove_enemy).pack(side="left")
        ttk.Button(enemy_btn_frame, text="Duplicate", command=self._duplicate_enemy).pack(side="left")
        
        # Right panel - Enemy list and properties
        right_frame = ttk.Frame(main_pane)
        main_pane.add(right_frame, weight=1)
        
        ttk.Label(right_frame, text="Enemies in Wave", font=("Arial", 10, "bold")).pack(anchor="w", pady=(0, 5))
        
        self.enemy_listbox = tk.Listbox(right_frame, height=10, exportselection=False)
        self.enemy_listbox.pack(fill="both", expand=True)
        self.enemy_listbox.bind("<<ListboxSelect>>", self._on_enemy_list_select)
        
        ttk.Label(right_frame, text="Enemy Properties", font=("Arial", 10, "bold")).pack(anchor="w", pady=(10, 5))
        self.enemy_editor = EnemyPropertyEditor(right_frame, on_change=self._on_enemy_change)
        self.enemy_editor.pack(fill="x")
    
    def _update_wave_list(self):
        self.wave_listbox.delete(0, tk.END)
        for i, wave in enumerate(self.level.waves):
            enemy_count = len(wave.enemies)
            self.wave_listbox.insert(tk.END, f"Wave {i+1} ({enemy_count} enemies)")
        
        if self.current_wave_idx >= 0 and self.current_wave_idx < len(self.level.waves):
            self.wave_listbox.selection_set(self.current_wave_idx)
    
    def _update_enemy_list(self):
        self.enemy_listbox.delete(0, tk.END)
        if self.current_wave_idx < 0 or self.current_wave_idx >= len(self.level.waves):
            return
        
        wave = self.level.waves[self.current_wave_idx]
        for i, enemy in enumerate(wave.enemies):
            self.enemy_listbox.insert(tk.END, f"{i+1}. {enemy.type} @ ({int(enemy.x)}, {int(enemy.y)})")
    
    def _on_wave_select(self, event):
        selection = self.wave_listbox.curselection()
        if selection:
            self.current_wave_idx = selection[0]
            wave = self.level.waves[self.current_wave_idx]
            self.wave_editor.set_wave(wave)
            self.canvas.set_wave(wave)
            self._update_enemy_list()
            self.enemy_editor.set_enemy(None)
        else:
            self.current_wave_idx = -1
            self.wave_editor.set_wave(None)
            self.canvas.set_wave(None)
            self.enemy_listbox.delete(0, tk.END)
            self.enemy_editor.set_enemy(None)
    
    def _on_enemy_select(self, enemy):
        self.enemy_editor.set_enemy(enemy)
        if enemy and self.current_wave_idx >= 0:
            wave = self.level.waves[self.current_wave_idx]
            try:
                idx = wave.enemies.index(enemy)
                self.enemy_listbox.selection_clear(0, tk.END)
                self.enemy_listbox.selection_set(idx)
            except ValueError:
                pass
    
    def _on_enemy_list_select(self, event):
        selection = self.enemy_listbox.curselection()
        if selection and self.current_wave_idx >= 0:
            wave = self.level.waves[self.current_wave_idx]
            enemy = wave.enemies[selection[0]]
            self.enemy_editor.set_enemy(enemy)
            self.canvas.select_enemy(enemy)
    
    def _on_wave_change(self):
        self._update_wave_list()
    
    def _on_enemy_change(self):
        self.canvas.refresh()
        self._update_enemy_list()
    
    def _on_enemy_move(self):
        self.enemy_editor.set_enemy(self.canvas.selected_enemy)
        self._update_enemy_list()
    
    def _on_name_change(self, event=None):
        self.level.name = self.name_var.get()
    
    def _on_difficulty_change(self, event=None):
        try:
            self.level.difficulty = self.difficulty_var.get()
        except tk.TclError:
            pass
    
    def _on_wave_delay_change(self, event=None):
        try:
            self.level.waveDelay = self.wave_delay_var.get()
        except tk.TclError:
            pass
    
    def _add_wave(self):
        wave = WaveConfig()
        self.level.waves.append(wave)
        self._update_wave_list()
        self.wave_listbox.selection_clear(0, tk.END)
        self.wave_listbox.selection_set(len(self.level.waves) - 1)
        self._on_wave_select(None)
    
    def _remove_wave(self):
        if self.current_wave_idx >= 0:
            del self.level.waves[self.current_wave_idx]
            self.current_wave_idx = -1
            self._update_wave_list()
            self.wave_editor.set_wave(None)
            self.canvas.set_wave(None)
            self.enemy_listbox.delete(0, tk.END)
    
    def _move_wave_up(self):
        if self.current_wave_idx > 0:
            waves = self.level.waves
            waves[self.current_wave_idx], waves[self.current_wave_idx - 1] = \
                waves[self.current_wave_idx - 1], waves[self.current_wave_idx]
            self.current_wave_idx -= 1
            self._update_wave_list()
    
    def _move_wave_down(self):
        if self.current_wave_idx >= 0 and self.current_wave_idx < len(self.level.waves) - 1:
            waves = self.level.waves
            waves[self.current_wave_idx], waves[self.current_wave_idx + 1] = \
                waves[self.current_wave_idx + 1], waves[self.current_wave_idx]
            self.current_wave_idx += 1
            self._update_wave_list()
    
    def _add_enemy(self):
        if self.current_wave_idx < 0:
            messagebox.showinfo("Info", "Select a wave first")
            return
        
        enemy = EnemySpawnConfig(x=SPAWN_X, y=SCREEN_HEIGHT // 2)
        self.level.waves[self.current_wave_idx].enemies.append(enemy)
        self._update_enemy_list()
        self.canvas.refresh()
    
    def _remove_enemy(self):
        if self.current_wave_idx < 0:
            return
        
        wave = self.level.waves[self.current_wave_idx]
        if self.canvas.selected_enemy and self.canvas.selected_enemy in wave.enemies:
            wave.enemies.remove(self.canvas.selected_enemy)
            self.canvas.selected_enemy = None
            self.enemy_editor.set_enemy(None)
            self._update_enemy_list()
            self.canvas.refresh()
    
    def _duplicate_enemy(self):
        if self.current_wave_idx < 0 or not self.canvas.selected_enemy:
            return
        
        original = self.canvas.selected_enemy
        duplicate = EnemySpawnConfig(
            type=original.type,
            x=original.x,
            y=original.y + 50,  # Offset slightly
            vx=original.vx,
            vy=original.vy,
            health=original.health,
            scoreValue=original.scoreValue
        )
        self.level.waves[self.current_wave_idx].enemies.append(duplicate)
        self._update_enemy_list()
        self.canvas.refresh()
    
    def _new_level(self):
        if messagebox.askyesno("New Level", "Create a new level? Unsaved changes will be lost."):
            self.level = LevelConfig()
            self.current_wave_idx = -1
            self.current_file = None
            self._refresh_ui()
    
    def _open_level(self):
        filepath = filedialog.askopenfilename(
            title="Open Level",
            filetypes=[("JSON files", "*.json"), ("All files", "*.*")],
            initialdir="config/levels"
        )
        if filepath:
            try:
                self.level = LevelConfig.load(filepath)
                self.current_file = filepath
                self.current_wave_idx = -1
                self._refresh_ui()
                messagebox.showinfo("Success", f"Loaded: {os.path.basename(filepath)}")
            except Exception as e:
                messagebox.showerror("Error", f"Failed to load level:\n{e}")
    
    def _save_level(self):
        if self.current_file:
            try:
                self.level.save(self.current_file)
                messagebox.showinfo("Success", f"Saved: {os.path.basename(self.current_file)}")
            except Exception as e:
                messagebox.showerror("Error", f"Failed to save:\n{e}")
        else:
            self._save_level_as()
    
    def _save_level_as(self):
        filepath = filedialog.asksaveasfilename(
            title="Save Level",
            defaultextension=".json",
            filetypes=[("JSON files", "*.json"), ("All files", "*.*")],
            initialdir="config/levels"
        )
        if filepath:
            try:
                self.level.save(filepath)
                self.current_file = filepath
                messagebox.showinfo("Success", f"Saved: {os.path.basename(filepath)}")
            except Exception as e:
                messagebox.showerror("Error", f"Failed to save:\n{e}")
    
    def _refresh_ui(self):
        self.name_var.set(self.level.name)
        self.difficulty_var.set(self.level.difficulty)
        self.wave_delay_var.set(self.level.waveDelay)
        self._update_wave_list()
        self.wave_editor.set_wave(None)
        self.canvas.set_wave(None)
        self.enemy_listbox.delete(0, tk.END)
        self.enemy_editor.set_enemy(None)


def main():
    root = tk.Tk()
    root.title("R-Type Level Editor")
    root.geometry("1200x700")
    
    editor = LevelEditor(root)
    editor.pack(fill="both", expand=True)
    
    root.mainloop()


if __name__ == "__main__":
    main()
