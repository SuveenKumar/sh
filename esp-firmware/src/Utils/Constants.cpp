#include "Constants.h"
#include <pins_arduino.h>

const uint8_t LED_PINS[] = {D3, D4, D5, D6};
const uint8_t NUM_PINS = 4;
const uint8_t DNS_PORT = 53;
const uint8_t RESET_BUTTON_PIN = 0;
const unsigned long IDLE_TIMEOUT = 10000;
uint8_t BROADCAST_MAC_ADDRESS[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
