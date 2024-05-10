import subprocess
import tkinter as tk
from tkinter import scrolledtext
import os

def run_c_program():
    algorithm = algorithm_var.get()

    if os.name == 'nt':  # Windows
        cmd = ['main.exe', algorithm]
    else:  # Unix-like
        cmd = ['./main', algorithm]

    process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    
    output_display.delete('1.0', tk.END)

    for line in iter(process.stdout.readline, ''):
        output_display.insert(tk.END, line)

    errors = process.stderr.read()
    if errors:
        output_display.insert(tk.END, f'Errors:\n{errors}')

root = tk.Tk()
root.title('Scheduling Algorithm Simulator')

algorithm_var = tk.StringVar(value='fcfs')  
algorithms = [('First-Come, First-Served', 'fcfs'), ('Shortest Job First', 'sjf'), ('Priority Preemptive', 'pp')]
for text, value in algorithms:
    tk.Radiobutton(root, text=text, variable=algorithm_var, value=value).pack(anchor=tk.W)

output_display = scrolledtext.ScrolledText(root, width=80, height=20)
output_display.pack(pady=20)

run_button = tk.Button(root, text='Run Selected Algorithm', command=run_c_program)
run_button.pack(pady=10)

root.mainloop()
