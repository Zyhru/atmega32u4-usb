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
        - [ ] handle GET_DESCRIPTOR
        - [ ] handle SET_CONFIGURATION 
        - [ ] handle SET_ADDRESS 
        - [ ] handle GET_STATUS
        - [ ] handle HID descriptor


# Notes

MCU = ATMega32U4
Clock Speed = 16MHz
USB Clock Speed = 48MHz


## Data Transfer Flow

