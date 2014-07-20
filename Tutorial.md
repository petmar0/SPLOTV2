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
###Tools Needed
- Soldering Iron and Solder
- Tweezers
- Flush Cutter
- Wire Stripper (capable of 30 AWG)
- Hot Glue Gun with Hot Glue
- Hot Air Gun or Lighter

###Parts
- This [Wish List](https://www.sparkfun.com/wish_lists/91347)
- 6-pin Right Angle 0.1" Male Header
- 30 AWG Wire
- 2x 2-pin Right Angle 0.1" Male Header
- 2x 3-pin Straight 0.1" Male Header
- 2x 4-pin Straight 0.1" Male Header
- 5-pin Straight 0.1" Male Header
- 6-pin Straight 0.1" Male Header
- 20 Female-to-Female Jumpers
- 1/8" Heat Shrink Tubing
- 1/4" Heat Shrink Tubing
- [SPOT Connect](http://www.findmespot.com/shop/index.php?main_page=product_info&cPath=5&products_id=27)

###Spectrum Shield Modification
The Spectrum Shield uses the same two pins on the Arduino to control its MSGEQ-7 chip as the power control for the SatUplink Shield. To get around this, the traces for pins 4 and 5 are cut on the Spectrum Shield, and 30-gauge wire is used to connect these to pins 6 and 7, respectively.

![Pins 4 and 5 redirected to 6 and 7](https://raw.githubusercontent.com/petmar0/SPLOTV2/master/wiremods.png "Pins 4 and 5 redirected to 6 and 7")

Now solder in a 6-pin right-angle 0.1" header in the 8-13 pin through-holes. Solder straight 0.1" headers into the L/G/R pin through-holes. Finally, use 30-gauge wire and 0.1" straight headers to run the remaining analog pins over to the prototyping area on the board. Finally, solder in the 2-pin header and connect both pins to ground, and solder a 4-pin header into the prototyping area of the board, and connect the remaining four analog pins (2-5) to these pins.
###Pinning
Solder the remaining 0.1" headers into the sensing boards. Now attach the boards such that they conform to the following image:
![Wiring Image](https://raw.githubusercontent.com/petmar0/SPLOTV2/master/splotutorial-02.jpg)
####Left Side:
- A2: Sound Detector Envelope
- A3: Light Sensor Signal

####Right Side:
- Ground: SHT15 Ground
- Ground: Light Sensor Ground
- 13: Sound Detector Power
- 12: SHT15 Power
- 11: SHT15 SCLK
- 10: SHT15 SDAT
- 9: Light Sensor Power
- 8: NC
- 7: MSGEQ-7 Reset
- 6: MSGEQ-7 Strobe
- L: Sound Detector Audio
- G: Sound Detector Ground
- R: NC


###Stacking
Open the SPOT Connect and remove the transceiver board as per [this tutorial](https://www.sparkfun.com/tutorials/340). Stack the transceiver board on the SatUplink Shield, and stack this on top of the modified Spectrum Shield and the Arduino.

###Power
Splice the two JST connector cables, red to red, black to black, with 1/8" shrink tubing over the red wires, and 1/4" over the overall splice area. Shrink both with the hot air gun or lighter. Plug the battery and solar panel into the charger, as seen here:
![Charger with Battery and Panel](https://raw.githubusercontent.com/petmar0/SPLOTV2/master/splotutorial-05.jpg)
Solder the remaining 2-pin header into the load port on the boost converter, and attach the barrel jack as seen here:
![Barrel Jack on Boost Converter](https://raw.githubusercontent.com/petmar0/SPLOTV2/master/splotutorial-06.jpg)
Now plug the new dual JST cable into the charger load port, and into the boost converter. Finally, plug the barrel jack into the Arduino power port:
![Full Setup](https://raw.githubusercontent.com/petmar0/SPLOTV2/master/splotutorial-07.jpg)
##Code
Code can be found [here](https://github.com/petmar0/SPLOTV2/blob/master/SPLOTV2.ino).