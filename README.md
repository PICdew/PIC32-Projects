# PIC Projects

This repository contains some real projects with the ```PIC Microcontroller```. Althought the repository is named PIC32,
there are other PIC based projects here.

## LCD + UART type-what-ever-you-want

Implements two things:

* Full duplex UART with a host machine. Echos back the text sent by UART
* Sends the same text for display on a ``16x2 HD44780 LCD```

Hardware:

```
PIC18F4520 on PICDEM2 Plus board 
16x2 4-bit interface LCD
```

## SD Card driver

Implements a driver to read and write to SD/SDHC cards

Hardware:
```
PIC32MX795F512L USB Starter Kit II, PICtail SD Card
```

I've tested this on SDHC Kingston 8GB. Note that some SD cards may not work as the initialization routine is sometimes different when you change card types.



