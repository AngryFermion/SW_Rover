#!/usr/bin/env python3
"""
FOTA Serial Receiver Simulator
Simulates a device receiving SREC data via serial and responding with OK
"""

import os
import sys
import time
import serial
import threading
from datetime import datetime
import random

# Configuration - Modify these as needed
SERIAL_PORT = "COM9"
BAUD_RATE = 115200
RESPONSE_DELAY_MS = 50          # Delay before sending OK response (milliseconds)
ERROR_SIMULATION_RATE = 0       # Percentage (0-100) chance of not sending OK (0 = no errors)
TIMEOUT_SECONDS = 5             # Serial read timeout

# Directories and files
TOOLS_DIR = os.path.dirname(os.path.abspath(__file__))
OUTPUT_DIR = os.path.join(TOOLS_DIR, "serial_rx")
OUTPUT_FILE = os.path.join(OUTPUT_DIR, "temp.srec")

# Global variables
received_lines = []
total_lines = 0
total_bytes = 0
start_time = None
ser = None
running = False

def create_output_directory():
    """Create output directory if it doesn't exist"""
    if not os.path.exists(OUTPUT_DIR):
        os.makedirs(OUTPUT_DIR)
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Created directory: {OUTPUT_DIR}")

def initialize_serial():
    """Initialize serial connection"""
    global ser
    try:
        ser = serial.Serial(
            port=SERIAL_PORT,
            baudrate=BAUD_RATE,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=TIMEOUT_SECONDS,
            xonxoff=False,
            rtscts=False,
            dsrdtr=False
        )
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Serial port {SERIAL_PORT} opened successfully")
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Configuration: {BAUD_RATE} baud, timeout: {TIMEOUT_SECONDS}s")
        return True
    except serial.SerialException as e:
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Error opening serial port {SERIAL_PORT}: {e}")
        return False

def send_ok_response():
    """Send OK response with configured delay"""
    global ser

    # Simulate processing delay
    if RESPONSE_DELAY_MS > 0:
        time.sleep(RESPONSE_DELAY_MS / 1000.0)

    # Error simulation - sometimes don't send OK
    if ERROR_SIMULATION_RATE > 0:
        if random.randint(1, 100) <= ERROR_SIMULATION_RATE:
            print(f"[{datetime.now().strftime('%H:%M:%S')}] Simulating error - NOT sending OK response")
            return False

    try:
        ser.write(b"OK\r\n")
        ser.flush()
        return True
    except serial.SerialException as e:
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Error sending OK response: {e}")
        return False

def save_received_data():
    """Save received SREC data to file"""
    try:
        with open(OUTPUT_FILE, 'w', encoding='utf-8') as f:
            for line in received_lines:
                f.write(line + '\n')

        file_size = os.path.getsize(OUTPUT_FILE)
        print(f"[{datetime.now().strftime('%H:%M:%S')}] File saved: {OUTPUT_FILE}")
        print(f"[{datetime.now().strftime('%H:%M:%S')}] File size: {file_size} bytes")
        return True
    except Exception as e:
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Error saving file: {e}")
        return False

def display_progress():
    """Display current progress"""
    global start_time, total_lines, total_bytes

    if start_time:
        elapsed = time.time() - start_time
        rate = total_lines / elapsed if elapsed > 0 else 0
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Progress: {total_lines} lines, {total_bytes} bytes, {rate:.1f} lines/sec")

def monitor_serial():
    """Main serial monitoring function"""
    global ser, running, received_lines, total_lines, total_bytes, start_time

    print(f"[{datetime.now().strftime('%H:%M:%S')}] Starting serial monitoring...")
    print(f"[{datetime.now().strftime('%H:%M:%S')}] Response delay: {RESPONSE_DELAY_MS}ms")
    print(f"[{datetime.now().strftime('%H:%M:%S')}] Error simulation: {ERROR_SIMULATION_RATE}%")
    print(f"[{datetime.now().strftime('%H:%M:%S')}] Waiting for SREC data...")

    line_buffer = ""
    last_progress_time = time.time()
    last_data_time = time.time()
    previous_total_lines = 0
    no_data_timeout = 30  # seconds

    while running:
        try:
            # Read data from serial port
            data = ser.read(1)
            if data:
                char = data.decode('utf-8', errors='ignore')
                last_data_time = time.time()  # Update last data received time

                if char == '\n':
                    # Complete line received
                    line = line_buffer.strip()
                    if line and line.startswith('S'):
                        if start_time is None:
                            start_time = time.time()
                            print(f"[{datetime.now().strftime('%H:%M:%S')}] First SREC line received - starting transfer")

                        received_lines.append(line)
                        total_lines += 1
                        total_bytes += len(line)

                        print(f"[{datetime.now().strftime('%H:%M:%S')}] Received line {total_lines}: {line[:20]}..." +
                              (f" ({len(line)} bytes)" if len(line) <= 20 else f" ({len(line)} bytes)"))

                        # Send OK response
                        if send_ok_response():
                            print(f"[{datetime.now().strftime('%H:%M:%S')}] Sent OK response for line {total_lines}")
                        else:
                            print(f"[{datetime.now().strftime('%H:%M:%S')}] Failed to send OK response for line {total_lines}")

                    line_buffer = ""

                elif char == '\r':
                    # Ignore carriage return
                    pass
                else:
                    # Add character to buffer
                    line_buffer += char

            # Check for transmission timeout
            current_time = time.time()
            if start_time and (current_time - last_data_time > no_data_timeout):
                print(f"[{datetime.now().strftime('%H:%M:%S')}] No data received for {no_data_timeout}s - transmission may be complete")
                break

            # Display progress every 5 seconds, but only if new lines received
            if current_time - last_progress_time >= 5.0:
                if total_lines > 0 and total_lines > previous_total_lines:
                    display_progress()
                    previous_total_lines = total_lines
                elif total_lines > 0 and start_time:
                    # Show final progress once without repeating
                    if previous_total_lines != total_lines:
                        display_progress()
                        previous_total_lines = total_lines
                last_progress_time = current_time

        except serial.SerialException as e:
            print(f"[{datetime.now().strftime('%H:%M:%S')}] Serial error: {e}")
            break
        except UnicodeDecodeError:
            # Ignore decode errors
            pass
        except KeyboardInterrupt:
            print(f"\n[{datetime.now().strftime('%H:%M:%S')}] Interrupted by user")
            break

def main():
    """Main function"""
    global running

    print("=" * 60)
    print("FOTA Serial Receiver Simulator")
    print("=" * 60)
    print(f"Port: {SERIAL_PORT}")
    print(f"Baud Rate: {BAUD_RATE}")
    print(f"Response Delay: {RESPONSE_DELAY_MS}ms")
    print(f"Error Simulation: {ERROR_SIMULATION_RATE}%")
    print(f"Output File: {OUTPUT_FILE}")
    print("=" * 60)

    # Create output directory
    create_output_directory()

    # Initialize serial port
    if not initialize_serial():
        return 1

    try:
        running = True

        # Start monitoring
        monitor_serial()

    except Exception as e:
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Unexpected error: {e}")
        return 1

    finally:
        running = False

        # Clean up
        if ser and ser.is_open:
            ser.close()
            print(f"[{datetime.now().strftime('%H:%M:%S')}] Serial port closed")

        # Save received data
        if received_lines:
            print(f"\n[{datetime.now().strftime('%H:%M:%S')}] Transfer completed!")
            display_progress()

            if save_received_data():
                print(f"[{datetime.now().strftime('%H:%M:%S')}] SUCCESS: {total_lines} lines saved to {OUTPUT_FILE}")
            else:
                print(f"[{datetime.now().strftime('%H:%M:%S')}] ERROR: Failed to save received data")
        else:
            print(f"[{datetime.now().strftime('%H:%M:%S')}] No SREC data received")

    return 0

if __name__ == "__main__":
    try:
        exit_code = main()
        sys.exit(exit_code)
    except KeyboardInterrupt:
        print(f"\n[{datetime.now().strftime('%H:%M:%S')}] Program terminated by user")
        sys.exit(0)