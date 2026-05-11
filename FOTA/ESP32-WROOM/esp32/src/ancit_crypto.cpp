#include "ancit_crypto.h"

#include <config_reader.h>
#include <mbedtls/aes.h>

#include "main.h"
#include <BT_LOGGER.h>

const uint8_t aes_key[16] = {'M', 'y', 'E', 'S', 'P', '3', '2', 'K',
                             'e', 'y', '1', '2', '3', '4', '5', '6'};

//For generating the password, 
// In the terminal run - python .\tools\wifi_password_manager.py
// Enter new password, it will update the registration.json file accordingly after encryption..

bool decrypt_password(char *finalStr) {
  const char *hex_input = registration_doc["oem"]["wifi_pw"];
  uint8_t encrypted[16];
  mbedtls_aes_context aes;

  // Check if hex_input is valid
  if (!hex_input) {
    g_Logger.Write(LogLevel::Error, LogCategory::WIFI, "decrypt_password",
                     "Missing wifi_pw in registration_doc");
    return (false);
  }

  // Convert hex string to byte array
  for (int i = 0; i < 16; i++) {
    sscanf(&hex_input[i * 2], "%2hhx", &encrypted[i]);
  }

  // Decrypt the password
  mbedtls_aes_init(&aes);
  mbedtls_aes_setkey_dec(&aes, aes_key, 128);
  mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_DECRYPT, encrypted,
                        (unsigned char *)finalStr);
  mbedtls_aes_free(&aes);

  finalStr[16] = '\0';  // ensure there's a null terminator (max 16 chars in 17-byte buffer)

  g_Logger.Write(LogLevel::Info, LogCategory::WIFI, "decrypt_password",
                   "Password decrypted successfully");
  return (true);
}
