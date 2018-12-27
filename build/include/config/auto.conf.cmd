deps_config := \
	/home/pebri/Projects/ESP32/esp-idf/components/app_trace/Kconfig \
	/home/pebri/Projects/ESP32/esp-idf/components/aws_iot/Kconfig \
	/home/pebri/Projects/ESP32/esp-idf/components/bt/Kconfig \
	/home/pebri/Projects/ESP32/esp-idf/components/driver/Kconfig \
	/home/pebri/Projects/ESP32/esp-idf/components/esp32/Kconfig \
	/home/pebri/Projects/ESP32/esp-idf/components/esp_adc_cal/Kconfig \
	/home/pebri/Projects/ESP32/esp-idf/components/esp_http_client/Kconfig \
	/home/pebri/Projects/ESP32/esp-idf/components/ethernet/Kconfig \
	/home/pebri/Projects/ESP32/esp-idf/components/fatfs/Kconfig \
	/home/pebri/Projects/ESP32/esp-idf/components/freertos/Kconfig \
	/home/pebri/Projects/ESP32/esp-idf/components/heap/Kconfig \
	/home/pebri/Projects/ESP32/esp-idf/components/libsodium/Kconfig \
	/home/pebri/Projects/ESP32/esp-idf/components/log/Kconfig \
	/home/pebri/Projects/ESP32/esp-idf/components/lwip/Kconfig \
	/home/pebri/Projects/ESP32/esp-idf/components/mbedtls/Kconfig \
	/home/pebri/Projects/ESP32/esp-idf/components/openssl/Kconfig \
	/home/pebri/Projects/ESP32/esp-idf/components/pthread/Kconfig \
	/home/pebri/Projects/ESP32/esp-idf/components/spi_flash/Kconfig \
	/home/pebri/Projects/ESP32/esp-idf/components/spiffs/Kconfig \
	/home/pebri/Projects/ESP32/esp-idf/components/tcpip_adapter/Kconfig \
	/home/pebri/Projects/ESP32/esp-idf/components/vfs/Kconfig \
	/home/pebri/Projects/ESP32/esp-idf/components/wear_levelling/Kconfig \
	/home/pebri/Projects/ESP32/esp-idf/components/bootloader/Kconfig.projbuild \
	/home/pebri/Projects/ESP32/esp-micrones-gb/components/drivers/Kconfig.projbuild \
	/home/pebri/Projects/ESP32/esp-idf/components/esptool_py/Kconfig.projbuild \
	/home/pebri/Projects/ESP32/esp-idf/components/partition_table/Kconfig.projbuild \
	/home/pebri/Projects/ESP32/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)

ifneq "$(IDF_CMAKE)" "n"
include/config/auto.conf: FORCE
endif

$(deps_config): ;
