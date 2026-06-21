import json
import tkinter as tk
import tkinter.font as tkfont
from pathlib import Path
from tkinter import messagebox, ttk


TASKS_FILE = Path(__file__).with_name("tasks.json")
README_FILE = Path(__file__).parent.parent / "TODO.md"


class TodoApp:
    def __init__(self, root: tk.Tk) -> None:
        self.root = root
        self.root.title("Direct3D 12 toy engine - TODO")
        self.root.geometry("520x560")
        self.root.minsize(420, 360)

        self.tasks: list[str] = self.load_tasks()

        tkfont.nametofont("TkDefaultFont").configure(family="Microsoft YaHei", size=12)

        self.build_ui()
        self.refresh_tasks()

        self.root.protocol("WM_DELETE_WINDOW", self.close)

    def build_ui(self) -> None:
        self.root.columnconfigure(0, weight=1)
        self.root.rowconfigure(1, weight=1)

        input_bar = ttk.Frame(self.root, padding=(12, 12, 12, 6))
        input_bar.grid(row=0, column=0, sticky="ew")
        input_bar.columnconfigure(0, weight=1)

        self.task_entry = ttk.Entry(input_bar)
        self.task_entry.grid(row=0, column=0, sticky="ew")
        self.task_entry.bind("<Return>", self.add_task)
        self.task_entry.focus_set()

        add_button = ttk.Button(input_bar, text="新建任务", command=self.add_task)
        add_button.grid(row=0, column=1, padx=(8, 0))

        list_area = ttk.Frame(self.root, padding=(12, 0, 12, 0))
        list_area.grid(row=1, column=0, sticky="nsew")
        list_area.columnconfigure(0, weight=1)
        list_area.rowconfigure(0, weight=1)

        self.canvas = tk.Canvas(list_area, highlightthickness=0)
        scrollbar = ttk.Scrollbar(list_area, orient=tk.VERTICAL, command=self.canvas.yview)
        self.canvas.configure(yscrollcommand=scrollbar.set)

        self.task_frame = ttk.Frame(self.canvas)
        self.canvas_window = self.canvas.create_window((0, 0), window=self.task_frame, anchor="nw")

        self.canvas.grid(row=0, column=0, sticky="nsew")
        scrollbar.grid(row=0, column=1, sticky="ns")

        self.task_frame.bind("<Configure>", self.update_scroll_region)
        self.canvas.bind("<Configure>", self.resize_task_frame)
        self.canvas.bind_all("<MouseWheel>", self.on_mouse_wheel)

        footer = ttk.Frame(self.root, padding=(12, 8, 12, 12))
        footer.grid(row=2, column=0, sticky="ew")
        footer.columnconfigure(0, weight=1)

        self.status_label = ttk.Label(footer)
        self.status_label.grid(row=0, column=0, sticky="w")

        clear_button = ttk.Button(footer, text="清空全部", command=self.clear_all)
        clear_button.grid(row=0, column=1, sticky="e")

    def load_tasks(self) -> list[str]:
        if not TASKS_FILE.exists():
            return []

        try:
            data = json.loads(TASKS_FILE.read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError):
            messagebox.showwarning("读取失败", f"无法读取 {TASKS_FILE.name}，将从空列表开始。")
            return []

        tasks: list[str] = []
        if isinstance(data, list):
            for item in data:
                if isinstance(item, str):
                    text = item.strip()
                    if text:
                        tasks.append(text)
                elif isinstance(item, dict) and not item.get("done", False):
                    text = str(item.get("text", "")).strip()
                    if text:
                        tasks.append(text)
        return tasks

    def save_tasks(self) -> None:
        TASKS_FILE.write_text(
            json.dumps(self.tasks, ensure_ascii=False, indent=2),
            encoding="utf-8",
        )
        self.update_readme()

    def update_readme(self) -> None:
        if not README_FILE.exists():
            return

        readme_text = README_FILE.read_text(encoding="utf-8")

        begin_marker = "<!-- TODO Begin -->"
        end_marker = "<!-- TODO End -->"

        if begin_marker not in readme_text or end_marker not in readme_text:
            return

        before, _ = readme_text.split(begin_marker, 1)
        _, after = readme_text.split(end_marker, 1)

        if self.tasks:
            lines = [f"- {task}" for task in self.tasks]
            tasks_block = "\n".join(lines)
            new_readme = f"{before}{begin_marker}\n{tasks_block}\n{end_marker}{after}"
        else:
            new_readme = f"{before}{begin_marker}\n{end_marker}{after}"

        README_FILE.write_text(new_readme, encoding="utf-8")

    def add_task(self, event: tk.Event | None = None) -> None:
        text = self.task_entry.get().strip()
        if not text:
            return

        self.tasks.append(text)
        self.task_entry.delete(0, tk.END)
        self.save_tasks()
        self.refresh_tasks()

    def complete_task(self, index: int) -> None:
        if not 0 <= index < len(self.tasks):
            return

        del self.tasks[index]
        self.save_tasks()
        self.refresh_tasks()

    def clear_all(self) -> None:
        if not self.tasks:
            return

        if messagebox.askyesno("确认", "确定要清空所有未完成任务吗？"):
            self.tasks.clear()
            self.save_tasks()
            self.refresh_tasks()

    def refresh_tasks(self) -> None:
        for child in self.task_frame.winfo_children():
            child.destroy()

        if not self.tasks:
            empty_label = ttk.Label(
                self.task_frame,
                text="暂无任务。输入一条任务后按 Enter 或点击“新建任务”。",
                foreground="#666666",
                padding=(4, 10),
            )
            empty_label.pack(anchor="w", fill=tk.X)
        else:
            for index, text in enumerate(self.tasks):
                row = ttk.Frame(self.task_frame, padding=(0, 3))
                row.pack(fill=tk.X)
                row.columnconfigure(1, weight=1)

                done = tk.BooleanVar(value=False)
                checkbox = ttk.Checkbutton(
                    row,
                    variable=done,
                    command=lambda task_index=index: self.complete_task(task_index),
                )
                checkbox.grid(row=0, column=0, sticky="n", padx=(0, 6))

                label = ttk.Label(row, text=text, wraplength=430, justify=tk.LEFT)
                label.grid(row=0, column=1, sticky="ew")

        count = len(self.tasks)
        self.status_label.configure(text=f"未完成任务：{count}")

    def update_scroll_region(self, event: tk.Event | None = None) -> None:
        self.canvas.configure(scrollregion=self.canvas.bbox("all"))

    def resize_task_frame(self, event: tk.Event) -> None:
        self.canvas.itemconfigure(self.canvas_window, width=event.width)
        wraplength = max(220, event.width - 52)
        for row in self.task_frame.winfo_children():
            for child in row.winfo_children():
                if isinstance(child, ttk.Label):
                    child.configure(wraplength=wraplength)

    def on_mouse_wheel(self, event: tk.Event) -> None:
        if self.canvas.winfo_containing(event.x_root, event.y_root) is None:
            return
        delta = int(-1 * (event.delta / 120))
        y0, y1 = self.canvas.yview()
        if (delta < 0 and y0 <= 0.0) or (delta > 0 and y1 >= 1.0):
            return
        self.canvas.yview_scroll(delta, "units")

    def close(self) -> None:
        self.save_tasks()
        self.root.destroy()


if __name__ == "__main__":
    window = tk.Tk()
    app = TodoApp(window)
    window.mainloop()
