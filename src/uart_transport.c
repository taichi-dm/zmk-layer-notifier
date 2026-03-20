/*
 * USB CDC ACM transport for Layer Notifier
 *
 * Protocol:
 *   FW -> Host: "L:<layer>\n"  (layer change notification)
 *   Host -> FW: "Q\n"          (query current layer)
 *   FW -> Host: "L:<layer>\n"  (response to query)
 *
 * DTS chosen: zmk,layer-notifier-uart
 * 参考: ZMK Studio uart_rpc_transport.c
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/logging/log.h>

#include <zmk/keymap.h>

LOG_MODULE_DECLARE(layer_notifier);

#if !DT_HAS_CHOSEN(zmk_layer_notifier_uart)
#error "DTS chosen node 'zmk,layer-notifier-uart' is required for USB transport"
#endif

static const struct device *uart_dev = DEVICE_DT_GET(DT_CHOSEN(zmk_layer_notifier_uart));

/* --- TX --- */

static void send_layer(uint8_t layer) {
    char buf[8];
    int len = snprintf(buf, sizeof(buf), "L:%d\n", layer);
    for (int i = 0; i < len; i++) {
        uart_poll_out(uart_dev, buf[i]);
    }
}

/* --- RX (query handling) --- */

static uint8_t rx_buf[4];
static uint8_t rx_pos;

static void uart_isr(const struct device *dev, void *user_data) {
    ARG_UNUSED(user_data);

    while (uart_irq_update(dev) && uart_irq_rx_ready(dev)) {
        uint8_t c;
        if (uart_fifo_read(dev, &c, 1) != 1) {
            continue;
        }

        if (c == '\n') {
            if (rx_pos == 1 && rx_buf[0] == 'Q') {
                send_layer(zmk_keymap_highest_layer_active());
            }
            rx_pos = 0;
        } else if (rx_pos < sizeof(rx_buf)) {
            rx_buf[rx_pos++] = c;
        } else {
            /* overflow — reset */
            rx_pos = 0;
        }
    }
}

/* --- Public API --- */

void layer_notifier_uart_send(uint8_t layer) {
    if (!device_is_ready(uart_dev)) {
        return;
    }
    send_layer(layer);
}

void layer_notifier_uart_init(void) {
    if (!device_is_ready(uart_dev)) {
        LOG_ERR("UART device not ready");
        return;
    }

    uart_irq_callback_set(uart_dev, uart_isr);
    uart_irq_rx_enable(uart_dev);

    LOG_INF("UART transport ready");
}
