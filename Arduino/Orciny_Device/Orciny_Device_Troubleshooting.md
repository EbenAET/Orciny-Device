# Orciny Device Physical Troubleshooting Guide

**Version:** V 0.9.0

---

## Assembly notes:
- Device has two internal batteries, only one of which is typically accessable.
- The inaccessable battery is a 3.7V LiPO cell stack that is connected to the microcontroller in the cavity under the pincer nozzle and should remain connected at all times.
- The accessable battery is a 5V "power bank" style unit attached in the broken end of the device.
- External troubleshooting is available for crew work, but Device Team should be contacted for internal work.

## Charging:
- Please unplug 5V battery between shows
- Use attached USB cable located above the core of the device to charge the 3.7V battery
- Plug in a MicroUSB cable into the power bank to charge the 5V battery

- **Note**: It is likely that the 5V battery will last for multiple shows, especially if it is unplugged when not in use. 
    The 3.7V internal battery should be plugged in daily since it can not be disconnected from the controller and will slowly drain over time.

## Arduino Notes:
- This program is written in the Arduino IDE and includes status prints over USB
- If you connect the device to a computer running the Arduino IDE, you can get internal statuses in the Serial Monitor at Baud 115220
    - This inculdes button presses, current status, and current step

## 1. Power & Battery Issues
- **Check Battery Connection:**
  - Be sure that 5V battery is plugged in
- **Charge Battery:**
  - Confirm it is fully charged.
  - Try powering via USB to rule out battery issues.
- **Power Switch:**
  - Confirm SW1 (Power) is toggled ON (see physical controls).

---

## 2. Device Sequence Issues
- **Sequence Out of Order:**
  - Press SW1 (Power) to toggle outputs OFF, then ON again to reset.
  - Hold SW1 + SW3 together for 5 seconds to reset to State 1 (outputs off).
  - If the sequence still seems incorrect, re-upload the firmware to the device.
- **Device Stuck in a State:**
  - Try toggling SW1 OFF/ON.
  - Hold SW1 + SW3 for a hard reset.
  - If unresponsive, power cycle the device (disconnect/reconnect battery or USB).
    - This can be done without opening the device by hooking device up to Arduino IDE and re-flashing the current code

---

## 3. Output & Animation Issues
- **No Lights or Effects:**
  - Confirm outputEnabled is ON (see Serial Monitor or LED indicator).
  - Check all physical connections to LEDs, servos, and NeoPixels.
  - Ensure the Prop-Maker FeatherWing is seated properly.
- **Servos Not Moving:**
  - Check servo power and signal wiring.
  - Ensure the PCA9685 servo wing is connected and powered.
  - Try swapping servos to rule out a faulty unit.
- **NeoPixels Not Lighting:**
  - Confirm correct data pin and ground connection.
  - Check that 5V battery is charged and plugged in.
  - Try a known-good NeoPixel strip.

---

## 4. Common Error Scenarios
- **Device Not Responding to Switches:** ~These are tasks for Device Team~
  - Check switch wiring and solder joints.
  - Test switches with a multimeter for continuity.
- **Random or Unstable Behavior:**
  - Check for loose wires or shorts.
  - Ensure all grounds are connected.
  - Try running from USB power only (remove battery) to rule out power noise.
- **Serial Monitor Not Showing Output:**
  - Confirm USB cable is data-capable (not charge-only).
  - Set baud rate to 115200 in Serial Monitor.
  - Ensure the correct COM port is selected.

---

## 5. Firmware & Software Checks
- **Re-upload Firmware:**
  - Use Arduino IDE or PlatformIO to upload the latest code.
  - Confirm successful upload (watch for errors in the IDE).
- **Check DeviceConfig.h:**
  - Verify pin assignments and tuning parameters match your hardware.
- **Library Dependencies:**
  - Ensure all required libraries are installed (see README or Arduino/libraries/).

---

## 6. Additional Tips
- **Document Observations:**
  - Note LED colors, servo movement, and sequence timing for troubleshooting.
- **Test One Subsystem at a Time:**
  - Disconnect servos, LEDs, or NeoPixels to isolate issues.
- **Consult the Serial Monitor:**
  - Add Serial.println() statements for additional debugging info.

---

## 7. Getting Help
- **Check README.md and TROUBLESHOOTING.md for updates.**
- **Contact Eben Alguire or Emery Smith.**
    - Please provide them with a detailed discription of the issue and what has been done so far

---

**See also:**
- arduino/Orciny_Device/Orciny_Device_Animation_Timeline.md
- arduino/Orciny_Device/DeviceConfig.h
- arduino/TROUBLESHOOTING.md
