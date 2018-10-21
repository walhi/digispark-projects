# Keyboard

A USB VIM Pedal keyboard example. Pressing a button generates a keystroke "i", releasing generates a keystroke "ESC".

## Hardware

A 10 kΩ resistor between PB2 pin and VCC is required. Button (normally open) connects to pin PB2 and GND.


## Usage

Build the program and flash it as usual to the digispark.

```bash
$ make clean && make
$ make upload
```

You can reboot the device for reflash by turning on the caps lock indicator more than five times.

After the program loads, you should see a new USB device:

```bash
$ lsusb
Bus 111 Device 222: ID 4242:e131 USB Design by Example
```

dmesg will also show the new keyboard:

```bash
$ dmesg | tail

[0001] usb 9-2.4: new low-speed USB device number 55 using xhci_hcd
[0002] usb 9-2.4: New USB device found, idVendor=4242, idProduct=e131
[0003] usb 9-2.4: New USB device strings: Mfr=1, Product=2, SerialNumber=0
[0004] usb 9-2.4: Product: VIM Pedal
[0005] usb 9-2.4: Manufacturer: obdev.at
[0006] input: obdev.at VIM Pedal as /devices/pci0000:00/0000:00:03.3/0000:03:00.0/usb9/9-2/9-2.4/9-2.4:1.0/0003:4242:E131.001D/input/input46
[0007] hid-generic 0003:4242:E131.001D: input,hidraw3: USB HID v1.01 Keyboard [obdev.at VIM Pedal] on usb-0000:03:00.0-2.4/input0

```

The canonical LED toggle is now controlled by the Caps Lock key!
