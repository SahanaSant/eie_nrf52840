/*
 * main.c
 */

#include <inttypes.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/settings/settings.h>
#include <zephyr/sys/printk.h>

#include "BTN.h"
#include "LED.h"

/* -------------------------------------------------------------------------- */
/* Macros                                                                     */
/* -------------------------------------------------------------------------- */

#define SLEEP_MS 1

#define BLE_CUSTOM_SERVICE_UUID \
    BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef0)

#define BLE_CUSTOM_CHARACTERISTIC_UUID \
    BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef2)

#define BLE_CUSTOM_CHARACTERISTIC_MAX_DATA_LENGTH 20

/* -------------------------------------------------------------------------- */
/* Forward declarations / storage                                             */
/* -------------------------------------------------------------------------- */

static ssize_t ble_custom_characteristic_read_cb(struct bt_conn *conn,
                                                 const struct bt_gatt_attr *attr,
                                                 void *buf, uint16_t len,
                                                 uint16_t offset);

static ssize_t ble_custom_characteristic_write_cb(struct bt_conn *conn,
                                                  const struct bt_gatt_attr *attr,
                                                  const void *buf, uint16_t len,
                                                  uint16_t offset, uint8_t flags);


static uint8_t ble_custom_characteristic_user_data
    [BLE_CUSTOM_CHARACTERISTIC_MAX_DATA_LENGTH + 1] = { 'E', 'i', 'E', 0 };

static const struct bt_uuid_128 ble_custom_service_uuid = BT_UUID_INIT_128(BLE_CUSTOM_SERVICE_UUID);
static const struct bt_uuid_128 ble_custom_characteristic_uuid = BT_UUID_INIT_128(BLE_CUSTOM_CHARACTERISTIC_UUID);

/* -------------------------------------------------------------------------- */
/* Advertising data                                                           */
/* -------------------------------------------------------------------------- */
static const struct bt_data ble_advertising_data[] = {
    /* Flags: general discoverable, BLE only */
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),

    /* Device name from Kconfig (CONFIG_BT_DEVICE_NAME) */
    BT_DATA(BT_DATA_NAME_COMPLETE,
            CONFIG_BT_DEVICE_NAME,
            sizeof(CONFIG_BT_DEVICE_NAME) - 1),
};


/* -------------------------------------------------------------------------- */
/* GATT service & characteristic                                              */
/* -------------------------------------------------------------------------- */
BT_GATT_SERVICE_DEFINE(
    ble_custom_service,
    BT_GATT_PRIMARY_SERVICE(&ble_custom_service_uuid),

    BT_GATT_CHARACTERISTIC(
        &ble_custom_characteristic_uuid.uuid,
        BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
        BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
        ble_custom_characteristic_read_cb,
        ble_custom_characteristic_write_cb,
        ble_custom_characteristic_user_data
    )
);

/* -------------------------------------------------------------------------- */
/* Characteristic callbacks                                                   */
/* -------------------------------------------------------------------------- */


static ssize_t ble_custom_characteristic_read_cb(struct bt_conn *conn,
                                                 const struct bt_gatt_attr *attr,
                                                 void *buf, uint16_t len,
                                                 uint16_t offset)
{
    const uint8_t *value = attr->user_data;
    size_t value_len = strnlen((const char *)value,
                               BLE_CUSTOM_CHARACTERISTIC_MAX_DATA_LENGTH);

    return bt_gatt_attr_read(conn, attr, buf, len, offset, value, value_len);
}

static ssize_t ble_custom_characteristic_write_cb(struct bt_conn *conn,
                                                  const struct bt_gatt_attr *attr,
                                                  const void *buf, uint16_t len,
                                                  uint16_t offset, uint8_t flags)
{
    uint8_t *value_ptr = attr->user_data;

    if (offset + len > BLE_CUSTOM_CHARACTERISTIC_MAX_DATA_LENGTH) {
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }

    memcpy(value_ptr + offset, buf, len);
    value_ptr[offset + len] = '\0';   /* char, not string */

    printk("[BLE] write (%u bytes): \"%s\"\n", len, (char *)value_ptr);

    return len;
}

/* -------------------------------------------------------------------------- */
/* main                                                                       */
/* -------------------------------------------------------------------------- */


int main(void) {

  if (0 > BTN_init()) {
    return 0;
  }
  if (0 > LED_init()) {
    return 0;
  }
  
int error = bt_enable(NULL);
    if (error) {
        printk("Bluetooth init failed (error %d)\n", error);
        return 0;
    }
    printk("Bluetooth initialized\n");

    error = bt_le_adv_start(BT_LE_ADV_CONN_FAST_1,
                            ble_advertising_data, ARRAY_SIZE(ble_advertising_data),
                            NULL, 0);
    if (error) {
        printk("Advertising failed (error %d)\n", error);
        return 0;
    }
    printk("Advertising started\n");

    while (1) {
        k_msleep(SLEEP_MS);
    }

    return 0;
}
