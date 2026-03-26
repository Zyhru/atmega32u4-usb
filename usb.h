#ifndef USB_H
#define USB_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

/* VID/PID */
#define VENDOR_ID 0x03eb  
#define PRODUCT_ID 0x2FF4

/* Endpoints */ 
#define FLASH_EP0_SINGLE_32

/* Global Variables */
uint8_t dev_config_status;

/* Standard Device Requests */


typedef enum {
    SET_ADDRESS = 0x05,
    GET_DESCRIPTOR = 0x7,
} DeviceRequests;

// HID Device Requests


/* structs */ 
typedef struct {
    uint8_t bmRequestType;
    uint8_t bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
} SetupPacket;

// configuration descriptor
// interface descriptor
// endpoint descriptor


int usb_init();
int usb_setup_packet_recv();
void usb_setendpoint_0(uint8_t data);

#endif // USB_H
