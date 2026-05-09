#!/usr/bin/env python3
"""
MQTT SREC File Publisher
Publishes .srec files from a folder to MQTT broker
"""

import os
import sys
import time
import glob
import json
import threading
from datetime import datetime
import paho.mqtt.client as mqtt

# MQTT Configuration
MQTT_BROKER = "broker.emqx.io" #"test.mosquitto.org"
MQTT_PORT = 1883 #1884
MQTT_USERNAME = "rw"
MQTT_PASSWORD = "readwrite"

# MQTT Topic Configuration (using new directional structure)
MQTT_BASE_TOPIC = "SmartWheelsNS"
DEVICE_ID = "ABCD"  # Device ID (fixed for now)

# Build MQTT topics with device ID - Server → Device (server publishes these)
TOPIC_SERVER_FOTA_METADATA = f"{MQTT_BASE_TOPIC}/server/{DEVICE_ID}/fota/metadata"
TOPIC_SERVER_FOTA_CHUNK = f"{MQTT_BASE_TOPIC}/server/{DEVICE_ID}/fota/chunk"
TOPIC_SERVER_FOTA_COMPLETE = f"{MQTT_BASE_TOPIC}/server/{DEVICE_ID}/fota/complete"

# Build MQTT topics with device ID - Device → Server (device publishes these, server subscribes)
TOPIC_DEVICE_FOTA_ACK = f"{MQTT_BASE_TOPIC}/device/{DEVICE_ID}/fota/ack"
TOPIC_DEVICE_STATUS = f"{MQTT_BASE_TOPIC}/device/{DEVICE_ID}/status"

# Legacy topic parts (for backward compatibility if needed)
MQTT_TOPIC_FOTA = "fota"
MQTT_TOPIC_DATA = "telematics"

# Configuration
DEBUG_MODE = False  # Set to False for interactive file selection
CHUNK_SIZE = 4096  # 4KB chunk size for FOTA transfer
ACK_TIMEOUT = 10   # Timeout in seconds to wait for chunk acknowledgment
TOOLS_DIR = os.path.dirname(os.path.abspath(__file__))
TARGET_DIR = os.path.join(TOOLS_DIR, "temp")

# Global variables for ACK handling
ack_received = {}
ack_event = None

def on_connect(client, userdata, flags, reason_code, properties):
    """Callback for MQTT connection"""
    if reason_code == 0:
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Connected to MQTT broker")
        # Subscribe to ACK topic to receive acknowledgments
        client.subscribe(TOPIC_DEVICE_FOTA_ACK, qos=0)
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Subscribed to ACK topic: {TOPIC_DEVICE_FOTA_ACK}")
    else:
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Failed to connect to MQTT broker, return code {reason_code}")

def on_message(client, userdata, msg):
    """Callback for MQTT message reception"""
    global ack_received, ack_event

    try:
        if msg.topic == TOPIC_DEVICE_FOTA_ACK:
            ack_data = json.loads(msg.payload.decode())
            chunk_num = ack_data.get("chunk")
            status = ack_data.get("status")

            print(f"[{datetime.now().strftime('%H:%M:%S')}] ACK received for chunk {chunk_num}: {status}")

            # Store the ACK result
            ack_received[chunk_num] = (status == "ok")

            # Signal that ACK was received
            if ack_event:
                ack_event.set()

    except Exception as e:
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Error processing ACK: {e}")

def on_publish(client, userdata, mid, reason_code, properties):
    """Callback for MQTT publish"""
    # Reduced logging for less spam
    pass

def list_srec_files():
    """List all .srec files in the target directory"""
    srec_files = glob.glob(os.path.join(TARGET_DIR, "*.srec"))

    if not srec_files:
        print(f"[{datetime.now().strftime('%H:%M:%S')}] No .srec files found in target directory")
        return []

    print(f"\n[{datetime.now().strftime('%H:%M:%S')}] Available .srec files in target folder:")
    print("-" * 60)

    for i, file_path in enumerate(srec_files, 1):
        filename = os.path.basename(file_path)
        file_size = os.path.getsize(file_path)
        print(f"{i}. {filename} ({file_size} bytes)")

    print("-" * 60)
    return srec_files

def get_user_selection(srec_files):
    """Get user's file selection"""
    while True:
        try:
            choice = input(f"\nEnter serial number (1-{len(srec_files)}) or 'q' to quit: ").strip()

            if choice.lower() == 'q':
                return None

            choice_num = int(choice)
            if 1 <= choice_num <= len(srec_files):
                selected_file = srec_files[choice_num - 1]
                filename = os.path.basename(selected_file)
                print(f"[{datetime.now().strftime('%H:%M:%S')}] Selected: {filename}")
                return selected_file
            else:
                print(f"Invalid choice. Please enter a number between 1 and {len(srec_files)}")

        except ValueError:
            print("Invalid input. Please enter a number or 'q' to quit")
        except KeyboardInterrupt:
            print(f"\n[{datetime.now().strftime('%H:%M:%S')}] Operation cancelled by user")
            return None

def read_srec_file(file_path):
    """Read .srec file and return content"""
    try:
        with open(file_path, 'r', encoding='utf-8') as file:
            content = file.read()
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Read {len(content)} bytes from '{os.path.basename(file_path)}'")
        return content
    except Exception as e:
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Error reading file '{file_path}': {e}")
        return None

def publish_srec_file(client, file_path):
    """Publish .srec file to MQTT in chunks"""
    filename = os.path.basename(file_path)
    file_size = os.path.getsize(file_path)

    # Read file content
    srec_content = read_srec_file(file_path)
    if srec_content is None:
        return False

    # Calculate total chunks
    chunk_size = CHUNK_SIZE
    total_chunks = (len(srec_content) + chunk_size - 1) // chunk_size

    # Create metadata
    metadata = {
        "filename": filename,
        "size": file_size,
        "content_length": len(srec_content),
        "total_chunks": total_chunks,
        "chunk_size": chunk_size,
        "timestamp": datetime.now().isoformat(),
        "lines": len(srec_content.splitlines())
    }

    # Publish metadata first
    metadata_topic = TOPIC_SERVER_FOTA_METADATA
    client.publish(metadata_topic, json.dumps(metadata), qos=1)
    print(f"[{datetime.now().strftime('%H:%M:%S')}] Published metadata to '{metadata_topic}' - {total_chunks} chunks")

    # Wait before publishing chunks
    time.sleep(2)

    # Publish file content in chunks with ACK waiting
    for chunk_num in range(total_chunks):
        start_pos = chunk_num * chunk_size
        end_pos = min(start_pos + chunk_size, len(srec_content))
        chunk_data = srec_content[start_pos:end_pos]

        chunk_topic = f"{TOPIC_SERVER_FOTA_CHUNK}/{chunk_num}"

        # Create event for waiting for ACK
        global ack_event
        ack_event = threading.Event()

        # Publish raw chunk data directly (topic already has the index)
        result = client.publish(chunk_topic, chunk_data, qos=1)

        if result.rc == mqtt.MQTT_ERR_SUCCESS:
            print(f"[{datetime.now().strftime('%H:%M:%S')}] Published chunk {chunk_num + 1}/{total_chunks} ({len(chunk_data)} bytes) - Waiting for ACK...")

            # Wait for acknowledgment
            if ack_event.wait(timeout=ACK_TIMEOUT):
                if chunk_num in ack_received and ack_received[chunk_num]:
                    print(f"[{datetime.now().strftime('%H:%M:%S')}] Chunk {chunk_num} confirmed successfully")
                else:
                    print(f"[{datetime.now().strftime('%H:%M:%S')}] Chunk {chunk_num} failed - retrying...")
                    return False
            else:
                print(f"[{datetime.now().strftime('%H:%M:%S')}] Timeout waiting for ACK on chunk {chunk_num}")
                return False
        else:
            print(f"[{datetime.now().strftime('%H:%M:%S')}] Failed to publish chunk {chunk_num}, error code: {result.rc}")
            return False

    # Send completion signal
    completion_topic = TOPIC_SERVER_FOTA_COMPLETE
    completion_message = {
        "filename": filename,
        "total_chunks": total_chunks,
        "total_size": len(srec_content)
    }
    client.publish(completion_topic, json.dumps(completion_message), qos=1)
    print(f"[{datetime.now().strftime('%H:%M:%S')}] File transfer complete - {total_chunks} chunks sent")

    return True

def main():
    """Main function"""
    print(f"[{datetime.now().strftime('%H:%M:%S')}] Starting SREC MQTT Publisher")
    print(f"[{datetime.now().strftime('%H:%M:%S')}] Broker: {MQTT_BROKER}:{MQTT_PORT}")
    print(f"[{datetime.now().strftime('%H:%M:%S')}] Tools Directory: {TOOLS_DIR}")
    print(f"[{datetime.now().strftime('%H:%M:%S')}] Target Directory: {TARGET_DIR}")

    # Command-line argument mode (replaces user selection)
    if len(sys.argv) < 2:
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Usage: python publish_srec.py <path_to_srec_file>")
        return

    selected_file = sys.argv[1]
    if not os.path.isfile(selected_file):
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Error: File not found - {selected_file}")
        return

    filename = os.path.basename(selected_file)
    print(f"[{datetime.now().strftime('%H:%M:%S')}] Selected file from command line: {filename}")

    # Setup MQTT client
    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
    client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)
    client.on_connect = on_connect
    client.on_message = on_message
    client.on_publish = on_publish

    try:
        # Connect to MQTT broker
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Connecting to MQTT broker...")
        client.connect(MQTT_BROKER, MQTT_PORT, 60)
        client.loop_start()

        # Wait for connection
        time.sleep(2)

        # Publish the selected .srec file
        if publish_srec_file(client, selected_file):
            print(f"[{datetime.now().strftime('%H:%M:%S')}] File published successfully")
            return 0
        else:
            print(f"[{datetime.now().strftime('%H:%M:%S')}] Failed to publish file")
            return 1

        # Wait for all messages to be sent
        time.sleep(2)

    except KeyboardInterrupt:
        print(f"\n[{datetime.now().strftime('%H:%M:%S')}] Interrupted by user")
    except Exception as e:
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Error: {e}")
    finally:
        client.loop_stop()
        client.disconnect()
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Disconnected from MQTT broker")

if __name__ == "__main__":
    sys.exit(main())
