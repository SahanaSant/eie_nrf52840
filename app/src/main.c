/*
 * main.c
 */



#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>

#include "BTN.h"
#include "LED.h"
#include "my_state_machine.h"

#define SLEEP_MS 1

#define BLE_CUSTOM_SERVICE_UUID \
    BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef0)

#define BLE_CUSTOM_CHARACTERISTIC_UUID \
    BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef2)

static const struct bt_data ble_advertising_data[] = {
  BT_DATA_BYTES(BT_DATA_FLAGS(BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
  BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME)-1),
};

static uint8_t ble_custom_characteristic_user_data[20] = {};

int main(void) {

  if (0 > BTN_init()) {
    return 0;
  }
  if (0 > LED_init()) {
    return 0;
  }

  state_machine_init();

  while(1) {

    int ret = state_machine_run();
    if(0>ret){
      return 0;
    }


    k_msleep(SLEEP_MS); 

  }
	return 0;
}