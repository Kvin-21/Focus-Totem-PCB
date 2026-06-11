# Focus Totem

A desk totem that runs a Pomodoro/study timer. Twist the encoder to dial in the minutes, press to start, and the two side LEDs breathe along with the session —
amber while setting up, green while the clock is running, red for the last minute, and then a flash when done. The OLED shows the live countdown and where you are (FOCUS / BREAK / DONE).

It's built around a Seeed XIAO RP2040 and runs QMK.

## Controls

| Input            | Does                                  |
|------------------|---------------------------------------|
| Encoder turn     | Set the minutes                       |
| Encoder press    | Start / pause (same as Key 1)         |
| Key 1            | Start / pause                         |
| Key 2            | Reset to the set time                 |
| Key 3            | Cycle the short-break preset          |

## Materials

| Ref      | Part                                   | Notes                        | Qty |
|----------|----------------------------------------|------------------------------|-----|
| U1       | Seeed XIAO RP2040                      | the brains                   | 1   |
| SW1–SW3  | MX-compatible switch                   | the three keys               | 3   |
| SW4      | Alps EC11 rotary encoder (with switch) | dial + push                  | 1   |
| J1       | 0.91" SSD1306 OLED, 128×32, I²C        | 4-pin module                 | 1   |
| D1, D2   | SK6812 MINI-E                          | addressable RGB              | 2   |
| R1, R2   | 4.7 kΩ, 0805                           | I²C pull-ups                 | 2   |
| R3       | 470 Ω, 0805                            | series on the LED data line  | 1   |
| C1       | 100 nF, 0805                           | OLED decoupling              | 1   |
| C2       | 10 µF, 0805                            | LED bulk                     | 1   |

## Wiring / pinout

| Pad  | RP2040 | Goes to                          |
|------|--------|----------------------------------|
| D0   | GP26   | Key 1                            |
| D1   | GP27   | Key 2                            |
| D2   | GP28   | Key 3                            |
| D3   | GP29   | Encoder switch                   |
| D4   | GP6    | OLED SDA (I²C1)                  |
| D5   | GP7    | OLED SCL (I²C1)                  |
| D6   | GP0    | Encoder A                        |
| D7   | GP1    | Encoder B                        |
| D10  | GP3    | SK6812 data in                   |
| 5V   | —      | LED V+, C2                       |
| 3V3  | —      | OLED V+, I²C pull-ups, C1        |
| GND  | —      | common ground                    |

## Layout

```
hardware/    KiCad project — schematic, routed board, project + libraries
firmware/    QMK keyboard folder (keyboard.json, config.h, rules.mk, keymaps/)
```

The board is a 70 × 72 mm two-layer design with a ground pour on both sides and M3 mounting holes in the corners. The XIAO sits on the bottom edge so its USB port lines up with the case cut-out.

## Firmware

The firmware is a QMK keyboard.

```sh
# from the root of qmk_firmware checkout
cp -r focus-totem-pcb/firmware keyboards/focus_totem
qmk compile -kb focus_totem -km default
```

To flash later: plug the XIAO in, double-tap its reset/boot so it mounts as a USB drive, and copy the `.uf2`. It'll reboot into the firmware on its own.
