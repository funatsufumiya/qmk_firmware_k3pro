/* Copyright 2021 @ Keychron (https://www.keychron.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "quantum.h"
#include "raw_hid.h"
#ifdef KC_BLUETOOTH_ENABLE
#include "transport.h"
#include "ckbt51.h"
#endif

#ifndef BL_TEST_KEY1
#    define BL_TEST_KEY1 KC_RIGHT
#endif

#ifndef BL_TEST_KEY2
#    define BL_TEST_KEY2 KC_HOME
#endif

extern bool bt_factory_reset;

enum {
    BACKLIGHT_TEST_OFF = 0,
    BACKLIGHT_TEST_WHITE,
    BACKLIGHT_TEST_RED,
    BACKLIGHT_TEST_GREEN,
    BACKLIGHT_TEST_BLUE,
    BACKLIGHT_TEST_MAX,
};

enum {
    KEY_PRESS_FN             = 0x01 << 0,
    KEY_PRESS_J              = 0x01 << 1,
    KEY_PRESS_Z              = 0x01 << 2,
    KEY_PRESS_BL_KEY1        = 0x01 << 3,
    KEY_PRESS_BL_KEY2        = 0x01 << 4,
    KEY_PRESS_FACTORY_RESET  = KEY_PRESS_FN | KEY_PRESS_J | KEY_PRESS_Z,
    KEY_PRESS_BACKLIGTH_TEST = KEY_PRESS_FN | KEY_PRESS_BL_KEY1 | KEY_PRESS_BL_KEY2,
};

enum {
    FACTORY_TEST_CMD_BACKLIGHT = 0x01,
    FACTORY_TEST_CMD_OS_SWITCH,
    FACTORY_TEST_CMD_JUMP_TO_BL,
    FACTORY_TEST_CMD_INT_PIN,
    FACTORY_TEST_CMD_GET_TRANSPORT,
    FACTORY_TEST_CMD_CHARGING_ADC,
    FACTORY_TEST_CMD_RADIO_CARRIER,
};

enum {
    OS_SWITCH = 0x01,
};

static uint32_t factory_reset_timer = 0;
static uint8_t  factory_reset_state = 0;
static uint8_t  backlight_test_mode = BACKLIGHT_TEST_OFF;

static uint32_t factory_reset_ind_timer = 0;
static uint8_t  factory_reset_ind_state = 0;
static bool     report_os_sw_state      = false;

void factory_timer_start(void) {
    factory_reset_timer = timer_read32() == 0 ? 1 : timer_read32();
}

static inline void factory_timer_check(void) {
    if (sync_timer_elapsed32(factory_reset_timer) > 3000) {
        factory_reset_timer = 0;

        if (factory_reset_state == KEY_PRESS_FACTORY_RESET) {
            factory_reset_ind_timer = timer_read32() == 0 ? 1 : timer_read32();
            factory_reset_ind_state++;

            layer_state_t default_layer_tmp = default_layer_state;
            eeconfig_init();
            default_layer_set(default_layer_tmp);
#ifdef LED_MATRIX_ENABLE
            if (!led_matrix_is_enabled()) led_matrix_enable();
            led_matrix_init();
#endif
#ifdef RGB_MATRIX_ENABLE
            if (!rgb_matrix_is_enabled()) rgb_matrix_enable();
            rgb_matrix_init();
#endif
#ifdef KC_BLUETOOTH_ENABLE
            ckbt51_factory_reset();
            bt_factory_reset = true;
#endif
        } else if (factory_reset_state == KEY_PRESS_BACKLIGTH_TEST) {
#ifdef LED_MATRIX_ENABLE
            if (!led_matrix_is_enabled()) led_matrix_enable();
#endif
#ifdef RGB_MATRIX_ENABLE
            if (!rgb_matrix_is_enabled()) rgb_matrix_enable();
#endif
            backlight_test_mode = BACKLIGHT_TEST_WHITE;
        }

        factory_reset_state = 0;
    }
}

static inline void factory_reset_ind_timer_check(void) {
    if (factory_reset_ind_timer && timer_elapsed32(factory_reset_ind_timer) > 250) {
        if (factory_reset_ind_state++ > 6) {
            factory_reset_ind_timer = factory_reset_ind_state = 0;
        } else {
            factory_reset_ind_timer = timer_read32() == 0 ? 1 : timer_read32();
        }
    }
}

void process_record_factory_reset(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
#if defined(FN_KEY1) || defined(FN_KEY2)
#    ifdef FN_KEY1
        case FN_KEY1: /* fall through */
#    endif
#    ifdef FN_KEY2
        case FN_KEY2:
#    endif
            if (record->event.pressed) {
                factory_reset_state |= KEY_PRESS_FN;
            } else {
                factory_reset_state &= ~KEY_PRESS_FN;
                factory_reset_timer = 0;
            }
            break;
#endif
        case KC_J:
            if (record->event.pressed) {
                factory_reset_state |= KEY_PRESS_J;
                if (factory_reset_state == 0x07) factory_timer_start();
            } else {
                factory_reset_state &= ~KEY_PRESS_J;
                factory_reset_timer = 0;
            }
            break;
        case KC_Z:
            if (record->event.pressed) {
                factory_reset_state |= KEY_PRESS_Z;
                if (factory_reset_state == 0x07) factory_timer_start();
            } else {
                factory_reset_state &= ~KEY_PRESS_Z;
                factory_reset_timer = 0;
            }
            break;
#ifdef BL_TEST_KEY1
        case BL_TEST_KEY1:
            if (record->event.pressed) {
                if (backlight_test_mode) {
                    if (++backlight_test_mode >= BACKLIGHT_TEST_MAX) {
                        backlight_test_mode = BACKLIGHT_TEST_WHITE;
                    }
                } else {
                    factory_reset_state |= KEY_PRESS_BL_KEY1;
                    if (factory_reset_state == 0x19) factory_timer_start();
                }
            } else {
                factory_reset_state &= ~KEY_PRESS_BL_KEY1;
                factory_reset_timer = 0;
            }
            break;
#endif
#ifdef BL_TEST_KEY2
        case BL_TEST_KEY2:
            if (record->event.pressed) {
                if (backlight_test_mode) {
                    backlight_test_mode = BACKLIGHT_TEST_OFF;
                } else {
                    factory_reset_state |= KEY_PRESS_BL_KEY2;
                    if (factory_reset_state == 0x19) factory_timer_start();
                }
            } else {
                factory_reset_state &= ~KEY_PRESS_BL_KEY2;
                factory_reset_timer = 0;
            }
            break;
#endif
    }
}

#ifdef LED_MATRIX_ENABLE
bool led_matrix_indicators_user(void) {
    if (factory_reset_ind_state) {
        led_matrix_set_value_all(factory_reset_ind_state % 2 ? 0 : 255);
    }

    return true;
}
#endif

#ifdef RGB_MATRIX_ENABLE
bool rgb_matrix_indicators_user(void) {
    if (factory_reset_ind_state) {
        backlight_test_mode = BACKLIGHT_TEST_OFF;
        rgb_matrix_set_color_all(factory_reset_ind_state % 2 ? 0 : 255, 0, 0);
    } else if (backlight_test_mode) {
        switch (backlight_test_mode) {
            case BACKLIGHT_TEST_WHITE:
                rgb_matrix_set_color_all(255, 255, 255);
                break;
            case BACKLIGHT_TEST_RED:
                rgb_matrix_set_color_all(255, 0, 0);
                break;
            case BACKLIGHT_TEST_GREEN:
                rgb_matrix_set_color_all(0, 255, 0);
                break;
            case BACKLIGHT_TEST_BLUE:
                rgb_matrix_set_color_all(0, 0, 255);
                break;
        }
    }

    return true;
}
#endif

void factory_reset_task(void) {
    if (factory_reset_timer) factory_timer_check();
    if (factory_reset_ind_timer) factory_reset_ind_timer_check();
}

void factory_test_send(uint8_t *payload, uint8_t length) {
    uint16_t checksum         = 0;
    uint8_t  data[RAW_EPSIZE] = {0};

    uint8_t i = 0;
    data[i++] = 0xAB;

    memcpy(&data[i], payload, length);
    i += length;

    for (uint8_t i = 1; i < RAW_EPSIZE - 3; i++) checksum += data[i];
    data[RAW_EPSIZE - 2] = checksum & 0xFF;
    data[RAW_EPSIZE - 1] = (checksum >> 8) & 0xFF;

    raw_hid_send(data, RAW_EPSIZE);
}

void factory_test_rx(uint8_t *data, uint8_t length) {
    if (data[0] == 0xAB) {
        uint16_t checksum = 0;

        for (uint8_t i = 1; i < RAW_EPSIZE - 3; i++) {
            checksum += data[i];
        }
        /* Verify checksum */
        if ((checksum & 0xFF) != data[RAW_EPSIZE - 2] || checksum >> 8 != data[RAW_EPSIZE - 1]) return;

#ifdef KC_BLUETOOTH_ENABLE
        uint8_t payload[32];
        uint8_t len = 0;
#endif

        switch (data[1]) {
            case FACTORY_TEST_CMD_BACKLIGHT:
                backlight_test_mode = data[2];
                factory_reset_timer = 0;
                break;
            case FACTORY_TEST_CMD_OS_SWITCH:
                report_os_sw_state = data[2];
                if (report_os_sw_state) {
                    dip_switch_read(true);
                }
                break;
            case FACTORY_TEST_CMD_JUMP_TO_BL:
                // if (memcmp(&data[2], "JumpToBootloader", strlen("JumpToBootloader")) == 0) bootloader_jump();
                break;
#ifdef KC_BLUETOOTH_ENABLE
            case FACTORY_TEST_CMD_INT_PIN:
                switch (data[2]) {
                    /* Enalbe/disable test */
                    case 0xA1:
                        ckbt51_int_pin_test(data[3]);
                        break;
                    /* Set INT state */
                    case 0xA2:
                        writePin(CKBT51_INT_INPUT_PIN, data[3]);
                        break;
                    /* Report INT state */
                    case 0xA3:
                        payload[len++] = FACTORY_TEST_CMD_INT_PIN;
                        payload[len++] = 0xA3;
                        payload[len++] = readPin(BLUETOOTH_INT_INPUT_PIN);
                        factory_test_send(payload, len);
                        break;
                }
                break;
            case FACTORY_TEST_CMD_GET_TRANSPORT:
                payload[len++] = FACTORY_TEST_CMD_GET_TRANSPORT;
                payload[len++] = get_transport();
                payload[len++] = readPin(USB_POWER_SENSE_PIN);
                factory_test_send(payload, len);
                break;
#endif
#ifdef BATTERY_CHARGE_DONE_DETECT_ADC
            case FACTORY_TEST_CMD_CHARGING_ADC:
            case 0xA1:
                battery_charging_monitor(data[3]);
                break;
            case 0xA2:
                payload[len++] = FACTORY_TEST_CMD_CHARGING_ADC;
                payload[len++] = battery_adc_read_charging_pin();
                factory_test_send(payload, len);
                break;
#endif
            case FACTORY_TEST_CMD_RADIO_CARRIER:
                if (data[2] < 79) ckbt51_radio_test(data[2]);
                break;
        }
    }
}

bool dip_switch_update_user(uint8_t index, bool active) {
    if (report_os_sw_state) {
#ifdef INVERT_OS_SWITCH_STATE
        active = !active;
#endif
        uint8_t payload[3] = {FACTORY_TEST_CMD_OS_SWITCH, OS_SWITCH, active};
        factory_test_send(payload, 3);
    }

    return true;
}
