PSOC4_swd_programmer
====================

SWD Programmer for PSoc4 based devices.

To build the programmer you would need a basic FX2LP device. One from amazon or ebay based on CY7C68013A would do. Install the FX2LP SDK from Cypress and flash swd.iic from the FX2LP folder on the FX2LP device.

Connct:
<p> FX2LP
<p> Device ---> Target
<p> PD0    ---> SWDIO
<p> PD1    ---> SWDCL
<p> PB2    ---> XRES (or RESET)
<p> GND    ---> GND
<p> 3.3V   ---> VCC

This will power the target as 3.3V from the FX2LP board. Pleas make sure there is no other power connected to the target as the FX2LP device is not 5V tollerant.

Use the PC software to flash a .hex file to the target.

Limitations:
  -- only tested on PSoc 4100 (4200 should work YMMV)
  -- only tested on 32K flash device. For 16K you may need to modify the PC software.

Building the software:

The PC project uses visual studio 2010, you can probably make it work with other platforms/compilers.

The device software needs the Keil PK51 toolchain.