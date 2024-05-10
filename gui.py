import subprocess
import tkinter as tk
from tkinter import scrolledtext
import os

def run_c_program():
    # Determine which scheduling algorithm was selected
    algorithm = algorithm_var.get()

    if os.name == 'nt':  # Windows
        cmd = ['main.exe', algorithm]
    else:  # Unix-like
        cmd = ['./main', algorithm]

# Run the C executable with the chosen algorithm
    process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    
    # Clear previous output
    output_display.delete('1.0', tk.END)

    # Display new output in the scrolled text widget
    for line in iter(process.stdout.readline, ''):
        output_display.insert(tk.END, line)
        output_display.yview(tk.END)  # Auto-scroll to the bottom

    # Handle any errors
    errors = process.stderr.read()
    if errors:
        output_display.insert(tk.END, f'Errors:\n{errors}')

# Create the main window
root = tk.Tk()
root.title('Scheduling Algorithm Simulator')

# Radio buttons for algorithm selection
algorithm_var = tk.StringVar(value='fcfs')  # Default selection
algorithms = [('First-Come, First-Served', 'fcfs'), ('Shortest Job First', 'sjf'), ('Priority Preemptive', 'pp')]
for text, value in algorithms:
    tk.Radiobutton(root, text=text, variable=algorithm_var, value=value).pack(anchor=tk.W)

# Scrolled text widget for output
output_display = scrolledtext.ScrolledText(root, width=80, height=20)
output_display.pack(pady=20)

# Button to run the C program
run_button = tk.Button(root, text='Run Selected Algorithm', command=run_c_program)
run_button.pack(pady=10)

# Start the GUI event loop
root.mainloop()
