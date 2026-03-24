#ifndef USB_H
#define USB_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#define FLASH_EP0_SINGLE_32

uint8_t dev_config_status;


// Standard Device Requests


// HID Device Requests

// setup packet

// configuration descriptor
// interface descriptor
// endpoint descriptor


int usb_init();

#endif // USB_H
