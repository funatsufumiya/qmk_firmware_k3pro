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

// clang-format off
#ifdef LED_MATRIX_ENABLE
const ckled2001_led g_ckled2001_leds[LED_MATRIX_LED_COUNT] = {
/* Refer to CKLED2001 manual for these locations
 *   driver
 *   |  LED address
 *   |  | */
    {0, A_16}, 
    {0, A_15}, 
    {0, A_14}, 
    {0, A_13}, 
    {0, A_12}, 
    {0, A_11}, 
    {0, A_10}, 
    {0, A_9}, 
    {0, A_8}, 
    {0, A_7}, 
    {0, A_6}, 
    {0, A_5}, 
    {0, A_4},
    {0, A_2},  
    {0, A_1},  
    {0, D_2},

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
    {0, B_2}, 
    {0, B_1}, 
    {0, E_2},

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
    {0, C_2}, 
    {0, C_1}, 
    {0, D_1},

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
    {0, E_15},
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
    {0, E_1},

    {0, F_16},
    {0, F_15},
    {0, F_14},

    {0, F_10},
    {0, F_6},
    {0, F_5},
    {0, F_4},
    {0, F_3},
    {0, F_2},
    {0, F_1}, 
    {0, D_4},
};

led_config_t g_led_config = {
	{
		{  0,      1,      2,      3,      4,      5,      6,      7,      8,      9,      10,     11,     12,     NO_LED, 13,     14,     15 },
		{  16,     17,     18,     19,     20,     21,     22,     23,     24,     25,     26,     27,     28,     29,     30,     31,     32 },
		{  33,     34,     35,     36,     37,     38,     39,     40,     41,     42,     43,     44,     45,     46,     47,     48,     49 },
		{  50,     51,     52,     53,     54,     55,     56,     57,     58,     59,     60,     61,     NO_LED, 62,     NO_LED, NO_LED, NO_LED },
		{  63,     64,     65,     66,     67,     68,     69,     70,     71,     72,     73,     74,     NO_LED, 75,     NO_LED, 76,     NO_LED },
        {  77,     78,     79,     NO_LED, NO_LED, NO_LED, 80,     NO_LED, NO_LED, NO_LED, 81,     82,     83,     84,     85,     86,     87}
	},
	{
        {0, 0},          {25, 0}, {38, 0}, {51, 0}, {64, 0}, {84, 0}, { 97, 0}, {110, 0}, {123, 0}, {142, 0}, {155, 0}, {168, 0}, {181, 0}, {198, 0}, {211, 0}, {224, 0},
        {0,14}, {12,14}, {25,14}, {38,14}, {51,14}, {64,14}, {77,14}, { 90,14}, {103,14}, {116,14}, {129,14}, {142,14}, {155,14}, {175,14}, {198,14}, {211,14}, {224,14},
        {3,26}, {19,26}, {32,26}, {45,26}, {58,26}, {71,26}, {84,26}, { 97,26}, {110,26}, {123,26}, {136,26}, {149,26}, {162,26}, {178,26}, {198,26}, {211,26}, {224,26},
        {4,39}, {22,39}, {35,39}, {48,39}, {61,39}, {74,39}, {87,39}, {100,39}, {113,39}, {126,39}, {139,39}, {152,39},           {173,39},
        {0,51}, {16,51}, {29,51}, {42,51}, {55,51}, {68,51}, {81,51}, {94,51},  {107,51}, {120,51}, {132,51}, {145,51},           {170,51},           {211,51},
        {1,64}, {17,64}, {34,64},                            {82,64},                               {131,64}, {147,64}, {163,64}, {180,64}, {198,64}, {211,64}, {224,64}
    },
	{
        4,    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,          4,    4,    4,
        4,    8,    8,    8,    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,
        4,    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,
        4,    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,          4,         
        4,    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,    4,          4,          4,
        4,    4,    4,                      4,                      4,    4,    4,    4,    4,    4,    4
	}
};

#endif