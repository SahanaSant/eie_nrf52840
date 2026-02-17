#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/sys/printk.h>
#include <lvgl.h>

int main(void)
{
    const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
    if (!device_is_ready(display_dev)) {
        printk("display not ready\n");
        while (1) { k_msleep(1000); }
    }

    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0xFF0000), LV_PART_MAIN);

    lv_obj_t *label = lv_label_create(scr);
    lv_label_set_text(label, "EQ FORCE 777");
    lv_obj_set_style_text_color(label, lv_color_black(), LV_PART_MAIN);
    lv_obj_center(label);

    lv_timer_handler();              /* render one frame first */
    display_blanking_off(display_dev);

    while (1) {
        lv_timer_handler();
        k_msleep(10);
    }
}
