/*
    main.c
*/

// #include <zephyr/kernel.h>
// #include<zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
// #include <zephyr/device.h>
// #include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
// #include <inttypes.h>

#include "BTN.h"
#include "LED.h"

#define SLEEP_MS 500


int main (void)
{
  int count_b0 = 0,count_b1 = 0, count_b2 = 0, count_b3 = 0;
  int result =0;
  if (0>BTN_init()){
    return 0;
  }
  if (0>LED_init()){
    return 0;
  }
  while (1)
  {
    //button 0, led 0
    if (BTN_check_clear_pressed(BTN0)){
      result = LED_toggle(LED0);
      if (result != 0 && count_b0<16){
        count_b0++;
        printk("BUTTON 0 PRESSED");
      }
      else if (count_b0>=16)
        count_b0=0;
    } 
    //button 1, led 1
    if (BTN_check_clear_pressed(BTN1)){
      result = LED_toggle(LED1);
      if (result != 0 && count_b1<16){
        count_b1++;
        printk("BUTTON 1 PRESSED");
      }
      else  if(count_b1>=16)
        count_b1=0;
    }
    //button 2, led 2
    if (BTN_check_clear_pressed(BTN2)){
      result = LED_toggle(LED2);
      if (result != 0 && count_b2<16){
        count_b2++;
        printk("BUTTON 2 PRESSED");
      }
      else if (count_b2>=16)
        count_b2=0; 
    }
    //button 3, led 3
    if (BTN_check_clear_pressed(BTN3)){
      result = LED_toggle(LED3);
      if (result != 0 && count_b3<16){
        count_b3++;
        printk("BUTTON 3 PRESSED");
      }
      else if (count_b3>=16)
        count_b3=0; 
    }
    k_msleep(SLEEP_MS);
  }
          
  return 0;
}