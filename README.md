# squawkbox_v1.0

### Twilio side set-up
- Register SIM & give it a name.
- Set SIM to "Active".
- Create a phone # for the SquawkBox.
- Make a new "send-message-to-device()" (This is specific to each SquawkBox).
- Link the new phone # to the new function.
- Add the new SIM SID to the new function.
- Load the new phone # onto the SD card in the SquawkBox in the "From.txt" file.

### SD card set-up
- Create:
  - "to1.txt","to2.txt","to3.txt", etc. (Save a single customer contact to each of these files format -> "6155555555").
  - "From.txt" (This is the SquawkBox's phone # format -> "From=%2b16155555555&").
  - "url.txt" (This is the twilio end pioint URL, saved in the backups).

### Parts-List
- Twilio
  - SIM card (Super SIM)
  - Phone #

- Rugged Circuits
  - Ruggeduino Mega
  - DIN rail mounts

- DF Robot
  - SIM7000A Module
  - Large GSM Antenna
  - Interface Cable SMA to U.FL

- Adafruit
  - microSD Breakout Board
  - RTC module PCF8523
  - Standard LCD 20x4
  - LCD I2C/SPI backpack

- OshPark / JLC PCB
  - PCB

- Automation Direct
  - Contactors (KPR-5CF-115VACDC-1)
  - Jumpers

- Amazon
  - 120VAC -> 12VDC Transformer - Mean Well MDR-60-12
  - RS-485 Module
  - Rotary Encoder
  - Screw terminals
  - 12VDC Barrel connectors
  - 3mm screws, washers
  - Pin headers male/female
  - SD card 32G

