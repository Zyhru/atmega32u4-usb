#include "usb.h"

/* Global Variables */
uint8_t dev_config_status;

static const DeviceDesc device_descriptor PROGMEM = {
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

/* Private Functions */
static u8 wait_for_in_or_out();


// Steps
// 1. Enable USB Pad Regulator (for power)
// 2. Configure PLL interface (Set to 16MHz) 3. Enable PLL
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
    PLLCSR |= (1 << PINDIV | 1 << PLLE);

    // Waiting for PLL
    while(!(PLLCSR & (1 << PLOCK)));
   
    // enabling USBE and usb pad
    USBCON |= (1 << USBE | 1 << OTGPADE);
    
    // unfreeze the clock
    USBCON &= ~(1 << FRZCLK);

    // set speed = Full Speed
    UDCON &= ~(1 << LSM);

    // attach device
    UDCON &= ~(1 << DETACH);

    // end of reset
    UDIEN |= (1 << EORSTI | 1 << SOFE);
    
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
        // configure ep0
        usb_setendpoint_0(0);

        // activating the endpoint
        UECONX |= (1 << EPEN);

        // configure the direction
        // configure the endpoint type
        UECFG0X = 0x00; // OUT (Host -> Device)
        UECFG1X = (1 << EPSIZE1) | (1 << EPSIZE0) | (1 << ALLOC);
        
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
    usb_setendpoint_0(0);

    // is setup packet received?
    if(!usb_setup_packet_recv()) {
        return;
    }

    // Read the UEDAT
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
    UEINTX &= ~(1 << RXSTPE | 1 << RXOUTI | 1 << TXINI);

    // check for standard device request
    // check for SET_ADDRESS and GET_DESCRIPTOR
    // is it HOST TO DEVICE
    // 0000 0000
    if(sp.request_type == DEVICETOHOST) {
        switch(sp.request) {
            case SET_ADDRESS:
            break;
        }
    } else if(sp.request_type == DEVICETOHOST) {
       switch(sp.request) {
            case GET_DESCRIPTOR:
                usb_get_descriptor(&sp);
                break;
        
        }
    }
}

/* helper funcs */
void usb_setendpoint_0(uint8_t data) {
    UENUM = data;
}

int usb_setup_packet_recv() {
    return (UEINTX & (1 << RXSTPI));
}

bool usb_get_descriptor(SetupPacket *sp) {
    u8 type = sp->hb_value;
    if(type == DEVICE_DESCRIPTOR) {
        u8 *device_addr = (u8 *)&device_descriptor;
        u8 device_length = pgm_read_byte(device_addr);
        
        if(!wait_for_in_or_out()) return false;
       
        //TODO: Handle 9.4.3 GET_DESCRIPTOR packet length
        // If the descriptor is shorter than the wLength field, the
        // device indicates the end of the control transfer by sending a short packet when further data is requested. A
        // short packet is defined as a packet shorter than the maximum payload size or a zero length data packet
       
        // setting each field of device descriptor to UEDATX 
        for(i32 i = 0; i < device_length; ++i) {
            UEDATX = pgm_read_byte(device_addr + i);
        }

        // clear TXINI to send each byte to host
        UEINTX &= ~(1 << TXINI);
    }

    return true;
}

// wait for IN or OUT
// If RXOUTI has aborted, then clear RXOUTI and return
static u8 wait_for_in_or_out(void) {
    while(!(UEINTX & ( (1 << TXINI) | (1 << RXOUTI) )));
    return (UEINTX & (1 << RXOUTI)) == 0;
}
