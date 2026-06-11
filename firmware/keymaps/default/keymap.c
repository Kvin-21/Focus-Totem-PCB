#include QMK_KEYBOARD_H

enum totem_keys {
    TM_TOGGLE = SAFE_RANGE,  // start / pause
    TM_RESET,                // back to the set time
    TM_BREAK                 // short break preset
};

enum phase { SETTING, RUNNING, PAUSED, DONE };

static uint8_t  phase       = SETTING;
static bool     on_break    = false;
static uint8_t  set_minutes = 25;
static uint16_t remaining   = 25 * 60;   // seconds left in the session
static uint32_t tick        = 0;

static const uint8_t break_mins[] = { 5, 10, 15 };
static uint8_t break_idx = 0;

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT(
        TM_TOGGLE, TM_RESET, TM_BREAK, TM_TOGGLE
    )
};

static void run(uint8_t minutes, bool brk) {
    on_break  = brk;
    remaining = minutes * 60;
    phase     = RUNNING;
    tick      = timer_read32();
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (!record->event.pressed) {
        return true;
    }
    switch (keycode) {
        case TM_TOGGLE:
            if (phase == RUNNING) {
                phase = PAUSED;
            } else {
                if (phase == SETTING) {
                    remaining = set_minutes * 60;
                }
                phase = RUNNING;
                tick  = timer_read32();
            }
            return false;
        case TM_RESET:
            phase     = SETTING;
            on_break  = false;
            remaining = set_minutes * 60;
            return false;
        case TM_BREAK:
            break_idx = (break_idx + 1) % ARRAY_SIZE(break_mins);
            run(break_mins[break_idx], true);
            return false;
    }
    return true;
}

bool encoder_update_user(uint8_t index, bool clockwise) {
    if (phase == SETTING) {
        if (clockwise && set_minutes < 99) {
            set_minutes++;
        } else if (!clockwise && set_minutes > 1) {
            set_minutes--;
        }
        remaining = set_minutes * 60;
    }
    return false;
}

static void set_leds(void) {
    uint8_t hue, val;
    if (phase == DONE) {                       // flash on completion
        hue = 0;
        val = (timer_read32() / 250) % 2 ? RGBLIGHT_LIMIT_VAL : 0;
    } else if (phase == RUNNING && remaining < 60) {
        hue = 0;                               // red for the last minute
        val = RGBLIGHT_LIMIT_VAL;
    } else if (phase == RUNNING) {
        hue = 85;                              // green while counting
        val = RGBLIGHT_LIMIT_VAL;
    } else {                                   // amber breathe while idle
        uint16_t t = timer_read32() % 2000;
        hue = 21;
        val = (t < 1000 ? t : 2000 - t) * RGBLIGHT_LIMIT_VAL / 1000;
    }
    rgblight_sethsv_noeeprom(hue, 255, val);
}

void housekeeping_task_user(void) {
    if (phase == RUNNING && timer_elapsed32(tick) >= 1000) {
        tick = timer_read32();
        if (remaining > 0) {
            remaining--;
        }
        if (remaining == 0) {
            phase = DONE;
        }
    }

    static uint32_t led_at = 0;
    if (timer_elapsed32(led_at) >= 16) {
        led_at = timer_read32();
        set_leds();
    }
}

void keyboard_post_init_user(void) {
    rgblight_enable_noeeprom();
    rgblight_mode_noeeprom(RGBLIGHT_MODE_STATIC_LIGHT);
    tick = timer_read32();
}

#ifdef OLED_ENABLE
oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return OLED_ROTATION_0;
}

bool oled_task_user(void) {
    const char *label = on_break ? "BREAK" : "FOCUS";
    if (phase == DONE) {
        label = "DONE ";
    } else if (phase == SETTING) {
        label = "SET  ";
    }

    char clock[6];
    clock[0] = '0' + remaining / 600;
    clock[1] = '0' + remaining / 60 % 10;
    clock[2] = ':';
    clock[3] = '0' + remaining % 60 / 10;
    clock[4] = '0' + remaining % 60 % 10;
    clock[5] = '\0';

    oled_set_cursor(0, 0);
    oled_write(label, false);
    oled_set_cursor(0, 2);
    oled_write(clock, false);
    return false;
}
#endif
