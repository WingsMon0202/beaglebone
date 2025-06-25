savedcmd_/home/wings/beaglebone/gpio_led_driver.mod := printf '%s\n'   gpio_led_driver.o | awk '!x[$$0]++ { print("/home/wings/beaglebone/"$$0) }' > /home/wings/beaglebone/gpio_led_driver.mod
