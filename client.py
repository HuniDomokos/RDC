import socket
import sys
import tkinter as tk
from tkinter import messagebox

HOST = sys.argv[1] if len(sys.argv) > 1 else "127.0.0.1"
PORT = int(sys.argv[2]) if len(sys.argv) > 2 else 5000


class Connection:
    def __init__(self, host, port):
        self.addr = (host, port)
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.settimeout(3)
        self.sock.connect(self.addr)

    def calculate(self, a, b, op):
        self.sock.send(f"{a} {op} {b}".encode())
        try:
            data = self.sock.recv(4096)
        except socket.timeout:
            raise ConnectionError("No reply from server (timed out)")
        return data.decode().strip()

    def close(self):
        self.sock.close()


class CalculatorApp:
    def __init__(self, root, conn):
        self.conn = conn
        self.root = root
        root.title(f"Calculator - {HOST}:{PORT}")
        root.resizable(False, False)

        self.display = tk.Entry(root, font=("Consolas", 22), justify="right",
                                bd=6, relief="sunken")
        self.display.grid(row=0, column=0, columnspan=4, sticky="nsew", padx=4, pady=4)
        self.display.insert(0, "0")

        layout = [
            ["7", "8", "9", "/"],
            ["4", "5", "6", "*"],
            ["1", "2", "3", "-"],
            ["C", "0", "=", "+"],
        ]
        for r, row in enumerate(layout, start=1):
            for c, label in enumerate(row):
                tk.Button(root, text=label, font=("Consolas", 16), width=4, height=2,
                          command=lambda l=label: self.press(l)).grid(
                    row=r, column=c, padx=2, pady=2)

        self.first = None
        self.op = None
        self.fresh = True  # next digit starts a new number

    def set_display(self, text):
        self.display.delete(0, tk.END)
        self.display.insert(0, text)

    def press(self, key):
        if key.isdigit():
            if self.fresh:
                self.set_display(key)
                self.fresh = False
            else:
                self.set_display(self.display.get() + key)
        elif key == "C":
            self.first = self.op = None
            self.fresh = True
            self.set_display("0")
        elif key in "+-*/":
            if self.op is None:
                self.first = int(self.display.get())
                self.op = key
                self.fresh = True
            else:
                self.op = key
        elif key == "=":
            self.evaluate()

    def evaluate(self):
        if self.op is None:
            return
        try:
            if self.conn is None:
                self.conn = Connection(HOST, PORT)
                self.root.title(f"Calculator - {HOST}:{PORT} (connected)")
            second = int(self.display.get())
            reply = self.conn.calculate(self.first, second, self.op)
        except (OSError, ConnectionError) as e:
            self.conn = None
            self.root.title(f"Calculator - {HOST}:{PORT} (not connected)")
            messagebox.showerror("Connection error", f"{HOST}:{PORT}\n{e}")
            return
        if reply.startswith("Result:"):
            self.set_display(reply.split(":", 1)[1].strip())
        else:
            self.set_display("Error")
            messagebox.showwarning("Server", reply)
        self.first = self.op = None
        self.fresh = True


def main():
    try:
        conn = Connection(HOST, PORT)
    except OSError as e:
        print(f"Could not connect to {HOST}:{PORT} - {e} (will retry on '=')")
        conn = None

    root = tk.Tk()
    app = CalculatorApp(root, conn)
    if conn is None:
        root.title(f"Calculator - {HOST}:{PORT} (not connected)")

    def on_close():
        if app.conn:
            app.conn.close()
        root.destroy()

    root.protocol("WM_DELETE_WINDOW", on_close)
    root.mainloop()


if __name__ == "__main__":
    main()