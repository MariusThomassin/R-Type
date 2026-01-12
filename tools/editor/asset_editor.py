"""
R-Type Asset Editor
Browser and editor for game assets (sprites, sounds, music)
"""

import tkinter as tk
from tkinter import ttk, filedialog, messagebox
from pathlib import Path
from typing import Optional, List
import os

try:
    from PIL import Image, ImageTk
    PIL_AVAILABLE = True
except ImportError:
    PIL_AVAILABLE = False
    print("Warning: Pillow not installed. Image preview will be limited.")

try:
    import pygame
    pygame.mixer.init()
    PYGAME_AVAILABLE = True
except ImportError:
    PYGAME_AVAILABLE = False
    print("Warning: pygame not installed. Audio playback not available.")


class SpriteViewer(ttk.Frame):
    """Sprite sheet viewer with frame selection"""
    
    def __init__(self, parent):
        super().__init__(parent)
        self.current_image: Optional[Image.Image] = None
        self.photo_image: Optional[ImageTk.PhotoImage] = None
        self.current_file: Optional[str] = None
        self.zoom = 1.0
        self.frame_width = 32
        self.frame_height = 32
        self.selected_frame = (0, 0)
        self._build_ui()
    
    def _build_ui(self):
        # Controls
        controls = ttk.Frame(self)
        controls.pack(fill="x", padx=5, pady=5)
        
        ttk.Button(controls, text="Open Sprite", command=self._open_sprite).pack(side="left", padx=2)
        
        ttk.Separator(controls, orient="vertical").pack(side="left", fill="y", padx=10)
        
        ttk.Label(controls, text="Frame W:").pack(side="left", padx=2)
        self.frame_w_var = tk.IntVar(value=32)
        self.frame_w_spin = ttk.Spinbox(controls, from_=1, to=512, 
                                         textvariable=self.frame_w_var, width=5)
        self.frame_w_spin.pack(side="left", padx=2)
        self.frame_w_spin.bind("<Return>", self._on_frame_size_change)
        self.frame_w_spin.bind("<FocusOut>", self._on_frame_size_change)
        
        ttk.Label(controls, text="H:").pack(side="left", padx=2)
        self.frame_h_var = tk.IntVar(value=32)
        self.frame_h_spin = ttk.Spinbox(controls, from_=1, to=512, 
                                         textvariable=self.frame_h_var, width=5)
        self.frame_h_spin.pack(side="left", padx=2)
        self.frame_h_spin.bind("<Return>", self._on_frame_size_change)
        self.frame_h_spin.bind("<FocusOut>", self._on_frame_size_change)
        
        ttk.Separator(controls, orient="vertical").pack(side="left", fill="y", padx=10)
        
        ttk.Label(controls, text="Zoom:").pack(side="left", padx=2)
        self.zoom_var = tk.DoubleVar(value=1.0)
        zoom_combo = ttk.Combobox(controls, textvariable=self.zoom_var, 
                                   values=[0.5, 1.0, 2.0, 3.0, 4.0], width=5)
        zoom_combo.pack(side="left", padx=2)
        zoom_combo.bind("<<ComboboxSelected>>", self._on_zoom_change)
        
        # Info label
        self.info_label = ttk.Label(controls, text="")
        self.info_label.pack(side="right", padx=10)
        
        # Canvas with scrollbars
        canvas_frame = ttk.Frame(self)
        canvas_frame.pack(fill="both", expand=True, padx=5, pady=5)
        
        self.canvas = tk.Canvas(canvas_frame, bg="#2a2a3a", highlightthickness=0)
        
        h_scroll = ttk.Scrollbar(canvas_frame, orient="horizontal", command=self.canvas.xview)
        v_scroll = ttk.Scrollbar(canvas_frame, orient="vertical", command=self.canvas.yview)
        
        self.canvas.configure(xscrollcommand=h_scroll.set, yscrollcommand=v_scroll.set)
        
        h_scroll.pack(side="bottom", fill="x")
        v_scroll.pack(side="right", fill="y")
        self.canvas.pack(side="left", fill="both", expand=True)
        
        self.canvas.bind("<Button-1>", self._on_canvas_click)
        
        # Selected frame info
        frame_info = ttk.Frame(self)
        frame_info.pack(fill="x", padx=5, pady=5)
        
        self.selected_label = ttk.Label(frame_info, text="Selected: None")
        self.selected_label.pack(side="left")
        
        ttk.Button(frame_info, text="Copy Frame Coords", 
                   command=self._copy_frame_coords).pack(side="right", padx=2)
    
    def _open_sprite(self):
        if not PIL_AVAILABLE:
            messagebox.showerror("Error", "Pillow library required for image viewing.\n"
                                 "Install with: pip install Pillow")
            return
        
        filepath = filedialog.askopenfilename(
            title="Open Sprite Sheet",
            filetypes=[
                ("Image files", "*.png *.gif *.bmp *.jpg *.jpeg"),
                ("All files", "*.*")
            ],
            initialdir="assets/sprites"
        )
        if filepath:
            self._load_image(filepath)
    
    def _load_image(self, filepath: str):
        if not PIL_AVAILABLE:
            return
        
        try:
            self.current_image = Image.open(filepath)
            self.current_file = filepath
            self._refresh_display()
            
            # Update info
            w, h = self.current_image.size
            self.info_label.config(text=f"{os.path.basename(filepath)} - {w}x{h}")
        except Exception as e:
            messagebox.showerror("Error", f"Failed to load image:\n{e}")
    
    def _refresh_display(self):
        if not self.current_image or not PIL_AVAILABLE:
            return
        
        self.frame_width = self.frame_w_var.get()
        self.frame_height = self.frame_h_var.get()
        self.zoom = self.zoom_var.get()
        
        # Scale image
        w, h = self.current_image.size
        new_w = int(w * self.zoom)
        new_h = int(h * self.zoom)
        
        scaled = self.current_image.resize((new_w, new_h), Image.Resampling.NEAREST)
        self.photo_image = ImageTk.PhotoImage(scaled)
        
        # Update canvas
        self.canvas.delete("all")
        self.canvas.create_image(0, 0, anchor="nw", image=self.photo_image)
        
        # Draw grid
        cell_w = int(self.frame_width * self.zoom)
        cell_h = int(self.frame_height * self.zoom)
        
        for x in range(0, new_w + 1, cell_w):
            self.canvas.create_line(x, 0, x, new_h, fill="#666666", dash=(2, 2))
        for y in range(0, new_h + 1, cell_h):
            self.canvas.create_line(0, y, new_w, y, fill="#666666", dash=(2, 2))
        
        # Highlight selected frame
        if self.selected_frame:
            fx, fy = self.selected_frame
            x1 = fx * cell_w
            y1 = fy * cell_h
            self.canvas.create_rectangle(x1, y1, x1 + cell_w, y1 + cell_h,
                                          outline="#00FF00", width=2)
        
        self.canvas.configure(scrollregion=(0, 0, new_w, new_h))
    
    def _on_frame_size_change(self, event=None):
        self._refresh_display()
    
    def _on_zoom_change(self, event=None):
        self._refresh_display()
    
    def _on_canvas_click(self, event):
        if not self.current_image:
            return
        
        # Get canvas coordinates
        x = self.canvas.canvasx(event.x)
        y = self.canvas.canvasy(event.y)
        
        # Calculate frame index
        cell_w = int(self.frame_width * self.zoom)
        cell_h = int(self.frame_height * self.zoom)
        
        if cell_w > 0 and cell_h > 0:
            fx = int(x // cell_w)
            fy = int(y // cell_h)
            self.selected_frame = (fx, fy)
            self.selected_label.config(text=f"Selected: Frame ({fx}, {fy}) - "
                                       f"Pixel ({fx * self.frame_width}, {fy * self.frame_height})")
            self._refresh_display()
    
    def _copy_frame_coords(self):
        if self.selected_frame:
            fx, fy = self.selected_frame
            text = f"frameX={fx * self.frame_width}, frameY={fy * self.frame_height}, " \
                   f"frameWidth={self.frame_width}, frameHeight={self.frame_height}"
            self.clipboard_clear()
            self.clipboard_append(text)
            self.selected_label.config(text=f"Copied: {text}")


class AudioPlayer(ttk.Frame):
    """Audio file browser and player"""
    
    def __init__(self, parent):
        super().__init__(parent)
        self.current_file: Optional[str] = None
        self.is_playing = False
        self._build_ui()
    
    def _build_ui(self):
        # Controls
        controls = ttk.Frame(self)
        controls.pack(fill="x", padx=5, pady=5)
        
        self.play_btn = ttk.Button(controls, text="▶ Play", command=self._toggle_play)
        self.play_btn.pack(side="left", padx=2)
        
        ttk.Button(controls, text="⏹ Stop", command=self._stop).pack(side="left", padx=2)
        
        ttk.Separator(controls, orient="vertical").pack(side="left", fill="y", padx=10)
        
        ttk.Label(controls, text="Volume:").pack(side="left", padx=2)
        self.volume_var = tk.DoubleVar(value=0.7)
        volume_scale = ttk.Scale(controls, from_=0, to=1, variable=self.volume_var,
                                  orient="horizontal", length=100, command=self._on_volume_change)
        volume_scale.pack(side="left", padx=2)
        
        self.file_label = ttk.Label(controls, text="No file loaded")
        self.file_label.pack(side="right", padx=10)
        
        # File browser
        browser_frame = ttk.Frame(self)
        browser_frame.pack(fill="both", expand=True, padx=5, pady=5)
        
        # Tree view for files
        columns = ("name", "type", "size")
        self.tree = ttk.Treeview(browser_frame, columns=columns, show="tree headings")
        self.tree.heading("#0", text="Path")
        self.tree.heading("name", text="Name")
        self.tree.heading("type", text="Type")
        self.tree.heading("size", text="Size")
        
        self.tree.column("#0", width=200)
        self.tree.column("name", width=200)
        self.tree.column("type", width=80)
        self.tree.column("size", width=80)
        
        scrollbar = ttk.Scrollbar(browser_frame, orient="vertical", command=self.tree.yview)
        self.tree.configure(yscrollcommand=scrollbar.set)
        
        scrollbar.pack(side="right", fill="y")
        self.tree.pack(side="left", fill="both", expand=True)
        
        self.tree.bind("<<TreeviewSelect>>", self._on_select)
        self.tree.bind("<Double-1>", self._on_double_click)
        
        # Load initial files
        self._refresh_files()
    
    def _refresh_files(self):
        self.tree.delete(*self.tree.get_children())
        
        sound_dir = Path("assets/sound")
        if not sound_dir.exists():
            sound_dir = Path("../assets/sound")
        
        if sound_dir.exists():
            self._add_directory(sound_dir, "")
    
    def _add_directory(self, path: Path, parent: str):
        dir_id = self.tree.insert(parent, "end", text=path.name, 
                                   values=("", "Folder", ""))
        
        try:
            for item in sorted(path.iterdir()):
                if item.is_dir():
                    self._add_directory(item, dir_id)
                elif item.suffix.lower() in ('.wav', '.ogg', '.mp3', '.flac'):
                    size = item.stat().st_size
                    size_str = f"{size / 1024:.1f} KB" if size < 1024 * 1024 else f"{size / 1024 / 1024:.1f} MB"
                    self.tree.insert(dir_id, "end", text=str(item), 
                                      values=(item.name, item.suffix[1:].upper(), size_str))
        except PermissionError:
            pass
    
    def _on_select(self, event):
        selection = self.tree.selection()
        if selection:
            item = selection[0]
            filepath = self.tree.item(item, "text")
            if Path(filepath).is_file():
                self.current_file = filepath
                self.file_label.config(text=os.path.basename(filepath))
    
    def _on_double_click(self, event):
        if self.current_file:
            self._play()
    
    def _toggle_play(self):
        if self.is_playing:
            self._pause()
        else:
            self._play()
    
    def _play(self):
        if not PYGAME_AVAILABLE:
            messagebox.showinfo("Info", "pygame required for audio playback.\n"
                               "Install with: pip install pygame")
            return
        
        if not self.current_file:
            messagebox.showinfo("Info", "Select an audio file first")
            return
        
        try:
            pygame.mixer.music.load(self.current_file)
            pygame.mixer.music.set_volume(self.volume_var.get())
            pygame.mixer.music.play()
            self.is_playing = True
            self.play_btn.config(text="⏸ Pause")
        except Exception as e:
            messagebox.showerror("Error", f"Failed to play audio:\n{e}")
    
    def _pause(self):
        if PYGAME_AVAILABLE:
            pygame.mixer.music.pause()
            self.is_playing = False
            self.play_btn.config(text="▶ Play")
    
    def _stop(self):
        if PYGAME_AVAILABLE:
            pygame.mixer.music.stop()
            self.is_playing = False
            self.play_btn.config(text="▶ Play")
    
    def _on_volume_change(self, value):
        if PYGAME_AVAILABLE:
            pygame.mixer.music.set_volume(float(value))


class AssetBrowser(ttk.Frame):
    """General asset browser with preview"""
    
    def __init__(self, parent):
        super().__init__(parent)
        self._build_ui()
    
    def _build_ui(self):
        # Toolbar
        toolbar = ttk.Frame(self)
        toolbar.pack(fill="x", padx=5, pady=5)
        
        ttk.Button(toolbar, text="Refresh", command=self._refresh).pack(side="left", padx=2)
        ttk.Button(toolbar, text="Open in Explorer", command=self._open_folder).pack(side="left", padx=2)
        
        # Paned window
        pane = ttk.PanedWindow(self, orient="horizontal")
        pane.pack(fill="both", expand=True, padx=5, pady=5)
        
        # Tree view
        tree_frame = ttk.Frame(pane)
        pane.add(tree_frame, weight=1)
        
        self.tree = ttk.Treeview(tree_frame, show="tree")
        scrollbar = ttk.Scrollbar(tree_frame, orient="vertical", command=self.tree.yview)
        self.tree.configure(yscrollcommand=scrollbar.set)
        
        scrollbar.pack(side="right", fill="y")
        self.tree.pack(side="left", fill="both", expand=True)
        
        self.tree.bind("<<TreeviewSelect>>", self._on_select)
        
        # Preview panel
        preview_frame = ttk.Frame(pane)
        pane.add(preview_frame, weight=2)
        
        ttk.Label(preview_frame, text="Preview", font=("Arial", 10, "bold")).pack(anchor="w")
        
        self.preview_canvas = tk.Canvas(preview_frame, bg="#2a2a3a", width=300, height=300)
        self.preview_canvas.pack(fill="both", expand=True, pady=5)
        
        self.preview_info = ttk.Label(preview_frame, text="Select a file to preview")
        self.preview_info.pack(anchor="w")
        
        self._refresh()
    
    def _refresh(self):
        self.tree.delete(*self.tree.get_children())
        
        assets_dir = Path("assets")
        if not assets_dir.exists():
            assets_dir = Path("../assets")
        
        if assets_dir.exists():
            self._add_directory(assets_dir, "")
    
    def _add_directory(self, path: Path, parent: str):
        dir_id = self.tree.insert(parent, "end", text=path.name)
        
        try:
            for item in sorted(path.iterdir()):
                if item.is_dir():
                    self._add_directory(item, dir_id)
                else:
                    self.tree.insert(dir_id, "end", text=item.name, 
                                      values=(str(item),))
        except PermissionError:
            pass
    
    def _on_select(self, event):
        selection = self.tree.selection()
        if not selection:
            return
        
        # Get full path
        item = selection[0]
        path_parts = []
        current = item
        while current:
            path_parts.insert(0, self.tree.item(current, "text"))
            current = self.tree.parent(current)
        
        filepath = Path("/".join(path_parts))
        if not filepath.exists():
            filepath = Path("..") / "/".join(path_parts)
        
        if filepath.is_file():
            self._preview_file(filepath)
    
    def _preview_file(self, filepath: Path):
        self.preview_canvas.delete("all")
        
        suffix = filepath.suffix.lower()
        
        if suffix in ('.png', '.gif', '.bmp', '.jpg', '.jpeg'):
            if PIL_AVAILABLE:
                try:
                    img = Image.open(filepath)
                    # Resize to fit canvas
                    canvas_w = self.preview_canvas.winfo_width() or 300
                    canvas_h = self.preview_canvas.winfo_height() or 300
                    
                    ratio = min(canvas_w / img.width, canvas_h / img.height, 1.0)
                    new_size = (int(img.width * ratio), int(img.height * ratio))
                    
                    if ratio < 1:
                        img = img.resize(new_size, Image.Resampling.NEAREST)
                    
                    self._preview_photo = ImageTk.PhotoImage(img)
                    self.preview_canvas.create_image(canvas_w // 2, canvas_h // 2,
                                                      image=self._preview_photo)
                    self.preview_info.config(text=f"{filepath.name} - {img.width}x{img.height}")
                except Exception as e:
                    self.preview_info.config(text=f"Error: {e}")
            else:
                self.preview_info.config(text="Pillow required for image preview")
        
        elif suffix in ('.wav', '.ogg', '.mp3', '.flac'):
            size = filepath.stat().st_size
            size_str = f"{size / 1024:.1f} KB" if size < 1024 * 1024 else f"{size / 1024 / 1024:.1f} MB"
            self.preview_canvas.create_text(150, 150, text="♪ Audio File ♪",
                                             font=("Arial", 16), fill="#888888")
            self.preview_info.config(text=f"{filepath.name} - {size_str}")
        
        elif suffix == '.json':
            try:
                content = filepath.read_text()[:500]
                self.preview_canvas.create_text(10, 10, text=content,
                                                 font=("Courier", 10), fill="#888888",
                                                 anchor="nw", width=280)
                self.preview_info.config(text=f"{filepath.name}")
            except Exception as e:
                self.preview_info.config(text=f"Error: {e}")
        
        else:
            self.preview_canvas.create_text(150, 150, text="No preview",
                                             font=("Arial", 14), fill="#666666")
            self.preview_info.config(text=filepath.name)
    
    def _open_folder(self):
        assets_dir = Path("assets").absolute()
        if not assets_dir.exists():
            assets_dir = Path("../assets").absolute()
        
        if assets_dir.exists():
            os.startfile(assets_dir) if os.name == 'nt' else os.system(f'xdg-open "{assets_dir}"')


class AssetEditor(ttk.Frame):
    """Main asset editor with tabs"""
    
    def __init__(self, parent):
        super().__init__(parent)
        self._build_ui()
    
    def _build_ui(self):
        notebook = ttk.Notebook(self)
        notebook.pack(fill="both", expand=True)
        
        # Sprite viewer tab
        self.sprite_viewer = SpriteViewer(notebook)
        notebook.add(self.sprite_viewer, text="Sprite Viewer")
        
        # Audio player tab
        self.audio_player = AudioPlayer(notebook)
        notebook.add(self.audio_player, text="Audio Player")
        
        # Asset browser tab
        self.asset_browser = AssetBrowser(notebook)
        notebook.add(self.asset_browser, text="Asset Browser")


def main():
    root = tk.Tk()
    root.title("R-Type Asset Editor")
    root.geometry("1000x700")
    
    editor = AssetEditor(root)
    editor.pack(fill="both", expand=True)
    
    root.mainloop()


if __name__ == "__main__":
    main()
