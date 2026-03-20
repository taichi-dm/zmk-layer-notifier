/*
 * ZMK Layer Notifier
 *
 * zmk_layer_state_changed イベントを購読し、アクティブレイヤーを
 * USB CDC ACM / BLE GATT 経由で macOS に通知する。
 *
 * パターン参考: zmk-rgbled-widget widget.c L342-344
 */

#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>

#include <zmk/event_manager.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/keymap.h>

LOG_MODULE_REGISTER(layer_notifier, CONFIG_LAYER_NOTIFIER_LOG_LEVEL);

/* --- Transport forward declarations --- */

#if IS_ENABLED(CONFIG_LAYER_NOTIFIER_USB)
extern void layer_notifier_uart_send(uint8_t layer);
extern void layer_notifier_uart_init(void);
#endif

#if IS_ENABLED(CONFIG_LAYER_NOTIFIER_BLE)
extern void layer_notifier_ble_send(uint8_t layer);
#endif

/* --- Debounce work (16 ms) --- */

static uint8_t pending_layer;

static void layer_notify_work_handler(struct k_work *work) {
    uint8_t layer = pending_layer;
    LOG_INF("layer=%d", layer);

#if IS_ENABLED(CONFIG_LAYER_NOTIFIER_USB)
    layer_notifier_uart_send(layer);
#endif

#if IS_ENABLED(CONFIG_LAYER_NOTIFIER_BLE)
    layer_notifier_ble_send(layer);
#endif
}

static K_WORK_DELAYABLE_DEFINE(layer_notify_work, layer_notify_work_handler);

/* --- ZMK event listener --- */

static int layer_notifier_cb(const zmk_event_t *eh) {
    pending_layer = zmk_keymap_highest_layer_active();
    k_work_reschedule(&layer_notify_work, K_MSEC(16));
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(layer_notifier, layer_notifier_cb);
ZMK_SUBSCRIPTION(layer_notifier, zmk_layer_state_changed);

/* --- Init --- */

static int layer_notifier_init(void) {
#if IS_ENABLED(CONFIG_LAYER_NOTIFIER_USB)
    layer_notifier_uart_init();
#endif
    LOG_INF("ready");
    return 0;
}

SYS_INIT(layer_notifier_init, APPLICATION);
