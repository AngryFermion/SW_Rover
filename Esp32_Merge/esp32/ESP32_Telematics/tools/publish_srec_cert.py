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
from datetime import datetime
import paho.mqtt.client as mqtt

# AWS IoT Core Configuration
MQTT_BROKER = "abyj343g32dcn-ats.iot.us-east-1.amazonaws.com"
MQTT_PORT = 8883  # TLS port for AWS IoT Core
MQTT_CLIENT_ID = "sdv-vehicle-003"  # Thing name as client ID
MQTT_TOPIC_BASE = "sdv/vehicles/sdv-vehicle-001"
MQTT_TOPIC_FOTA = "fota"
MQTT_TOPIC_DATA = "data"

# Certificate paths
TOOLS_DIR = os.path.dirname(os.path.abspath(__file__))
CERT_DIR = os.path.join(os.path.dirname(TOOLS_DIR), "certificates")
AWS_ROOT_CA = os.path.join(CERT_DIR, "AmazonRootCA1.pem")
AWS_CERTIFICATE = os.path.join(CERT_DIR, "certificate.pem.crt")
AWS_PRIVATE_KEY = os.path.join(CERT_DIR, "private.pem.key")

# Configuration
DEBUG_MODE = False  # Set to False for interactive file selection
TARGET_DIR = os.path.join(TOOLS_DIR, "target")

def on_connect(client, userdata, flags, rc):
    """Callback for MQTT connection"""
    if rc == 0:
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Connected to AWS IoT Core successfully")
        userdata['connected'] = True
    else:
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Failed to connect to AWS IoT Core, return code {rc}")
        # AWS IoT Core specific error codes
        error_messages = {
            1: "Connection refused - incorrect protocol version",
            2: "Connection refused - invalid client identifier",
            3: "Connection refused - server unavailable",
            4: "Connection refused - bad username or password",
            5: "Connection refused - not authorized (check certificates and policies)"
        }
        if rc in error_messages:
            print(f"[{datetime.now().strftime('%H:%M:%S')}] Error details: {error_messages[rc]}")
        userdata['connected'] = False

def on_publish(client, userdata, mid):
    """Callback for MQTT publish"""
    print(f"[{datetime.now().strftime('%H:%M:%S')}] Message published with ID: {mid}")

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
    chunk_size = (1024*4)
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
    metadata_topic = f"{MQTT_TOPIC_BASE}/{MQTT_TOPIC_FOTA}/metadata"
    client.publish(metadata_topic, json.dumps(metadata), qos=1)
    print(f"[{datetime.now().strftime('%H:%M:%S')}] Published metadata to '{metadata_topic}' - {total_chunks} chunks")

    # Wait before publishing chunks
    time.sleep(2)

    # Publish file content in chunks
    for chunk_num in range(total_chunks):
        start_pos = chunk_num * chunk_size
        end_pos = min(start_pos + chunk_size, len(srec_content))
        chunk_data = srec_content[start_pos:end_pos]

        chunk_topic = f"{MQTT_TOPIC_BASE}/{MQTT_TOPIC_FOTA}/chunk/{chunk_num}"

        # Create chunk message with metadata
        chunk_message = {
            "chunk_num": chunk_num,
            "total_chunks": total_chunks,
            "data": chunk_data
        }

        result = client.publish(chunk_topic, json.dumps(chunk_message), qos=1)

        if result.rc == mqtt.MQTT_ERR_SUCCESS:
            print(f"[{datetime.now().strftime('%H:%M:%S')}] Published chunk {chunk_num + 1}/{total_chunks} ({len(chunk_data)} bytes)")
        else:
            print(f"[{datetime.now().strftime('%H:%M:%S')}] Failed to publish chunk {chunk_num}, error code: {result.rc}")
            return False

        # Small delay between chunks
        time.sleep(0.1)

    # Send completion signal
    completion_topic = f"{MQTT_TOPIC_BASE}/{MQTT_TOPIC_FOTA}/complete"
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

    # List available .srec files
    srec_files = list_srec_files()
    if not srec_files:
        print(f"[{datetime.now().strftime('%H:%M:%S')}] No .srec files found. Exiting.")
        return

    # Select file based on debug mode
    if DEBUG_MODE:
        # Automatically select the first file
        selected_file = srec_files[0]
        filename = os.path.basename(selected_file)
        print(f"[{datetime.now().strftime('%H:%M:%S')}] DEBUG MODE: Auto-selected first file: {filename}")
    else:
        # Get user's file selection
        selected_file = get_user_selection(srec_files)
        if selected_file is None:
            print(f"[{datetime.now().strftime('%H:%M:%S')}] No file selected. Exiting.")
            return

    # Setup MQTT client with TLS for AWS IoT Core
    userdata = {'connected': False}
    client = mqtt.Client(client_id=MQTT_CLIENT_ID, userdata=userdata)

    # Validate certificate files exist
    print(f"[{datetime.now().strftime('%H:%M:%S')}] Validating certificates...")
    for cert_file, cert_path in [
        ("Root CA", AWS_ROOT_CA),
        ("Certificate", AWS_CERTIFICATE),
        ("Private Key", AWS_PRIVATE_KEY)
    ]:
        if not os.path.exists(cert_path):
            print(f"[{datetime.now().strftime('%H:%M:%S')}] Error: {cert_file} not found at {cert_path}")
            return
        print(f"[{datetime.now().strftime('%H:%M:%S')}] ✓ Found {cert_file}")

    # Configure TLS/SSL
    print(f"[{datetime.now().strftime('%H:%M:%S')}] Configuring TLS/SSL...")
    client.tls_set(
        ca_certs=AWS_ROOT_CA,
        certfile=AWS_CERTIFICATE,
        keyfile=AWS_PRIVATE_KEY
    )
    client.on_connect = on_connect
    client.on_publish = on_publish

    try:
        # Connect to AWS IoT Core
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Connecting to AWS IoT Core...")
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Endpoint: {MQTT_BROKER}")
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Client ID: {MQTT_CLIENT_ID}")
        client.connect(MQTT_BROKER, MQTT_PORT, 60)
        client.loop_start()

        # Wait for connection with timeout
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Waiting for connection...")
        max_wait = 10  # seconds
        wait_time = 0
        while not userdata['connected'] and wait_time < max_wait:
            time.sleep(0.5)
            wait_time += 0.5

        if not userdata['connected']:
            print(f"[{datetime.now().strftime('%H:%M:%S')}] Connection timeout - failed to connect within {max_wait} seconds")
            return

        # Publish the selected .srec file
        if publish_srec_file(client, selected_file):
            print(f"[{datetime.now().strftime('%H:%M:%S')}] File published successfully")
        else:
            print(f"[{datetime.now().strftime('%H:%M:%S')}] Failed to publish file")

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
    main()