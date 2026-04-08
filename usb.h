#ifndef USB_H
#define USB_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdbool.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;

/* VID/PID */
#define VENDOR_ID 0x03eb  
#define PRODUCT_ID 0x2FF4

/* Endpoints */ 
#define FLASH_EP0_SINGLE_32

#define DEVICETOHOST 0x80
#define HOSTTODEVICE 0x00

/* Standard Device Requests */
typedef enum {
    SET_ADDRESS = 0x05,
    GET_DESCRIPTOR = 0x06,
} DeviceRequests;

typedef enum { 
    DEVICE_DESCRIPTOR = 0x01,
    CONFIGURATION_DESCRIPTOR = 0x02,
} Descriptors;

// HID Device Requests

/* structs */ 
typedef struct {
    u8 request_type;
    u8 request;
    u8 lb_value;
    u8 hb_value;
    u16 index;
    u16 length;
} SetupPacket;

typedef struct {
    u8 length;
    u8 type;
    u16 bcd_usb;
    u8 device_class;
    u8 device_subclass;
    u8 device_protocol;
    u8 max_packet_size;
    u16 vendor_id;
    u16 product_id;
    u16 bcd_device;
    u8 manufacturer;
    u8 product;
    u8 serial_number;
    u8 num_configurations;
} DeviceDesc;

// configuration descriptor
// interface descriptor
// endpoint descriptor

int usb_init();
int usb_setup_packet_recv();
void usb_setendpoint_0(u8 data);
bool usb_get_descriptor(SetupPacket *sp);

#endif // USB_H
