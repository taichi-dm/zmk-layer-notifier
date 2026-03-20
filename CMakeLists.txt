if(CONFIG_LAYER_NOTIFIER)

target_sources(app PRIVATE src/layer_notifier.c)

if(CONFIG_LAYER_NOTIFIER_USB)
  target_sources(app PRIVATE src/uart_transport.c)
endif()

if(CONFIG_LAYER_NOTIFIER_BLE)
  target_sources(app PRIVATE src/ble_transport.c)
endif()

endif()
