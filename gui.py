import subprocess
import tkinter as tk
from tkinter import scrolledtext
import os
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
from datetime import datetime
import time

def parse_output(output):
    data = []
    lines = output.split("\n")
    current_time = 0
    for line in lines:
        if "Current Time:" in line:
            # Capture the current time from the output
            parts = line.split()
            current_time = float(parts[3])
        if "Executing process with PID" in line:
            # Parse the process details
            parts = line.split(',')
            pid = int(parts[0].split()[-1])
            burst_time = int(parts[1].split()[-1])
            # Append the data tuple for Gantt chart
            data.append((pid, burst_time, current_time))
    return data


def plot_gantt_chart(data):
    if not data:
        print("No data to plot.")
        return

    fig, ax = plt.subplots()
    yticks = []
    yticklabels = []

    for i, (pid, burst, start_time) in enumerate(data):
        ax.broken_barh([(start_time, burst)], (i-0.4, 0.8), facecolors='tab:blue')
        yticks.append(i)
        yticklabels.append(f'PID {pid}')

    ax.set_yticks(yticks)
    ax.set_yticklabels(yticklabels)
    ax.set_xlabel('Time since start (seconds)')
    ax.set_ylabel('Processes')
    ax.set_title('Process Execution Gantt Chart')
    ax.xaxis.set_major_locator(plt.MultipleLocator(1))  # Set major ticks to be at every integer
    ax.xaxis.set_major_formatter(plt.FuncFormatter(lambda x, _: f'{int(x):d}'))  # Format x-axis ticks as integers

    plt.show()


def run_c_program():
    algorithm = algorithm_var.get()

    if os.name == 'nt':  # Windows
        cmd = ['main.exe', algorithm]
    else:  # Unix-like
        cmd = ['./main', algorithm]

    process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, bufsize=1, universal_newlines=True)

    output_display.delete('1.0', tk.END)
    full_output = ""

    # Reading the output line by line in real-time
    while True:
        line = process.stdout.readline()
        if not line:
            break
        output_display.insert(tk.END, line)
        output_display.update()  # Update the widget
        full_output += line
        time.sleep(0.5)  # Add a small delay to simulate real-time update

    data = parse_output(full_output)
    plot_gantt_chart(data)

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
