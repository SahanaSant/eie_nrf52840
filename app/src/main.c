/**
 * @file main.c
 */



#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>

#include "BTN.h"
#include "LED.h"

#define SLEEP_MS 1


static uint8_t ble_custom_characteristic_user_data[20] = {};

int main(void) {
  if (0 > BTN_init()) {
    return 0;
  }
  if (0 > LED_init()) {
    return 0;
  }

  while (1) {
    k_msleep(SLEEP_MS);
  }
  return 0;
}
