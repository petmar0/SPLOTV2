#SPLOT V2: The Sound Pressure Level Observing Transponder V2
##Introduction
In many situations, it becomes worthwhile to be able to transmit environmental data back from a field location in near-realtime. To that end, satellite-enabled systems are very useful in conveying data back from the field very quickly. In this case, this will be done with sound data and attendant environmental parameters.
###System Overview

The SPLOT is comprised of four subsystems:

- Sensing
- Power
- Satellite Communication
- Controller

####Sensing
The sensing subsystem has the [light](https://www.sparkfun.com/products/8688), [temperature and relative humidity](https://www.sparkfun.com/products/8257), and [barometric pressure](https://www.sparkfun.com/products/12039) sensors, along with the audio sensing subassembly. Audio sensing is carried out by a [Sound Detector board](https://www.sparkfun.com/products/12642) with its envelope pin wired to an analog input, and its audio output wired to a [Spectrum Shield](https://www.sparkfun.com/products/10306).
####Power
At the core of the power subsystem is the [LiPoly Charger](https://www.sparkfun.com/products/12711), which handles charging and loading of a [6 Ah LiPo Battery](https://www.sparkfun.com/products/8484). This is recharged by a [0.45 W Solar Panel](https://www.sparkfun.com/products/7845), while the rest of the subsystems are served by a [LiPower Boost Converter](https://www.sparkfun.com/products/10255) to keep the power rail at 5 VDC.
####Satellite Communication
Satellite communication is accomplished through the use of the [SatUplink](https://www.sparkfun.com/products/retired/11088) shield and a [SPOT Connect](http://www.findmespot.com/en/index.php?cid=116) beacon.
####Controller
Finally, the controller for the whole system is an [Arduino Uno R3](https://www.sparkfun.com/products/11021).
##Construction
###Spectrum Shield Modification
The Spectrum Shield uses the same two pins on the Arduino to control its MSGEQ-7 chip as the power control for the SatUplink Shield. To get around this, the traces for pins 4 and 5 are cut on the Spectrum Shield, and 30-gauge wire is used to connect these to pins 6 and 7, respectively. [image]()