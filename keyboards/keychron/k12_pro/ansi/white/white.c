/* Copyright 2023 @ Keychron (https://www.keychron.com)
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

// clang-format off
#ifdef LED_MATRIX_ENABLE
const ckled2001_led g_ckled2001_leds[LED_MATRIX_LED_COUNT] = {
/* Refer to CKLED2001 manual for these locations
 *   driver
 *   |  LED address
 *   |  | */
    {0, B_16},
    {0, B_15},
    {0, B_14},
    {0, B_13},
    {0, B_12},
    {0, B_11},
    {0, B_10},
    {0, B_9},
    {0, B_8},
    {0, B_7},
    {0, B_6},
    {0, B_5},
    {0, B_4},
    {0, B_3},

    {0, C_16},
    {0, C_15},
    {0, C_14},
    {0, C_13},
    {0, C_12},
    {0, C_11},
    {0, C_10},
    {0, C_9},
    {0, C_8},
    {0, C_7},
    {0, C_6},
    {0, C_5},
    {0, C_4},
    {0, C_3},

    {0, D_16},
    {0, D_15},
    {0, D_14},
    {0, D_13},
    {0, D_12},
    {0, D_11},
    {0, D_10},
    {0, D_9},
    {0, D_8},
    {0, D_7},
    {0, D_6},
    {0, D_5},
    {0, D_3},

    {0, E_16},
    {0, E_14},
    {0, E_13},
    {0, E_12},
    {0, E_11},
    {0, E_10},
    {0, E_9},
    {0, E_8},
    {0, E_7},
    {0, E_6},
    {0, E_5},
    {0, E_3},

    {0, F_16},
    {0, F_15},
    {0, F_14},
    {0, F_10},
    {0, F_6},
    {0, F_5},
    {0, F_4},
    {0, F_3},
};
#endif
