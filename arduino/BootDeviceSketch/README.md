arduino-cli compile --fqbn arduino:avr:nano BootDeviceSketch/BootDeviceSketch.ino

arduino-cli upload -p /dev/cu.usbserial-10 --fqbn arduino:avr:nano:cpu=atmega328old BootDeviceSketch/BootDeviceSketch.ino

arduino-cli monitor -p /dev/cu.usbserial-10 -c baudrate=115200
