#include QMK_KEYBOARD_H

enum totem_keys {
    TM_TOGGLE = SAFE_RANGE,  // start / pause
    TM_RESET,                // back to the set time
    TM_BREAK                 // short break preset
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT(
        TM_TOGGLE, TM_RESET, TM_BREAK, TM_TOGGLE
    )
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    return true;
}

bool encoder_update_user(uint8_t index, bool clockwise) {
    return true;
}

#ifdef OLED_ENABLE
bool oled_task_user(void) {
    oled_write_P(PSTR("focus totem\n"), false);
    return false;
}
#endif
