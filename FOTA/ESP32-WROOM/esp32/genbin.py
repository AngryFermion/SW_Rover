import os
from datetime import datetime

folder = "bin/"
title = "qi"
board_id = "061ss"
name = "DISP_BOARD"
memory_size_str = "0x400000"
com_port = "COM36"

now = datetime.now() # current date and time
date_time = now.strftime("%y%m%d-%H%M")
print("date and time:",date_time)

# os.system('cmd /k "esptool.py -p COM36 -b 921600 read_flash 0 0x400000 FoTA.bin"')
fName =  folder + title + '_' + board_id + '_' + name + '_' + date_time + '.bin'
cmd = "python -m esptool -p " + com_port + " -b 921600 read-flash 0 " + memory_size_str + " " + fName
print("cmd:" + cmd)

os.system('cmd /k "' + cmd + '"')