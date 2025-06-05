import tkinter as tk
from tkinter import ttk
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import threading
import time
import os
import ctypes
from ctypes import c_int, c_bool, c_uint, POINTER



# Archivos compartidos con el CDD
SELECT_FILE = "../signal_select.txt"
DATA_FILE = "../signal_data.txt"

class SignalApp:
    def __init__(self, root):
        script_dir = os.path.dirname(os.path.abspath(__file__))
        # Construye la ruta absoluta al archivo de la biblioteca
        lib_path = os.path.join(script_dir, "liboxcode.so")
        self.lib = ctypes.CDLL(lib_path)
        self.lib.switch_signal.argtypes = [c_uint]
        self.lib.switch_signal.restype = c_bool
        self.lib.read_signal_values.argtypes = []
        self.lib.read_signal_values.restype = c_bool

        self.root = root
        self.root.title("Visualización de Señales")

        self.signal_choice = tk.IntVar(value=1)
        self.data = []
        self.timestamps = []
        self.last_signal = 1

        # Configura UI
        self.create_widgets()

        # Inicia el hilo de actualización de datos
        self.running = True
        self.update_thread = threading.Thread(target=self.update_data_loop,daemon=True)
        self.update_thread.start()


        # Firmas de las funciones
        self.lib.init.restype = c_bool

        self.lib.cleanup.restype = None

    def create_widgets(self):
        # Selector de señal
        frame = ttk.Frame(self.root)
        frame.pack(pady=10)

        ttk.Label(frame, text="Seleccionar señal:").pack(side=tk.LEFT)

        ttk.Radiobutton(frame, text="Señal 1", variable=self.signal_choice, value=1, command=self.on_signal_change).pack(side=tk.LEFT, padx=5)
        ttk.Radiobutton(frame, text="Señal 2", variable=self.signal_choice, value=2, command=self.on_signal_change).pack(side=tk.LEFT, padx=5)

        # Gráfico
        self.fig, self.ax = plt.subplots(figsize=(6, 4))
        self.canvas = FigureCanvasTkAgg(self.fig, master=self.root)
        self.canvas.get_tk_widget().pack()
        self.ax.set_xlabel("Tiempo [s]")
        self.ax.set_ylabel("Valor de Señal")

        self.update_title()

    def update_title(self):
        tipo = self.signal_choice.get()
        self.ax.set_title(f"Señal {tipo} en función del tiempo")
        self.ax.set_xlabel("Tiempo [s]")
        self.ax.set_ylabel("Valor digital (0 o 1)")
        self.canvas.draw()

    def on_signal_change(self):
        signal = self.signal_choice.get()

        # Llamar al driver para cambiar la señal
        result= self.lib.switch_signal(signal -1)

        #print ("El driver devolvio:" +str(result))

        # Resetear gráfico
        self.data.clear()
        self.timestamps.clear()
        self.ax.cla()
        self.update_title()

    def leer_valor_de_cdd(self):
        #values = (c_int * 1)()  # Un array de 1 entero
        success = self.lib.read_signal_values()
        return success


    def update_data_loop(self):
        max_points = 100
        while self.running:
            value = self.leer_valor_de_cdd()
            print (value)

            current_time = time.time()
            self.data.append(value)
            self.timestamps.append(current_time)

            if len(self.data) > max_points:
                self.data = self.data[-max_points:]
                self.timestamps = self.timestamps[-max_points:]

            plot_timestamps = [ts - self.timestamps[0] for ts in self.timestamps]

            self.ax.cla()
            if plot_timestamps:
                self.ax.plot(plot_timestamps, self.data, marker='o', linestyle='-',
                             label=f"Señal {self.signal_choice.get()}")
            self.ax.set_ylim(-0.1, 1.1)
            self.update_title()
            if plot_timestamps:
                self.ax.legend(loc='upper left')
            self.canvas.draw()
            time.sleep(1)

    def on_close(self):
        self.running = False
        self.lib.cleanup()
        self.root.destroy()

if __name__ == "__main__":
    root = tk.Tk()
    app = SignalApp(root)
    root.protocol("WM_DELETE_WINDOW", app.on_close)
    root.mainloop()
