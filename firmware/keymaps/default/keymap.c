#include QMK_KEYBOARD_H

enum totem_keys {
    TM_TOGGLE = SAFE_RANGE,  // start / pause
    TM_RESET,                // hold to go back to the set time
    TM_BREAK                 // short break preset
};

enum phase { SETTING, RUNNING, PAUSED, DONE };

#define RESET_HOLD_MS 600
#define IDLE_MS       60000
#define ENC_GUARD_MS  5
#define BREATHE_MS    4000

static uint8_t  phase       = SETTING;
static bool     on_break    = false;
static uint8_t  set_minutes = 25;
static uint16_t total       = 25 * 60;   // length of the current session
static uint16_t remaining   = 25 * 60;
static uint32_t tick        = 0;
static uint32_t last_act    = 0;
static uint32_t reset_since = 0;         // 0 when key 2 is not held
static bool     asleep      = false;

static const uint8_t break_mins[] = { 5, 10, 15 };
static uint8_t break_idx = 0;

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT(
        TM_TOGGLE, TM_RESET, TM_BREAK, TM_TOGGLE
    )
};

static void begin(uint8_t minutes, bool brk) {
    on_break  = brk;
    total     = minutes * 60;
    remaining = total;
    phase     = RUNNING;
    tick      = timer_read32();
}

static void wake(void) {
    last_act = timer_read32();
    if (asleep) {
        asleep = false;
        oled_on();
    }
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        wake();
    }
    switch (keycode) {
        case TM_TOGGLE:
            if (!record->event.pressed) {
                return false;
            }
            if (phase == RUNNING) {
                phase = PAUSED;
            } else if (remaining == 0) {
                begin(on_break ? break_mins[break_idx] : set_minutes, on_break);
            } else {
                phase = RUNNING;
                tick  = timer_read32();
            }
            return false;
        case TM_RESET:
            reset_since = record->event.pressed ? timer_read32() : 0;
            return false;
        case TM_BREAK:
            if (!record->event.pressed) {
                return false;
            }
            break_idx = (break_idx + 1) % ARRAY_SIZE(break_mins);
            begin(break_mins[break_idx], true);
            return false;
    }
    return true;
}

bool encoder_update_user(uint8_t index, bool clockwise) {
    static uint32_t last_step = 0;
    wake();
    if (timer_elapsed32(last_step) < ENC_GUARD_MS) {
        return false;   // shrug off contact bounce between detents
    }
    last_step = timer_read32();
    if (phase == SETTING) {
        if (clockwise && set_minutes < 99) {
            set_minutes++;
        } else if (!clockwise && set_minutes > 1) {
            set_minutes--;
        }
        total = remaining = set_minutes * 60;
    }
    return false;
}

static void set_leds(void) {
    if (asleep) {
        rgblight_sethsv_noeeprom(0, 0, 0);
        return;
    }
    uint8_t hue, val;
    if (phase == DONE) {
        hue = 0;                                   // flash red on completion
        val = (timer_read32() / 250) % 2 ? RGBLIGHT_LIMIT_VAL : 0;
    } else if (phase == RUNNING && remaining <= 60) {
        hue = 0;                                   // red through the last minute
        val = RGBLIGHT_LIMIT_VAL;
    } else if (phase == RUNNING) {
        hue = 85;                                  // green while counting
        val = RGBLIGHT_LIMIT_VAL;
    } else {
        uint16_t t = timer_read32() % BREATHE_MS;          // amber breathe while idle
        uint8_t tri = (t < BREATHE_MS / 2 ? t : BREATHE_MS - t) * 255UL / (BREATHE_MS / 2);
        hue = 21;
        val = (uint32_t)tri * tri * RGBLIGHT_LIMIT_VAL / (255UL * 255);   // eased, no libm
    }
    rgblight_sethsv_noeeprom(hue, 255, val);
}

void housekeeping_task_user(void) {
    if (reset_since && timer_elapsed32(reset_since) >= RESET_HOLD_MS) {
        reset_since = 0;
        phase       = SETTING;
        on_break    = false;
        total = remaining = set_minutes * 60;
    }

    if (phase == RUNNING && timer_elapsed32(tick) >= 1000) {
        tick = timer_read32();
        if (remaining > 0) {
            remaining--;
        }
        if (remaining == 0) {
            phase = DONE;
        }
    }

    bool idle = phase != RUNNING && timer_elapsed32(last_act) >= IDLE_MS;
    if (idle && !asleep) {
        asleep = true;
        oled_off();
    } else if (!idle && asleep) {
        asleep = false;
        oled_on();
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
    last_act = tick = timer_read32();
}

#ifdef OLED_ENABLE
oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return OLED_ROTATION_0;
}

bool oled_task_user(void) {
    if (asleep) {
        return false;
    }
    oled_clear();

    const char *label = on_break ? "BREAK" : "FOCUS";
    if (phase == DONE) {
        label = "DONE";
    } else if (phase == SETTING) {
        label = "READY";
    }
    oled_set_cursor(0, 0);
    oled_write(label, false);

    char clk[6];
    clk[0] = '0' + remaining / 600;
    clk[1] = '0' + remaining / 60 % 10;
    clk[2] = ':';
    clk[3] = '0' + remaining % 60 / 10;
    clk[4] = '0' + remaining % 60 % 10;
    clk[5] = '\0';
    oled_set_cursor(11, 0);
    oled_write(clk, false);

    uint8_t fill = total ? (uint32_t)(total - remaining) * 123 / total : 0;
    for (uint8_t x = 0; x < 126; x++) {
        oled_write_pixel(x, 18, true);
        oled_write_pixel(x, 30, true);
    }
    for (uint8_t y = 18; y <= 30; y++) {
        oled_write_pixel(0, y, true);
        oled_write_pixel(125, y, true);
    }
    for (uint8_t x = 1; x <= fill; x++) {
        for (uint8_t y = 20; y <= 28; y++) {
            oled_write_pixel(x, y, true);
        }
    }
    return false;
}
#endif
