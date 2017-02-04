# dalibor-display
This is a 6 Nixie Tube driver board.

CAUTION: I don't yet know if this works!

## Overview

This is a 6 Nixie Tube driver board driven by two 180V power supplies and two open-collector drivers (3 tubes on each driver). It is designed for the Dalibor Farny R|Z568M numberical Nixie Tube. If you have a custom Nixie Tube or a Nixie Tube from another vendor you should verify that it is compatible.

## Connections

There are two 5-pin 0.1 inch pitch headers on the back which may be used to daisy-chain a number of these boards together. The open-collector drivers use a shift register. Assert the latch pin and then clock in the data for all six Nixie Tubes and continue clocking in data for any additional boards in the daisy-chain.

This board requires up to 700mA @ 12V, so if multiple boards are daisy-chained then you might need to connect a dedicated 12V power supply to each board. Also, the M3 mount may be used as a power connector by jumpering the "solder link" on the bottom of the board (see label on bottom of board).

The 3 input signals (clock, data, latch) can be driven from 1.2V to 15V. The daisy-chain output "data pin" is 12V with a series 47k resistor. Just note this in case for some reason you're going to connect the last board of your daisy-chain back into a processor.

Input Connector:

|pin|signal|
|----|----|
|1|12 Volts|
|2|Latch|
|3|Clock|
|4|Data In|
|5|Ground|

Output (daisy-chain) Connector:

|pin|signal|
|----|----|
|1|12 Volts|
|2|Latch|
|3|Clock|
|4|Data Out|
|5|Ground|

Pin #1 is adjacent to the component designator.

## Dimensions

The board is 360mm by 75mm.

The mounting holes are designed to accomodate up to a 1/4-20 inch cap screw (1/4 inch finished hole size). Note that the mounting hole positions are not symetric:

|Y|X|
|-----|-----|
|9mm |8.5mm|
|9mm |119.5mm|
|9mm |240.5mm|
|9mm |351.5mm|
|66mm |8.5mm|
|66mm |119.5mm|
|66mm |240.5mm|
|66mm |351.5mm|

This board may be put adjacent to another board and still maintain 60mm spacing between all tubes.

## Manufacturing

If you don't have access to Altium Designer then you can take the the zip file in the output directory and just send that over to any regular PCB Manufacturer. Specify the board thickness (I used 0.092") and the color. This is a simple 2-layer board.

The Excel BOM file is a complete parts purchase list. There is a PDF of the schematics in the output directory for your reference.
