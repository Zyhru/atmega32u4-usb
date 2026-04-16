# TODO

- [ ] usb_init
    - [x] PLL Lock (48MHz)
    - [x] Reset bus
    - [x] Full Speed 
    - [x] Enable USB Controller, power pads,
    and pad regulator
    - [x] Attach USB controller
- [x] setup EP0
- address setup
    - [ ] enumeration
        - [x] handle GET_DESCRIPTOR
        - [x] handle SET_ADDRESS 
        - [ ] handle SET_CONFIGURATION 
        - [ ] handle GET_STATUS
        - [ ] handle HID descriptor


# Notes

MCU = ATMega32U4
Clock Speed = 16MHz
USB Clock Speed = 48MHz

## Data Transfer Flow

On power up the ATMega32U4 will intialize it's clock by locking PLL, starting the clock, enabling the speed,
enabling the attach pull-ups, and finally indicate the end of reset.

The USB will then go through it's enumeration phase and configure itself.


