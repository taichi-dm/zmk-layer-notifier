/*
 * BLE GATT transport for Layer Notifier
 *
 * Service UUID:        E6B30001-5AB5-4D09-B3A0-7D5E3A8C93F1
 * Characteristic UUID: E6B30002-5AB5-4D09-B3A0-7D5E3A8C93F1
 *
 * Characteristic: 1 byte (active layer index), Read + Notify
 * Security: BT_GATT_PERM_READ (暗号化不要、MVP)
 *
 * 参考: ZMK hog.c, ZMK Studio gatt_rpc_transport.c
 */

#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(layer_notifier);

/* --- UUIDs --- */

#define LN_SVC_UUID_VAL \
    BT_UUID_128_ENCODE(0xe6b30001, 0x5ab5, 0x4d09, 0xb3a0, 0x7d5e3a8c93f1)
#define LN_SVC_UUID BT_UUID_DECLARE_128(LN_SVC_UUID_VAL)

#define LN_CHAR_UUID_VAL \
    BT_UUID_128_ENCODE(0xe6b30002, 0x5ab5, 0x4d09, 0xb3a0, 0x7d5e3a8c93f1)
#define LN_CHAR_UUID BT_UUID_DECLARE_128(LN_CHAR_UUID_VAL)

/* --- State --- */

static uint8_t current_layer;

/* --- GATT callbacks --- */

static ssize_t read_layer(struct bt_conn *conn,
                          const struct bt_gatt_attr *attr,
                          void *buf, uint16_t len, uint16_t offset) {
    return bt_gatt_attr_read(conn, attr, buf, len, offset,
                             &current_layer, sizeof(current_layer));
}

/* --- Service definition --- */

BT_GATT_SERVICE_DEFINE(layer_notifier_svc,
    BT_GATT_PRIMARY_SERVICE(LN_SVC_UUID),
    BT_GATT_CHARACTERISTIC(LN_CHAR_UUID,
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
                           BT_GATT_PERM_READ,
                           read_layer, NULL, &current_layer),
    BT_GATT_CCC(NULL, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
);

/* --- Public API --- */

void layer_notifier_ble_send(uint8_t layer) {
    current_layer = layer;
    int err = bt_gatt_notify(NULL, &layer_notifier_svc.attrs[1],
                             &current_layer, sizeof(current_layer));
    if (err && err != -ENOTCONN) {
        LOG_WRN("BLE notify failed: %d", err);
    }
}
