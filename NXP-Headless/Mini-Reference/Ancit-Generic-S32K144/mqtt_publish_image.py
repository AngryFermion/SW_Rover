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

# MQTT Configuration
MQTT_BROKER = "test.mosquitto.org"  #"broker.emqx.io"#"test.mosquitto.org"
MQTT_PORT = 1884    #1883#1884
MQTT_USERNAME = "rw"
MQTT_PASSWORD = "readwrite"
MQTT_TOPIC_BASE = "SmartWheels"
MQTT_TOPIC_FOTA = "telematics/fota"
MQTT_TOPIC_DATA = "telematics/data"

# Configuration
TOOLS_DIR = os.path.dirname(os.path.abspath(__file__))
TARGET_DIR = os.path.join(TOOLS_DIR, "target")

def on_connect(client, userdata, flags, rc):
    """Callback for MQTT connection"""
    if rc == 0:
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Connected to MQTT broker")
    else:
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Failed to connect to MQTT broker, return code {rc}")

def on_publish(client, userdata, mid):
    """Callback for MQTT publish"""
    print(f"[{datetime.now().strftime('%H:%M:%S')}] Message published with ID: {mid}")

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

    # --- changed section: take file from command-line args ---
    if len(sys.argv) < 2:
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Usage: python3 {sys.argv[0]} <path/to/file.srec>")
        return

    selected_file = sys.argv[1]
    if not os.path.isfile(selected_file):
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Error: file '{selected_file}' not found")
        return

    print(f"[{datetime.now().strftime('%H:%M:%S')}] Selected file: {os.path.basename(selected_file)}")

    # Setup MQTT client
    client = mqtt.Client()
    client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)
    client.on_connect = on_connect
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
