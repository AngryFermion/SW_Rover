#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Decrypts the Wi-Fi password from registration_doc["wifi_pw"] hex string
// Assumes AES-128 ECB mode and hex input length of 32 characters
// Output must be a buffer of at least 17 bytes (16 + null terminator)
bool decrypt_password(char *output);

#ifdef __cplusplus
}
#endif
