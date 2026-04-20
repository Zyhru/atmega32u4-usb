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
#define ENDPOINT_3 3 
#define ENDPOINT_4 4
#define ENDPOINT_5 5
#define ENDPOINT_6 6

#define REQUEST_DEVICETOHOST 0x80
#define REQUEST_HOSTTODEVICE 0x00

#define REQUEST_TYPE 0x60
#define REQUEST_STANDARD 0x00

/* Standard Device Requests */
typedef enum {
    SET_ADDRESS = 0x05,
    GET_DESCRIPTOR = 0x06,
} DeviceRequests;

typedef enum { 
    DEVICE_DESCRIPTOR = 0x01,
    CONFIGURATION_DESCRIPTOR = 0x02
} Descriptors;

typedef struct {
    u8 *descriptor;
    u8 length;
} DescData;

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
typedef struct {
	u8	len;			// 9
	u8	dtype;			// 2
	u16 clen;			// total length
	u8	num_interfaces;
	u8	config;
	u8	iconfig;
	u8	attributes;
	u8	max_power;
} ConfigDesc;

// interface descriptor
typedef struct {
    u8 len;
    u8 descriptor_type;
    u8 interface_number;
    u8 alternate_setting;
    u8 num_endpoints;
    u8 interface_class;
    u8 interface_sub_class;
    u8 interface_protocol;
    u8 interface;
} InterfaceDesc;

// HID Descriptor
typedef struct {
	u8 len; // Total size of the HID Descriptor
	u8 descriptor_type; // name specifying the type of HID descriptor
	u16 bcd_hid; // Numeric Expression identifying the HID Class
	u8 country_code; // Country code of localized hardware
	u8 num_descriptors; // number of class descriptors
	u8 class_type; // name of type of class descriptor
	u16 descriptor_length; // Total size of report descriptor
} HIDDesc;

typedef struct {
	u8 len; // Total size of the HID Descriptor
	u8 descriptor_type; // name specifying the type of HID descriptor
	u16 bcd_hid; // Numeric Expression identifying the HID Class
	u8 country_code; // Country code of localized hardware
	u8 num_descriptors; // number of class descriptors
	u8 class_type; // name of type of class descriptor
	u16 descriptor_length; // Total size of report descriptor
} ReportDesc;

// endpoint descriptor
typedef struct {
    u8 len;
    u8 descriptor_type;
    u8 endpoint_address;
    u8 attributes;
    u16 max_packet_size;
    u8 interval;
} EndpointDesc;

int usb_init();

#endif // USB_H
