import serial
import time


def wait_for_ack(ser, ack=b"AOK", timeout=5) -> bool:
    """
    Wait for ACK from target.
    Args:
        ser: opened serial.Serial object
        ack (bytes): expected response, default b"AOK"
        timeout (int): timeout in seconds
    Returns:
        True if ACK received, False otherwise
    """
    resp = ser.read(3)  # read exactly 3 bytes
    if resp == b"AOK":
        print("Received ACK: AOK")
        return True
    elif resp:
        print(f"Received unexpected response: {resp.decode(errors='replace')}")
    else:
        print("No response received (timeout).")
    return False


def transmit_srec(port: str, baudrate: int, srec_file: str, ack: bytes = b"AOK") -> None:
    """
    Transmit an SREC file line by line over UART and wait for ACK each line.

    Args:
        port (str): serial port (e.g., "COM25" or "/dev/ttyUSB0")
        baudrate (int): baudrate (e.g., 115200)
        srec_file (str): path to the SREC file
        ack (bytes): expected ACK response (default b"AOK")
    """
    ser = serial.Serial(port=port, baudrate=baudrate, timeout=5)

    try:
        with open(srec_file, "r") as f:
            for line in f:
                line = line.strip()
                if not line:
                    continue

                # Send line
                ser.reset_output_buffer()
                ser.write(line.encode())
                ser.flush()
                # print(f"Sent: {line}")

                if line.startswith("S5"):
                    print("Update Complete. Waiting for Application to Boot")
                    continue


                # Wait for acknowledgment
                if not wait_for_ack(ser, ack=ack):
                    print("Stopping transmission due to invalid/missing ACK.")
                    break
    finally:
        ser.close()
        print("SREC transmission complete.")
