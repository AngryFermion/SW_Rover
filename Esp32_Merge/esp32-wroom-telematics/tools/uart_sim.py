"""
uart_sim.py — SmartWheels ESP32-WROOM Telematics
Simulates the NXP MCU sending CAN signals to the ESP32 over UART.

Usage:
    python uart_sim.py                         # auto mode, COM3, 115200, 1 s interval
    python uart_sim.py --port COM5             # different port
    python uart_sim.py --interval 0.5          # faster (500 ms per signal)
    python uart_sim.py -i                      # interactive: type your own signals
    python uart_sim.py --signal Speed --value 60  # send one signal and exit

Wire-up (when USE_DUMMY_DATA is commented out in uart_config.h):
    PC TX  -->  ESP32 GPIO16 (UART1 RX)
    PC RX  <--  ESP32 GPIO17 (UART1 TX)
    GND    ---  GND

Protocol: one line per signal, terminated with \\n
    SIGNAL_NAME,VALUE\\n
    e.g.  Speed,85\\n  or  ENGINE_RPM,3200\\n
"""

import argparse
import sys
import time

try:
    import serial
except ImportError:
    print("[ERROR] pyserial not installed. Run: pip install pyserial")
    sys.exit(1)


# ---------------------------------------------------------------------------
# Signal definitions — mirrors the dummy data engine in uart_manager.cpp
# ---------------------------------------------------------------------------
SIGNALS = [
    {"name": "Speed",          "min": 0,    "max": 120,  "step": 5,   "value": 0},
    {"name": "ENGINE_RPM",     "min": 800,  "max": 6000, "step": 200, "value": 800},
    {"name": "THROTTLE",       "min": 0,    "max": 100,  "step": 5,   "value": 0},
    {"name": "BRAKE",          "min": 0,    "max": 100,  "step": 10,  "value": 0},
    {"name": "STEERING_ANGLE", "min": -90,  "max": 90,   "step": 15,  "value": -90},
]


def open_port(port: str, baud: int) -> serial.Serial:
    try:
        ser = serial.Serial(port, baudrate=baud, timeout=1)
        print(f"[UART Sim] Port {port} opened at {baud} baud")
        return ser
    except serial.SerialException as e:
        print(f"[ERROR] Cannot open {port}: {e}")
        sys.exit(1)


def send_signal(ser: serial.Serial, name: str, value: int | float) -> None:
    line = f"{name},{int(value)}\n"
    ser.write(line.encode("ascii"))
    print(f"[TX] {line.strip()}")


def next_value(sig: dict) -> int:
    sig["value"] += sig["step"]
    if sig["value"] > sig["max"]:
        sig["value"] = sig["min"]
    return sig["value"]


# ---------------------------------------------------------------------------
# Modes
# ---------------------------------------------------------------------------

def run_auto(ser: serial.Serial, interval: float) -> None:
    """Continuously cycle through all signals, one per interval."""
    print(f"\n[UART Sim] Auto mode — interval {interval} s  |  Ctrl+C to stop\n")
    idx = 0
    try:
        while True:
            sig = SIGNALS[idx]
            value = next_value(sig)
            send_signal(ser, sig["name"], value)
            idx = (idx + 1) % len(SIGNALS)
            time.sleep(interval)
    except KeyboardInterrupt:
        print("\n[UART Sim] Stopped.")


def run_interactive(ser: serial.Serial) -> None:
    """Let the user type signals manually."""
    print("\n[UART Sim] Interactive mode — type  SIGNAL_NAME,VALUE  or  q  to quit\n")
    while True:
        try:
            raw = input(">>> ").strip()
        except (EOFError, KeyboardInterrupt):
            print("\n[UART Sim] Stopped.")
            break

        if raw.lower() in ("q", "quit", "exit"):
            break

        if "," not in raw:
            print("[WARN] Format must be  SIGNAL_NAME,VALUE  (e.g.  Speed,80)")
            continue

        send_signal(ser, *raw.split(",", 1))


def run_once(ser: serial.Serial, name: str, value: str) -> None:
    """Send a single signal then exit."""
    send_signal(ser, name, value)


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

def main() -> None:
    parser = argparse.ArgumentParser(
        description="Simulate MCU UART output for ESP32-WROOM Telematics"
    )
    parser.add_argument("--port",     default="COM15",  help="Serial port (default: COM3)")
    parser.add_argument("--baud",     default=115200,  type=int, help="Baud rate (default: 115200)")
    parser.add_argument("--interval", default=1.0,     type=float,
                        help="Seconds between signals in auto mode (default: 1.0)")
    parser.add_argument("-i", "--interactive", action="store_true",
                        help="Interactive mode: type signals manually")
    parser.add_argument("--signal",   default=None,
                        help="Signal name for single-shot send (requires --value)")
    parser.add_argument("--value",    default=None,
                        help="Signal value for single-shot send (requires --signal)")
    args = parser.parse_args()

    ser = open_port(args.port, args.baud)

    # Brief pause so the ESP32 UART is ready after the port opens
    time.sleep(0.5)

    try:
        if args.signal and args.value:
            run_once(ser, args.signal, args.value)
        elif args.interactive:
            run_interactive(ser)
        else:
            run_auto(ser, args.interval)
    finally:
        ser.close()
        print("[UART Sim] Port closed.")


if __name__ == "__main__":
    main()
