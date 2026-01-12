#!/usr/bin/env python3
"""
R-Type Editor Suite
Combined level editor and asset editor for R-Type game development
"""

import tkinter as tk
from tkinter import ttk
import sys
import os

# Add the editor directory to path for imports
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from level_editor import LevelEditor
from asset_editor import AssetEditor


class EditorApp(tk.Tk):
    """Main application window with tabbed editors"""
    
    def __init__(self):
        super().__init__()
        
        self.title("R-Type Editor Suite")
        self.geometry("1300x800")
        
        # Set dark theme
        self.style = ttk.Style()
        self._configure_theme()
        
        # Create notebook for tabs
        self.notebook = ttk.Notebook(self)
        self.notebook.pack(fill="both", expand=True, padx=5, pady=5)
        
        # Level editor tab
        self.level_editor = LevelEditor(self.notebook)
        self.notebook.add(self.level_editor, text="Level Editor")
        
        # Asset editor tab
        self.asset_editor = AssetEditor(self.notebook)
        self.notebook.add(self.asset_editor, text="Asset Editor")
        
        # Status bar
        self.status_bar = ttk.Label(self, text="Ready", relief="sunken", anchor="w")
        self.status_bar.pack(fill="x", side="bottom")
        
        # Menu bar
        self._create_menu()
        
        # Set window icon if available
        try:
            # Try to load an icon from assets
            pass  # Icon loading would go here
        except:
            pass
    
    def _configure_theme(self):
        """Configure a dark theme for the editor"""
        try:
            self.style.theme_use('clam')
        except:
            pass
        
        # Configure colors
        bg_color = "#2d2d3d"
        fg_color = "#ffffff"
        select_bg = "#4a4a6a"
        
        self.configure(bg=bg_color)
        
        self.style.configure(".", background=bg_color, foreground=fg_color)
        self.style.configure("TFrame", background=bg_color)
        self.style.configure("TLabel", background=bg_color, foreground=fg_color)
        self.style.configure("TButton", background="#3d3d5d", foreground=fg_color)
        self.style.configure("TNotebook", background=bg_color)
        self.style.configure("TNotebook.Tab", background="#3d3d5d", foreground=fg_color,
                            padding=[10, 5])
        self.style.map("TNotebook.Tab", 
                      background=[("selected", "#4a4a6a")],
                      foreground=[("selected", fg_color)])
        self.style.configure("TEntry", fieldbackground="#3d3d5d", foreground=fg_color)
        self.style.configure("TSpinbox", fieldbackground="#3d3d5d", foreground=fg_color)
        self.style.configure("TCombobox", fieldbackground="#3d3d5d", foreground=fg_color)
        self.style.configure("Treeview", background="#3d3d5d", foreground=fg_color,
                            fieldbackground="#3d3d5d")
        self.style.configure("Treeview.Heading", background="#4a4a6a", foreground=fg_color)
    
    def _create_menu(self):
        """Create the menu bar"""
        menubar = tk.Menu(self)
        self.config(menu=menubar)
        
        # File menu
        file_menu = tk.Menu(menubar, tearoff=0)
        menubar.add_cascade(label="File", menu=file_menu)
        file_menu.add_command(label="New Level", command=self._new_level)
        file_menu.add_command(label="Open Level...", command=self._open_level)
        file_menu.add_command(label="Save Level", command=self._save_level)
        file_menu.add_command(label="Save Level As...", command=self._save_level_as)
        file_menu.add_separator()
        file_menu.add_command(label="Exit", command=self.quit)
        
        # Edit menu
        edit_menu = tk.Menu(menubar, tearoff=0)
        menubar.add_cascade(label="Edit", menu=edit_menu)
        edit_menu.add_command(label="Add Wave", command=self._add_wave)
        edit_menu.add_command(label="Add Enemy", command=self._add_enemy)
        
        # View menu
        view_menu = tk.Menu(menubar, tearoff=0)
        menubar.add_cascade(label="View", menu=view_menu)
        view_menu.add_command(label="Level Editor", command=lambda: self.notebook.select(0))
        view_menu.add_command(label="Asset Editor", command=lambda: self.notebook.select(1))
        
        # Help menu
        help_menu = tk.Menu(menubar, tearoff=0)
        menubar.add_cascade(label="Help", menu=help_menu)
        help_menu.add_command(label="About", command=self._show_about)
    
    def _new_level(self):
        self.notebook.select(0)
        self.level_editor._new_level()
    
    def _open_level(self):
        self.notebook.select(0)
        self.level_editor._open_level()
    
    def _save_level(self):
        self.level_editor._save_level()
    
    def _save_level_as(self):
        self.level_editor._save_level_as()
    
    def _add_wave(self):
        self.notebook.select(0)
        self.level_editor._add_wave()
    
    def _add_enemy(self):
        self.notebook.select(0)
        self.level_editor._add_enemy()
    
    def _show_about(self):
        tk.messagebox.showinfo(
            "About R-Type Editor",
            "R-Type Editor Suite v1.0\n\n"
            "Level Editor:\n"
            "  - Create and edit level JSON files\n"
            "  - Visual wave and enemy placement\n"
            "  - Drag-and-drop enemy positioning\n\n"
            "Asset Editor:\n"
            "  - Browse sprite sheets\n"
            "  - Preview and play audio files\n"
            "  - Manage game assets\n\n"
            "For R-Type ECS Game"
        )
    
    def set_status(self, message: str):
        """Update the status bar message"""
        self.status_bar.config(text=message)


def main():
    # Change to project root directory if running from tools/editor
    if os.path.basename(os.getcwd()) == "editor":
        os.chdir("../..")
    elif os.path.basename(os.getcwd()) == "tools":
        os.chdir("..")
    
    app = EditorApp()
    app.mainloop()


if __name__ == "__main__":
    main()
