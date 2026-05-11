import os
import subprocess
from pathlib import Path


def align_srec(input_file: str, output_file: str,
               block_size: int = 128, address_length: int = 4) -> bool:
    """
    Calls srec_cat.exe to align an SREC file.
    
    Args:
        input_file (str): Path to the input .srec file.
        output_file (str): Path to the aligned .srec file.
        block_size (int): Output block size (default = 128).
        address_length (int): Address length (default = 4).
    
    Returns:
        bool: True if successful, False otherwise.
    """

    
    # exe = os.environ.get("SREC_CAT", "srec_cat.exe")

    # # If env variable points to a directory, append exe name
    # if os.path.isdir(exe):
    #     exe = os.path.join(exe, "srec_cat.exe")

    exe = 'D:/Vishnu/SmartWheelsPro/appimage/srecord-1.64-win32/srec_cat.exe'

    print("SREC_PATH:",exe)
    of = Path("build") / output_file 
    cmd = [
        exe,
        input_file,
        "-Output_Block_Size", str(block_size),
        "-Output_Block_Packing",
        "-address-length", str(address_length),
        "-output", of
    ]

    try:
        result = subprocess.check_call(cmd, shell=True)
        
        print("srec_cat executed successfully ")
        return True
    except subprocess.CalledProcessError as e:
        print("Error executing srec_cat:")
        print("STDOUT:\n", e.stdout)
        print("STDERR:\n", e.stderr)
        return False
    except FileNotFoundError:
        print(" srec_cat.exe not found. Make sure it's in PATH or provide full path.")
        return False
