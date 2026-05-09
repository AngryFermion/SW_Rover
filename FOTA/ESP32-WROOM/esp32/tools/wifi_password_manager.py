import json
from Crypto.Cipher import AES
import binascii
import os

CONFIG_PATH = os.path.join(os.path.dirname(__file__), '..', 'data', 'config', 'registration.json')
AES_KEY = b'MyESP32Key123456'

# Read registration.json and get wifi_pw
with open(CONFIG_PATH, 'r') as f:
    reg = json.load(f)

hex_pw = reg['oem']['wifi_pw']
enc_pw = binascii.unhexlify(hex_pw)

# Decrypt password
cipher = AES.new(AES_KEY, AES.MODE_ECB)
decrypted = cipher.decrypt(enc_pw)
# C++ code truncates at 16 characters, so show what ESP32 actually sees
plain_pw = decrypted.decode('utf-8')[:16].rstrip('\x00')
print(f'Current WiFi password (max 16 chars): {plain_pw}')

new_pw = input('Enter new WiFi password (max 16 chars, leave blank to keep current): ')
if new_pw:
    if len(new_pw) > 16:
        print(f'Warning: Password truncated to 16 characters: {new_pw[:16]}')
        new_pw = new_pw[:16]
    # Pad to 16 bytes
    padded = new_pw.encode('utf-8').ljust(16, b'\x00')
    enc_new = cipher.encrypt(padded)
    hex_new = binascii.hexlify(enc_new).decode('utf-8')
    reg['oem']['wifi_pw'] = hex_new
    with open(CONFIG_PATH, 'w') as f:
        json.dump(reg, f, indent=4)
    print('WiFi password updated and encrypted.')
else:
    print('WiFi password unchanged.')
