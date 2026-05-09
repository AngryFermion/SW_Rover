from pathlib import Path
import subprocess
import os

def run_command(cmd, description):
    print(f"\n Running: {description} ...")
    try:
        subprocess.check_call(cmd, shell=True)
        print(f"{description} succeeded.\n")
    except subprocess.CalledProcessError as e:
        print(f"{description} failed: {e}\n")
        exit(1)

def flash_with_jlink(elf_path: Path):
    jlink_script = Path("flash_factory.jlink")
    with open(jlink_script, "w") as f:
        f.write("r\n")
        f.write("halt\n")
        f.write(f"loadfile {elf_path.as_posix()}\n")
        f.write("r\n")
        f.write("go\n")
        f.write("exit\n")

    cmd = [
        "..\\tools\\JLink_V798h\\JLink.exe",
        "-device", "S32K144",
        "-if", "SWD",
        "-speed", "4000",
        "-CommanderScript", str(jlink_script)
    ]

    run_command(" ".join(cmd), "Flashing ELF with J-Link")

def main():
    print("Flashing factory default elf file")
    elf_file = Path("SubSys_Mini_HeadLight_original.elf")
    flash_with_jlink(elf_file)

if __name__ == "__main__":
    main()