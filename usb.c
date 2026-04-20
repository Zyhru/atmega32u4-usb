#include "usb.h"
#include <avr/io.h>
#include <avr/pgmspace.h>

/* Global Variables */
uint8_t dev_config_status;

typedef struct {
    ConfigDesc config;
    InterfaceDesc interface;
    HIDDesc hid;
    EndpointDesc endpoint;
} FullConfig;

const DeviceDesc device_descriptor PROGMEM = {
    .length = 18,
    .type = 1,
    .bcd_usb = 0x0200,
    .device_class = 0,
    .device_subclass = 0,
    .device_protocol = 0,
    .max_packet_size = 64,
    .vendor_id = VENDOR_ID,
    .product_id = PRODUCT_ID,
    .bcd_device = 0x01,
    .manufacturer = 0,
    .product = 0,
    .serial_number = 0,
    .num_configurations = 1
};

const ConfigDesc config_descriptor PROGMEM = {
    .len = 9,
    .dtype = 2,
    .clen = sizeof(FullConfig),
    .num_interfaces = 1,
    .config = 1,
    .iconfig = 0,
    .attributes = 0x80 | 0x20,
    .max_power = 500/2 
};

const EndpointDesc endpoint_desc PROGMEM = {
    .len = 7,
    .descriptor_type = 0x05,
    .endpoint_address = ENDPOINT_3 | 0x80, // IN/OUT Interrupt
    .attributes = 0x03,
    .max_packet_size = 8,
    .interval = 0x01,
};

const HIDDesc hid_descriptor PROGMEM = {
    .len = 9,
    .descriptor_type = 0x21,
    .bcd_hid = 0x0111,
    .country_code = 0,
    .num_descriptors = 1,
    .class_type = 0x22,
    .descriptor_length = sizeof(ReportDesc),
};

const InterfaceDesc interface_descriptor PROGMEM = {
    .len = 9,
    .descriptor_type = 4,
    .interface_number = 0,
    .alternate_setting = 0,
    .num_endpoints = 1,
    .interface_class = 0x03,
    .interface_sub_class = 0x01,
    .interface_protocol = 0x01,
    .interface = 0,
};

const FullConfig config PROGMEM = {
    .config = config_descriptor,
    .interface = interface_descriptor,
    .hid = hid_descriptor,
    .endpoint = endpoint_desc
};
    
const ReportDesc report PROGMEM = {

};

/* Private Functions */
static u8 wait_for_in_or_out();
static void wait_in();
static void clear_in();
static void stall();
static i32 send_config_descriptor();
static int setup_packet_recv();
static void set_ep(u8 data);
static void init_ep(u8 data);
static bool send_descriptor(SetupPacket *sp);

// Steps
// 1. Enable USB Pad Regulator (for power)
// 2. Configure PLL interface (Set to 16MHz) 
// 3. Enable PLL
// 4. Check PLL Lock
// 5. Enable USB interface
// 6. Enable USBE and USB pad
// 7. Set FRZCLK to 0
// 7. Wait for vbus info 
// 8. Attach usb device 
int usb_init() {
    cli(); // disabling global interrupts
    
    // enabling usb pad regulator
    UHWCON |= (1 << UVREGE);

    // setting clock source to 16MHz
    // enabling PLL
    //PLLCSR |= (1 << PINDIV | 1 << PLLE);
    PLLCSR |= (1 << PINDIV);
    PLLCSR |= (1 << PLLE);

    // Waiting for PLL
    while(!(PLLCSR & (1 << PLOCK)));
   
    // enabling USBE and usb pad
    USBCON |= (1 << USBE | 1 << OTGPADE);
    
    // start the usb clock
    USBCON &= ~(1 << FRZCLK);

    // Enabling speed, enabling the attach pull-ups
    UDCON &= ~((1 << LSM) | (1 << DETACH) | (1 << RSTCPU) | (1 << RMWKUP));


    // end of reset
    UDIEN |= (1 << EORSTI | 1 << SOFE | 1 << SUSPE);
    
    dev_config_status = 0; // not configured yet

    sei(); // enabling global interrupts
    return 0;
}

/* General Interrupt Vector */
ISR(USB_GEN_vect) {
    // after the end of reset
    // configure EP0
    uint8_t temp_udint = UDINT;
    UDINT = 0;
    if(temp_udint & (1 << EORSTI)) {
        
      
        /* ======================================================== */
                            /* Initializing EP0 */
        
        init_ep(0);
        
        /* ======================================================== */
        
        // check if ep0 is not configured
        if(!(UESTA0X & (1 << CFGOK))) {
            return;
        }

        UEIENX |= (1 << RXSTPE);
        return;
    }
}

/* Endpoint 0 Vector */
ISR(USB_COM_vect) {
    // Setting Endpoint 0
    set_ep(0);

    // is setup packet received?
    if(!setup_packet_recv()) {
        return;
    }

    SetupPacket sp;
    sp.request_type = UEDATX;
    sp.request = UEDATX;
   
    /* packing UEDATX into 16 bit values */
    sp.lb_value = UEDATX;
    sp.hb_value = UEDATX;

    sp.index = UEDATX;
    sp.index |= UEDATX << 8;

    sp.length = UEDATX;
    sp.length |= UEDATX << 8;

    // Reset SETUP, Received OUT, Transmitter Ready interupts
    // RXSTPE, RXOUTI, TXINI,
    // setting the transmit 
    UEINTX &= ~((1 << RXSTPI) | (1 << RXOUTI) | (1 << TXINI));
    
    if(sp.request_type & REQUEST_DEVICETOHOST) {
        wait_in();
    } else {
        clear_in(); 
    }

    // Handle Standard requests
    if(REQUEST_STANDARD == (sp.request_type & REQUEST_TYPE)) {
        if(SET_ADDRESS == sp.request) {
            wait_in();
            UDADDR = sp.lb_value | (1 << ADDEN);
            clear_in();
            return;
        } else if(GET_DESCRIPTOR == sp.request) {
            send_descriptor(&sp);
            return;
        } 
    }

    #if 0
    if(ok) {
        clear_in(); 
    } else {
        stall();
    }
    #endif
}

/* helper funcs */
static void init_ep(uint8_t data) {
    UENUM = data;
    UECONX |= (1 << EPEN);
    UECFG0X = 0x00; // OUT (Host -> Device)
    UECFG1X = 0x32; // 64 bank size
}

static int setup_packet_recv() {
    return UEINTX & (1 << RXSTPI);
}

static bool send_descriptor(SetupPacket *sp) {
    u8 type = sp->hb_value;
    
    if(type == DEVICE_DESCRIPTOR) {
        u8 *device_addr = (u8 *)&device_descriptor;
        u8 device_length = pgm_read_byte(device_addr);
        u16 length = sp->length;
        if(length > device_length) length = device_length;
        
        if(!wait_for_in_or_out()) return false;
        
        for(i32 i = 0; i < length; ++i) {
            UEDATX = pgm_read_byte(device_addr + i);
        }
        
        clear_in();     
    
    } else if(type == CONFIGURATION_DESCRIPTOR) {
        u8 *device_addr = (u8 * ) &config;
        u8 device_length = sizeof(FullConfig);
        u16 length = sp->length;
        
        // truncate
        if(length > device_length) length = device_length;
        if(!wait_for_in_or_out()) return false;
        
        for(i32 i = 0; i < length; ++i) {
            UEDATX = pgm_read_byte(device_addr + i);
        }
        
        clear_in();
    }
    
    return true;
}

// wait for IN or OUT
// If RXOUTI has aborted, then clear RXOUTI and return
static u8 wait_for_in_or_out(void) {
    while(!(UEINTX & ( (1 << TXINI) | (1 << RXOUTI))));
    return (UEINTX & (1 << RXOUTI)) == 0;
}

static void wait_in() {
    while (!(UEINTX & (1<<TXINI)));
}

static void clear_in() {
    UEINTX &= ~(1 << TXINI);
}

static void stall() {
    UECONX |= (1 << STALLRQ) | (1 << EPEN);
}

static i32 send_descriptor_data(u16 requested_length, void *data) {
        u8 *device_addr = (u8 * ) &data;
        u8 device_length = pgm_read_byte(device_addr);
        u16 length = requested_length;
        
        // truncate
        if(length > device_length) length = device_length;
        if(!wait_for_in_or_out()) return 1;
        
        for(i32 i = 0; i < length; ++i) {
            UEDATX = pgm_read_byte(device_addr + i);
        }
        
        clear_in();
        return 0;
}

static void set_ep(u8 data)  {
    UENUM = data;
}
