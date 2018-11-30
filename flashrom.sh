#!/bin/bash
. ${IDF_PATH}/add_path.sh
${IDF_PATH}/components/esptool_py/esptool/esptool.py --chip esp32 --port "/dev/ttyUSB0" --baud $((460800)) write_flash -fs 4MB 0x100000 "$1"
