PSOC4_swd_programmer
====================

##SWD Programmer for PSoc 4 based devices.

To build the programmer you would need a basic FX2LP device. One from amazon or ebay based on CY7C68013A would do. Install the FX2LP SDK from Cypress and flash swd.iic from the FX2LP folder on the FX2LP device.

Connct:
```
    FX2LP       PSoc 4
    Device      Target
    ------      ---------------
    PD0    ---> SWDIO
    PD1    ---> SWDCL
    PB2    ---> XRES (or RESET)
    GND    ---> GND
    3.3V   ---> VCC
```
This will power the target as 3.3V from the FX2LP board. Pleas make sure there is no other power connected to the target as the FX2LP device is not 5V tollerant.

Use the PC software to flash a .hex file to the target.

Limitations:
- only tested on PSoc 4100 and 4200 (I don't have 4000 device)
- only tested on 32K flash device (16K devices should work but YMMV).

Building the software:

The PC project uses visual studio 2010, you can probably make it work with other platforms/compilers.

The device software needs the Keil PK51 toolchain. There is a compiled binary swd.iic you can use as well.