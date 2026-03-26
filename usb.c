#include "usb.h"
#include <avr/io.h>
#include <stdint.h>

#define VENDOR_ID 0x03eb  
#define PRODUCT_ID 0x2FF4


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
    UDINT |= (1 << EORSTI | 1 << SOFI);
    
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
        // OUT (Host -> Device)
        UECFG0X = 0;
        UECFG1X = 0x32; // EP0 32 bytes, single
        
        // check if ep0 is not configured
        if(!(UESTA0X & (1 << CFGOK))) {
            dev_config_status = 0;
            return;
        }
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
    sp.bmRequestType = UEDATX;
    sp.bRequest = UEDATX;
   
    /* packing UEDATX into 16 bit values */
    sp.wValue = UEDATX;
    sp.wValue |= UEDATX << 8;

    sp.wIndex = UEDATX;
    sp.wIndex |= UEDATX << 8;

    sp.wLength = UEDATX;
    sp.wLength |= UEDATX << 8;

    // Reset SETUP, Received OUT, Transmitter Ready interupts
    // RXSTPE, RXOUTI, TXINI,
    UEINTX &= ~(1 << RXSTPE | 1 << RXOUTI | 1 << TXINI);

    // check for standard device request
    // check for SET_ADDRESS and GET_DESCRIPTOR
    // is it HOST TO DEVICE
    // 0000 0000
    if(sp.bmRequestType == 0x00) {
       switch(sp.bRequest) {
            case SET_ADDRESS:
                // STUB
            break;
            case GET_DESCRIPTOR:
                // STUB
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
