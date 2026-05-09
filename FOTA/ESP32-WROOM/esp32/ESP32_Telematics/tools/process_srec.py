#!/usr/bin/env python3
"""
SREC File Processor
Lists .srec files from a folder and allows user to select one for alignment and padding
"""

import os
import sys
import glob
from datetime import datetime
from pathlib import Path

# Add the current directory to the path to import our modules
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from srec_align import align_srec
from srec_padder import pad_srec

# Configuration
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
SOURCE_DIR = os.path.join(SCRIPT_DIR, "source")
TEMP_DIR = os.path.join(SCRIPT_DIR, "temp")
TARGET_DIR = os.path.join(SCRIPT_DIR, "target")

def list_srec_files(search_dir):
    """List all .srec files in the specified directory"""
    srec_files = glob.glob(os.path.join(search_dir, "*.srec"))

    if not srec_files:
        print(f"[{datetime.now().strftime('%H:%M:%S')}] No .srec files found in {search_dir}")
        return []

    print(f"\n[{datetime.now().strftime('%H:%M:%S')}] Available .srec files in {search_dir}:")
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

def process_srec_file(input_file):
    """Process the selected SREC file: align then pad"""
    filename = os.path.basename(input_file)
    name_without_ext = os.path.splitext(filename)[0]

    # Create output filenames in appropriate directories
    aligned_file = os.path.join(TEMP_DIR, f"{name_without_ext}_aligned.srec")
    final_file = os.path.join(TARGET_DIR, f"{name_without_ext}_aligned_padded.srec")

    print(f"\n[{datetime.now().strftime('%H:%M:%S')}] Processing {filename}...")

    # Step 1: Align the SREC file
    print(f"[{datetime.now().strftime('%H:%M:%S')}] Step 1: Aligning SREC file...")
    if align_srec(input_file, aligned_file):
        print(f"[{datetime.now().strftime('%H:%M:%S')}] ✓ Alignment successful: {os.path.basename(aligned_file)}")
    else:
        print(f"[{datetime.now().strftime('%H:%M:%S')}] ✗ Alignment failed")
        return False

    # Step 2: Pad the aligned file
    print(f"[{datetime.now().strftime('%H:%M:%S')}] Step 2: Padding aligned file...")
    try:
        pad_srec(aligned_file, final_file)
        print(f"[{datetime.now().strftime('%H:%M:%S')}] ✓ Padding successful: {os.path.basename(final_file)}")

        # Show final results
        print(f"\n[{datetime.now().strftime('%H:%M:%S')}] Processing complete!")
        print(f"Original file: source/{filename}")
        print(f"Aligned file: temp/{os.path.basename(aligned_file)}")
        print(f"Final file: target/{os.path.basename(final_file)}")

        return True
    except Exception as e:
        print(f"[{datetime.now().strftime('%H:%M:%S')}] ✗ Padding failed: {e}")
        return False

def main():
    """Main function"""
    print(f"[{datetime.now().strftime('%H:%M:%S')}] Starting SREC File Processor")
    print(f"[{datetime.now().strftime('%H:%M:%S')}] Script Directory: {SCRIPT_DIR}")
    print(f"[{datetime.now().strftime('%H:%M:%S')}] Source Directory: {SOURCE_DIR}")
    print(f"[{datetime.now().strftime('%H:%M:%S')}] Temp Directory: {TEMP_DIR}")
    print(f"[{datetime.now().strftime('%H:%M:%S')}] Target Directory: {TARGET_DIR}")

    # List available .srec files from source directory
    srec_files = list_srec_files(SOURCE_DIR)
    if not srec_files:
        print(f"[{datetime.now().strftime('%H:%M:%S')}] No .srec files found. Exiting.")
        return

    # Get user's file selection
    selected_file = get_user_selection(srec_files)
    if selected_file is None:
        print(f"[{datetime.now().strftime('%H:%M:%S')}] No file selected. Exiting.")
        return

    # Process the selected file
    if process_srec_file(selected_file):
        print(f"[{datetime.now().strftime('%H:%M:%S')}] All operations completed successfully!")
    else:
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Processing failed!")

if __name__ == "__main__":
    main()