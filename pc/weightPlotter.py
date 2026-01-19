import tkinter as tk
from tkinter import ttk, messagebox
import serial
from serial.tools import list_ports

import matplotlib
matplotlib.use("TkAgg")
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure


class UartPlotApp(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("UART Weight Plotter")
        self.geometry("900x700")

        # Serial state
        self.ser = None
        self.rx_buf = ""

        # Data for plot
        self.x = []
        self.y = []
        self.sample_idx = 0
        self.max_points = 2000

        # Tracking values (stored in grams)
        self.last_value_g = None
        self.max_value_g = None

        self._build_ui()
        self.refresh_ports()
        self._update_plot()

        self.protocol("WM_DELETE_WINDOW", self.on_close)

    # ---------- UI ----------
    def _build_ui(self):
        top = ttk.Frame(self)
        top.pack(side=tk.TOP, fill=tk.X, padx=10, pady=10)

        ttk.Label(top, text="Port:").pack(side=tk.LEFT)
        self.port_var = tk.StringVar()
        self.port_combo = ttk.Combobox(
            top, textvariable=self.port_var, width=20, state="readonly"
        )
        self.port_combo.pack(side=tk.LEFT, padx=(5, 10))

        ttk.Button(top, text="Refresh", command=self.refresh_ports)\
            .pack(side=tk.LEFT, padx=(0, 10))

        ttk.Label(top, text="Baud:").pack(side=tk.LEFT)
        self.baud_var = tk.StringVar(value="115200")
        self.baud_entry = ttk.Entry(top, textvariable=self.baud_var, width=10)
        self.baud_entry.pack(side=tk.LEFT, padx=(5, 10))

        self.connect_btn = ttk.Button(top, text="Connect", command=self.toggle_connect)
        self.connect_btn.pack(side=tk.LEFT)

        self.status_var = tk.StringVar(value="Disconnected")
        ttk.Label(top, textvariable=self.status_var).pack(side=tk.LEFT, padx=15)

        # ===== LAST / MAX displays =====
        values_frame = ttk.Frame(self)
        values_frame.pack(side=tk.TOP, fill=tk.X, padx=10, pady=(0, 5))
        values_frame.columnconfigure(0, weight=1)
        values_frame.columnconfigure(1, weight=1)

        left = ttk.Frame(values_frame)
        left.grid(row=0, column=0, sticky="nsew", padx=(0, 8))
        right = ttk.Frame(values_frame)
        right.grid(row=0, column=1, sticky="nsew", padx=(8, 0))

        ttk.Label(left, text="LAST", anchor="center").pack(fill=tk.X)
        self.last_value_var = tk.StringVar(value="--- kg")
        ttk.Label(
            left,
            textvariable=self.last_value_var,
            font=("Consolas", 30, "bold"),
            anchor="center"
        ).pack(fill=tk.X, pady=(0, 4))

        ttk.Label(right, text="MAX", anchor="center").pack(fill=tk.X)
        self.max_value_var = tk.StringVar(value="--- kg")
        ttk.Label(
            right,
            textvariable=self.max_value_var,
            font=("Consolas", 30, "bold"),
            anchor="center"
        ).pack(fill=tk.X, pady=(0, 4))

        ttk.Button(right, text="Reset MAX", command=self.reset_max).pack()

        # ===== Plot =====
        fig = Figure(figsize=(7, 4), dpi=100)
        self.ax = fig.add_subplot(111)
        self.ax.set_title("Weight (g) vs sample")
        self.ax.set_xlabel("Sample")
        self.ax.set_ylabel("Weight, g")
        self.line, = self.ax.plot([], [], linewidth=1)

        self.canvas = FigureCanvasTkAgg(fig, master=self)
        self.canvas.get_tk_widget().pack(
            side=tk.TOP, fill=tk.BOTH, expand=True, padx=10, pady=10
        )

        bottom = ttk.Frame(self)
        bottom.pack(side=tk.BOTTOM, fill=tk.X, padx=10, pady=(0, 10))
        ttk.Button(bottom, text="Clear", command=self.clear_plot).pack(side=tk.LEFT)

    # ---------- Serial ----------
    def refresh_ports(self):
        ports = [p.device for p in list_ports.comports()]
        self.port_combo["values"] = ports
        self.port_var.set(ports[0] if ports else "")

    def toggle_connect(self):
        if self.ser and self.ser.is_open:
            self.disconnect()
        else:
            self.connect()

    def connect(self):
        port = self.port_var.get().strip()
        if not port:
            messagebox.showerror("Error", "No COM port selected.")
            return

        try:
            baud = int(self.baud_var.get().strip() or "115200")
        except ValueError:
            messagebox.showerror("Error", "Invalid baud rate.")
            return

        try:
            self.ser = serial.Serial(port, baudrate=baud, timeout=0)
            self.rx_buf = ""
            self.status_var.set(f"Connected: {port} @ {baud}")
            self.connect_btn.config(text="Disconnect")
            self.after(20, self.poll_uart)
        except Exception as e:
            messagebox.showerror("Error", f"Failed to open port:\n{e}")

    def disconnect(self):
        if self.ser:
            try:
                self.ser.close()
            except Exception:
                pass
        self.ser = None
        self.status_var.set("Disconnected")
        self.connect_btn.config(text="Connect")

    def clear_plot(self):
        self.x.clear()
        self.y.clear()
        self.sample_idx = 0
        self.last_value_g = None
        self.max_value_g = None
        self.last_value_var.set("--- kg")
        self.max_value_var.set("--- kg")
        self._update_plot()

    def reset_max(self):
        self.max_value_g = None
        self.max_value_var.set("--- kg")

    # ---------- UART ----------
    def poll_uart(self):
        if not (self.ser and self.ser.is_open):
            return

        try:
            n = self.ser.in_waiting
            if n:
                chunk = self.ser.read(n).decode("utf-8", errors="ignore")
                self.rx_buf += chunk
                if self._consume_values():
                    self._update_plot()
        except Exception as e:
            self.disconnect()
            messagebox.showerror("UART error", str(e))
            return

        self.after(20, self.poll_uart)

    def _consume_values(self):
        updated = False
        parts = self.rx_buf.split(";")
        self.rx_buf = parts[-1]

        for token in parts[:-1]:
            token = token.strip()
            if not token:
                continue
            try:
                grams = float(token)
            except ValueError:
                continue

            # plot data (grams)
            self.x.append(self.sample_idx)
            self.y.append(grams)
            self.sample_idx += 1
            updated = True

            # LAST
            self.last_value_g = grams
            self.last_value_var.set(f"{grams / 1000.0:.2f} kg")

            # MAX
            if self.max_value_g is None or grams > self.max_value_g:
                self.max_value_g = grams
                self.max_value_var.set(f"{self.max_value_g / 1000.0:.2f} kg")

            if len(self.x) > self.max_points:
                self.x = self.x[-self.max_points:]
                self.y = self.y[-self.max_points:]

        return updated

    # ---------- Plot ----------
    def _update_plot(self):
        self.line.set_data(self.x, self.y)
        self.ax.relim()
        self.ax.autoscale_view()
        self.canvas.draw_idle()

    def on_close(self):
        self.disconnect()
        self.destroy()


if __name__ == "__main__":
    app = UartPlotApp()
    app.mainloop()
