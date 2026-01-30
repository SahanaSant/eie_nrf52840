/**
 * @file main.c
 */



#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>

#include <lvgl.h>

#include "BTN.h"
#include "LED.h"
#include <lv_data_obj.h>

#define SLEEP_MS 1

/*
  #define BLE_CUSTOM_SERVICE_UUID \
      BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef0)

  #define BLE_CUSTOM_CHARACTERISTIC_UUID \
      BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef2)

  static const struct bt_data ble_advertising_data[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS(BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME)-1),

  };
  static uint8_t ble_custom_characteristic_user_data[20] = {};
*/

static const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
static lv_obj_t *screen = NULL; 

int main(void) {
  if(!device_is_ready(display_dev)) {
    return 0;
  }
  screen = lv_screen_active(); 
  if(screen == NULL){
    return 0; 
  }

  if (0 > BTN_init()) {
    return 0;
  }
  if (0 > LED_init()) {
    return 0;
  }

  for (uint8_t i = 0; i < NUM_LEDS; i++){
    lv_obj_t *ui_btn = lv_button_create(screen); 
    //placing on a 2x2 grid in center, matching orientation of the LEDS
    lv_obj_align(ui_btn, LV_ALIGN_CENTER, 50 * (i % 2 ? 1 : -1), 20 * (i < 2 ? -1 : 1)); 
    lv_obj_t *button_label = lv_label_create(ui_btn); 
    char label_text[10]; 
    snprintf(label_text, 10, "LED %d", i);
    lv_label_set_text(button_label, label_text);
    lv_obj_align(button_label, LV_ALIGN_CENTER, 0, 0);  
  }


/*
  // For writing Hello World

  lv_obj_t *label = lv_label_create(screen);
  lv_label_set_text(label, "Hello World!"); 
*/


  display_blanking_off(display_dev);
  while(1) {
    lv_timer_handler(); 
    k_msleep(SLEEP_MS); 

  }
	return 0;
}
