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

#include QMK_KEYBOARD_H

// 薙刀式
#include "naginata.h"

// content of naginata.c

/* Copyright 2018-2019 eswai <@eswai>
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

#if !defined(__AVR__)
  #include <string.h>
  #ifndef memcpy_P
  #define memcpy_P(des, src, len) memcpy(des, src, len)
  #endif
#endif

#define NGBUFFER 5 // バッファのサイズ

static uint8_t ng_chrcount = 0; // 文字キー入力のカウンタ (シフトキーを除く)
static bool is_naginata = false; // 薙刀式がオンかオフか
static uint8_t naginata_layer = 0; // レイヤー番号
static uint8_t n_modifier = 0; // 押しているmodifierキーの数
static uint64_t keycomb = (uint64_t)0; // 新下駄 // 同時押しの状態を示す。64bitの各ビットがキーに対応する。

static bool is_shingeta = true;

// 50キーを64bitの各ビットに割り当てる // 新下駄
#define KB_Q    ((uint64_t)1<<0)
#define KB_W    ((uint64_t)1<<1)
#define KB_E    ((uint64_t)1<<2)
#define KB_R    ((uint64_t)1<<3)
#define KB_T    ((uint64_t)1<<4)

#define KB_Y    ((uint64_t)1<<5)
#define KB_U    ((uint64_t)1<<6)
#define KB_I    ((uint64_t)1<<7)
#define KB_O    ((uint64_t)1<<8)
#define KB_P    ((uint64_t)1<<9)

#define KB_A    ((uint64_t)1<<10)
#define KB_S    ((uint64_t)1<<11)
#define KB_D    ((uint64_t)1<<12)
#define KB_F    ((uint64_t)1<<13)
#define KB_G    ((uint64_t)1<<14)

#define KB_H    ((uint64_t)1<<15)
#define KB_J    ((uint64_t)1<<16)
#define KB_K    ((uint64_t)1<<17)
#define KB_L    ((uint64_t)1<<18)
#define KB_SCLN ((uint64_t)1<<19)

#define KB_Z    ((uint64_t)1<<20)
#define KB_X    ((uint64_t)1<<21)
#define KB_C    ((uint64_t)1<<22)
#define KB_V    ((uint64_t)1<<23)
#define KB_B    ((uint64_t)1<<24)

#define KB_N    ((uint64_t)1<<25)
#define KB_M    ((uint64_t)1<<26)
#define KB_COMM ((uint64_t)1<<27)
#define KB_DOT  ((uint64_t)1<<28)
#define KB_SLSH ((uint64_t)1<<29)

#define KB_1    ((uint64_t)1<<30) // 新下駄
#define KB_2    ((uint64_t)1<<31) // 新下駄
#define KB_3    ((uint64_t)1<<32) // 新下駄
#define KB_4    ((uint64_t)1<<33) // 新下駄
#define KB_5    ((uint64_t)1<<34) // 新下駄
#define KB_6    ((uint64_t)1<<35) // 新下駄
#define KB_7    ((uint64_t)1<<36) // 新下駄
#define KB_8    ((uint64_t)1<<37) // 新下駄
#define KB_9    ((uint64_t)1<<38) // 新下駄
#define KB_0    ((uint64_t)1<<39) // 新下駄
#define KB_MINS ((uint64_t)1<<40) // 新下駄

#define KB_X1   ((uint64_t)1<<41) // 新下駄

#define KB_SHFT ((uint64_t)1<<42)

#define KB_HAT ((uint64_t)1<<43) // JIS下駄
#define KB_YEN ((uint64_t)1<<44) // JIS下駄
#define KB_LB ((uint64_t)1<<45) // JIS下駄
#define KB_RB ((uint64_t)1<<46) // JIS下駄
#define KB_COLO ((uint64_t)1<<47) // JIS下駄
#define KB_BSLSH ((uint64_t)1<<48) // JIS下駄
#define KB_LSFT ((uint64_t)1<<49) // JIS下駄

// 文字入力バッファ
static uint16_t ninputs[NGBUFFER];

// キーコードとキービットの対応
// メモリ削減のため配列はNG_Qを0にしている
const uint64_t ng_key[] = { // 新下駄
  [NG_Q    - NG_Q] = KB_Q,
  [NG_W    - NG_Q] = KB_W,
  [NG_E    - NG_Q] = KB_E,
  [NG_R    - NG_Q] = KB_R,
  [NG_T    - NG_Q] = KB_T,

  [NG_Y    - NG_Q] = KB_Y,
  [NG_U    - NG_Q] = KB_U,
  [NG_I    - NG_Q] = KB_I,
  [NG_O    - NG_Q] = KB_O,
  [NG_P    - NG_Q] = KB_P,

  [NG_A    - NG_Q] = KB_A,
  [NG_S    - NG_Q] = KB_S,
  [NG_D    - NG_Q] = KB_D,
  [NG_F    - NG_Q] = KB_F,
  [NG_G    - NG_Q] = KB_G,

  [NG_H    - NG_Q] = KB_H,
  [NG_J    - NG_Q] = KB_J,
  [NG_K    - NG_Q] = KB_K,
  [NG_L    - NG_Q] = KB_L,
  [NG_SCLN - NG_Q] = KB_SCLN,

  [NG_Z    - NG_Q] = KB_Z,
  [NG_X    - NG_Q] = KB_X,
  [NG_C    - NG_Q] = KB_C,
  [NG_V    - NG_Q] = KB_V,
  [NG_B    - NG_Q] = KB_B,

  [NG_N    - NG_Q] = KB_N,
  [NG_M    - NG_Q] = KB_M,
  [NG_COMM - NG_Q] = KB_COMM,
  [NG_DOT  - NG_Q] = KB_DOT,
  [NG_SLSH - NG_Q] = KB_SLSH,

  [NG_1    - NG_Q] = KB_1, // 新下駄
  [NG_2    - NG_Q] = KB_2, // 新下駄
  [NG_3    - NG_Q] = KB_3, // 新下駄
  [NG_4    - NG_Q] = KB_4, // 新下駄
  [NG_5    - NG_Q] = KB_5, // 新下駄
  [NG_6    - NG_Q] = KB_6, // 新下駄
  [NG_7    - NG_Q] = KB_7, // 新下駄
  [NG_8    - NG_Q] = KB_8, // 新下駄
  [NG_9    - NG_Q] = KB_9, // 新下駄
  [NG_0    - NG_Q] = KB_0, // 新下駄
  [NG_MINS - NG_Q] = KB_MINS, // 新下駄

  [NG_X1   - NG_Q] = KB_X1, // 新下駄

  [NG_SHFT - NG_Q] = KB_SHFT, // 新下駄

  [NG_HAT  - NG_Q] = KB_HAT, // JIS下駄
  [NG_YEN  - NG_Q] = KB_YEN, // JIS下駄
  [NG_LB   - NG_Q] = KB_LB, // JIS下駄
  [NG_RB   - NG_Q] = KB_RB, // JIS下駄
  [NG_COLO - NG_Q] = KB_COLO, // JIS下駄
  [NG_BSLSH- NG_Q] = KB_BSLSH, // JIS下駄
  [NG_LSFT - NG_Q] = KB_LSFT, // JIS下駄
};

// 薙刀式カナ変換テーブル // 新下駄
// 順序つき
// #ifdef NAGINATA_JDOUJI
// typedef struct {
//   uint32_t key[3];
//   char kana[5];
// } naginata_keymap_ordered;
// #endif

// 順序なし
typedef struct {
  uint64_t key; // 新下駄
  char kana[5];
} naginata_keymap;

// 順序なしロング // 新下駄
// typedef struct {
//   uint32_t key;
//   char kana[15];
// } naginata_keymap_long;

// 順序なしUNICODE // 新下駄
// typedef struct {
//   uint32_t key;
//   char kana[10];
// } naginata_keymap_unicode;

// #ifdef NAGINATA_JDOUJI // 新下駄
// const PROGMEM naginata_keymap_ordered ngmapo[] = {
//   {.key = {NG_K, NG_E, 0}   , .kana = "ite"},
//   {.key = {NG_L, NG_D, 0}   , .kana = "uto"},
// };
// #endif

const PROGMEM naginata_keymap ngmap[] = { // 新下駄
  // 単独
  {.key = KB_1               , .kana = "nu"},
  {.key = KB_2               , .kana = "fu"},
  {.key = KB_3               , .kana = "a"},
  {.key = KB_4               , .kana = "ma"},
  {.key = KB_5               , .kana = "e"},
  {.key = KB_6               , .kana = "o"},
  {.key = KB_7               , .kana = "ya"},
  {.key = KB_8               , .kana = "yu"},
  {.key = KB_9               , .kana = "yo"},
  {.key = KB_0               , .kana = "wa"},
  {.key = KB_MINS            , .kana = "ho"},
  {.key = KB_HAT             , .kana = "he"},
  {.key = KB_YEN             , .kana = "wo"},
  {.key = KB_SHFT            , .kana = " "},

  {.key = KB_Q               , .kana = "ta"},
  {.key = KB_W               , .kana = "te"},
  {.key = KB_E               , .kana = "i"},
  {.key = KB_R               , .kana = "su"},
  {.key = KB_T               , .kana = "ka"},
  {.key = KB_Y               , .kana = "nn"},
  {.key = KB_U               , .kana = "na"},
  {.key = KB_I               , .kana = "ni"},
  {.key = KB_O               , .kana = "ra"},
  {.key = KB_P               , .kana = "se"},
  {.key = KB_X1              , .kana = "tu"},
  {.key = KB_LB              , .kana = "-"},

  {.key = KB_A               , .kana = "ti"},
  {.key = KB_S               , .kana = "to"},
  {.key = KB_D               , .kana = "si"},
  {.key = KB_F               , .kana = "ha"},
  {.key = KB_G               , .kana = "xtu"},
  {.key = KB_H               , .kana = "ku"},
  {.key = KB_J               , .kana = "u"},
  {.key = KB_K               , .kana = "no"},
  {.key = KB_L               , .kana = "ri"},
  {.key = KB_SCLN            , .kana = "re"},
  {.key = KB_COLO            , .kana = "ke"},
  {.key = KB_RB              , .kana = "mu"},

  {.key = KB_Z               , .kana = "ki"},
  {.key = KB_X               , .kana = "sa"},
  {.key = KB_C               , .kana = "so"},
  {.key = KB_V               , .kana = "hi"},
  {.key = KB_B               , .kana = "ko"},
  {.key = KB_N               , .kana = "mi"},
  {.key = KB_M               , .kana = "mo"},
  {.key = KB_COMM            , .kana = "ne"},
  {.key = KB_DOT             , .kana = "ru"},
  {.key = KB_SLSH            , .kana = "me"},
  {.key = KB_BSLSH           , .kana = "ro"},

  // 通常シフト
  {.key = KB_LSFT|KB_1        , .kana = "!"},
  {.key = KB_LSFT|KB_3        , .kana = "xa"},
  {.key = KB_LSFT|KB_5        , .kana = "xe"},
  {.key = KB_LSFT|KB_6        , .kana = "xo"},
  {.key = KB_LSFT|KB_7        , .kana = "xya"},
  {.key = KB_LSFT|KB_8        , .kana = "xyu"},
  {.key = KB_LSFT|KB_9        , .kana = "xyo"},
  {.key = KB_LSFT|KB_0        , .kana = "wo"},
  {.key = KB_LSFT|KB_MINS     , .kana = "-"},
  {.key = KB_LSFT|KB_HAT      , .kana = "~"},

  {.key = KB_LSFT|KB_E        , .kana = "xi"},

  {.key = KB_LSFT|KB_J        , .kana = "xu"},

  {.key = KB_LSFT|KB_LB       , .kana = "("},
  {.key = KB_LSFT|KB_RB       , .kana = ")"},
  {.key = KB_LSFT|KB_SCLN     , .kana = "+"},
  {.key = KB_LSFT|KB_COLO     , .kana = "*"},

  {.key = KB_LSFT|KB_COMM     , .kana = ","},
  {.key = KB_LSFT|KB_DOT      , .kana = "."},
  {.key = KB_LSFT|KB_SLSH     , .kana = "?"},
  {.key = KB_LSFT|KB_BSLSH    , .kana = "_"},

  // 中指シフト
  {.key = KB_K|KB_1        , .kana = "!"},
  {.key = KB_K|KB_2        , .kana = "bu"},
  {.key = KB_K|KB_3        , .kana = "xa"},
  {.key = KB_K|KB_5        , .kana = "xe"},
  {.key = KB_K|KB_6        , .kana = "xo"},
  {.key = KB_D|KB_7        , .kana = "xya"},
  {.key = KB_D|KB_8        , .kana = "xyu"},
  {.key = KB_D|KB_9        , .kana = "xyo"},
  {.key = KB_D|KB_0        , .kana = "wo"},
  {.key = KB_D|KB_MINS     , .kana = "bo"},
  {.key = KB_D|KB_HAT      , .kana = "be"},

  {.key = KB_K|KB_Q        , .kana = "da"},
  {.key = KB_K|KB_W        , .kana = "de"},
  {.key = KB_K|KB_E        , .kana = "xi"},
  {.key = KB_K|KB_R        , .kana = "zu"},
  {.key = KB_K|KB_T        , .kana = "ga"},
  {.key = KB_D|KB_Y        , .kana = "sye"}, // 拗音拡張
  {.key = KB_D|KB_U        , .kana = "sya"}, // 拗音拡張
  {.key = KB_D|KB_I        , .kana = "syu"}, // 拗音拡張
  {.key = KB_D|KB_O        , .kana = "syo"}, // 拗音拡張
  {.key = KB_D|KB_P        , .kana = "ze"},
  {.key = KB_D|KB_X1       , .kana = "du"},
  {.key = KB_D|KB_LB       , .kana = "["},

  {.key = KB_K|KB_A        , .kana = "di"},
  {.key = KB_K|KB_S        , .kana = "do"},
  {.key = KB_K|KB_D        , .kana = "ji"},
  {.key = KB_K|KB_F        , .kana = "ba"},
  // {.key = KB_K|KB_G        , .kana = ""},
  {.key = KB_D|KB_H        , .kana = "gu"},
  {.key = KB_D|KB_J        , .kana = "vu"},
  {.key = KB_D|KB_K        , .kana = "ji"},
  // {.key = KB_D|KB_L        , .kana = ""},
  {.key = KB_D|KB_SCLN     , .kana = "mu"},
  {.key = KB_D|KB_COLO     , .kana = "ge"},
  {.key = KB_D|KB_RB       , .kana = "]"},

  {.key = KB_K|KB_Z        , .kana = "gi"},
  {.key = KB_K|KB_X        , .kana = "za"},
  {.key = KB_K|KB_C        , .kana = "zo"},
  {.key = KB_K|KB_V        , .kana = "bi"},
  {.key = KB_K|KB_B        , .kana = "go"},
  // {.key = KB_D|KB_N        , .kana = ""},
  // {.key = KB_D|KB_M        , .kana = ""},
  {.key = KB_D|KB_COMM     , .kana = ","},
  {.key = KB_D|KB_DOT      , .kana = "."},
  {.key = KB_D|KB_SLSH     , .kana = "ro"},

  // {.key = KB_K|KB_1        , .kana = "la"},
  // {.key = KB_K|KB_2        , .kana = "li"},
  // {.key = KB_K|KB_3        , .kana = "lu"},
  // {.key = KB_K|KB_4        , .kana = "le"},
  // {.key = KB_K|KB_5        , .kana = "lo"},

  // 薬指シフト
  {.key = KB_L|KB_1        , .kana = "?"},
  {.key = KB_L|KB_2        , .kana = "pu"},
  {.key = KB_S|KB_0        , .kana = "wa"},
  {.key = KB_S|KB_MINS     , .kana = "po"},
  {.key = KB_S|KB_HAT      , .kana = "pe"},

  // {.key = KB_L|KB_Q        , .kana = "di"},
  // {.key = KB_L|KB_W        , .kana = "me"},
  // {.key = KB_L|KB_E        , .kana = "ke"},
  // {.key = KB_L|KB_R        , .kana = "teli"},
  // {.key = KB_L|KB_T        , .kana = "deli"},
  {.key = KB_S|KB_Y        , .kana = "gye"}, // 拗音拡張
  {.key = KB_S|KB_U        , .kana = "gya"}, // 拗音拡張
  {.key = KB_S|KB_I        , .kana = "gyu"}, // 拗音拡張
  {.key = KB_S|KB_O        , .kana = "gyo"}, // 拗音拡張
  // {.key = KB_S|KB_P        , .kana = "je"},

  // {.key = KB_L|KB_A        , .kana = "wo"},
  // {.key = KB_L|KB_S        , .kana = "sa"},
  // {.key = KB_L|KB_D        , .kana = ""},
  {.key = KB_L|KB_F        , .kana = "pa"},
  // {.key = KB_L|KB_G        , .kana = ""},
  // {.key = KB_S|KB_H        , .kana = ""},
  // {.key = KB_S|KB_J        , .kana = ""},
  // {.key = KB_S|KB_K        , .kana = ""},
  // {.key = KB_S|KB_L        , .kana = ""},
  {.key = KB_S|KB_SCLN     , .kana = "+"},
  {.key = KB_S|KB_COLO     , .kana = "*"},
  {.key = KB_S|KB_LB     , .kana = "("},
  {.key = KB_S|KB_RB     , .kana = ")"},

  // {.key = KB_L|KB_Z        , .kana = "ze"},
  // {.key = KB_L|KB_X        , .kana = "za"},
  // {.key = KB_L|KB_C        , .kana = "gi"},
  {.key = KB_L|KB_V        , .kana = "pi"},
  // {.key = KB_L|KB_B        , .kana = "nu"},
  // {.key = KB_S|KB_N        , .kana = "wa"},
  // {.key = KB_S|KB_M        , .kana = "da"},
  {.key = KB_S|KB_COMM     , .kana = "<"},
  {.key = KB_S|KB_DOT      , .kana = ">"},
  {.key = KB_S|KB_SLSH     , .kana = "/"},

  // U/Rレイヤー: 最上段数字
  {.key = KB_U|KB_1        , .kana = "1"},
  {.key = KB_U|KB_2        , .kana = "2"},
  {.key = KB_U|KB_3        , .kana = "3"},
  {.key = KB_U|KB_4        , .kana = "4"},
  {.key = KB_U|KB_5        , .kana = "5"},
  {.key = KB_U|KB_6        , .kana = "6"},
  {.key = KB_U|KB_7        , .kana = "7"},
  {.key = KB_U|KB_8        , .kana = "8"},
  {.key = KB_U|KB_9        , .kana = "9"},
  {.key = KB_U|KB_0        , .kana = "0"},
  {.key = KB_U|KB_MINS     , .kana = "-"},
  {.key = KB_U|KB_HAT      , .kana = "^"},
  {.key = KB_U|KB_YEN      , .kana = "¥"},

  {.key = KB_R|KB_1        , .kana = "1"},
  {.key = KB_R|KB_2        , .kana = "2"},
  {.key = KB_R|KB_3        , .kana = "3"},
  {.key = KB_R|KB_4        , .kana = "4"},
  {.key = KB_R|KB_5        , .kana = "5"},
  {.key = KB_R|KB_6        , .kana = "6"},
  {.key = KB_R|KB_7        , .kana = "7"},
  {.key = KB_R|KB_8        , .kana = "8"},
  {.key = KB_R|KB_9        , .kana = "9"},
  {.key = KB_R|KB_0        , .kana = "0"},
  {.key = KB_R|KB_MINS     , .kana = "-"},
  {.key = KB_R|KB_HAT      , .kana = "^"},
  {.key = KB_R|KB_YEN      , .kana = "\\"},

  // W/Oレイヤー: 最上段記号
  {.key = KB_W|KB_1        , .kana = "!"},
  {.key = KB_W|KB_2        , .kana = "\""},
  {.key = KB_W|KB_3        , .kana = "#"},
  {.key = KB_W|KB_4        , .kana = "$"},
  {.key = KB_W|KB_5        , .kana = "%"},
  {.key = KB_W|KB_6        , .kana = "&"},
  {.key = KB_W|KB_7        , .kana = "'"},
  {.key = KB_W|KB_8        , .kana = "("},
  {.key = KB_W|KB_9        , .kana = ")"},
  // {.key = KB_W|KB_0        , .kana = ""},
  {.key = KB_W|KB_MINS     , .kana = "="},
  {.key = KB_W|KB_HAT      , .kana = "~"},
  {.key = KB_W|KB_YEN      , .kana = "|"},

  {.key = KB_O|KB_1        , .kana = "!"},
  {.key = KB_O|KB_2        , .kana = "\""},
  {.key = KB_O|KB_3        , .kana = "#"},
  {.key = KB_O|KB_4        , .kana = "$"},
  {.key = KB_O|KB_5        , .kana = "%"},
  {.key = KB_O|KB_6        , .kana = "&"},
  {.key = KB_O|KB_7        , .kana = "'"},
  {.key = KB_O|KB_8        , .kana = "("},
  {.key = KB_O|KB_9        , .kana = ")"},
  // {.key = KB_W|KB_0        , .kana = ""},
  {.key = KB_O|KB_MINS     , .kana = "="},
  {.key = KB_O|KB_HAT      , .kana = "~"},
  {.key = KB_O|KB_YEN      , .kana = "|"},

  // 拗音拡張
  {.key = KB_A|KB_Y        , .kana = "che"},
  {.key = KB_A|KB_U        , .kana = "cha"},
  {.key = KB_A|KB_I        , .kana = "chu"},
  {.key = KB_A|KB_O        , .kana = "cho"},

  {.key = KB_F|KB_Y        , .kana = "hye"},
  {.key = KB_F|KB_U        , .kana = "hya"},
  {.key = KB_F|KB_I        , .kana = "hyu"},
  {.key = KB_F|KB_O        , .kana = "hyo"},

  {.key = KB_G|KB_Y        , .kana = "kye"},
  {.key = KB_G|KB_U        , .kana = "kya"},
  {.key = KB_G|KB_I        , .kana = "kyu"},
  {.key = KB_G|KB_O        , .kana = "kyo"},

  {.key = KB_Z|KB_Y        , .kana = "kye"},
  {.key = KB_Z|KB_U        , .kana = "kya"},
  {.key = KB_Z|KB_I        , .kana = "kyu"},
  {.key = KB_Z|KB_O        , .kana = "kyo"},

  {.key = KB_X|KB_Y        , .kana = "pye"},
  {.key = KB_X|KB_U        , .kana = "pya"},
  {.key = KB_X|KB_I        , .kana = "pyu"},
  {.key = KB_X|KB_O        , .kana = "pyo"},

  {.key = KB_V|KB_Y        , .kana = "bye"},
  {.key = KB_V|KB_U        , .kana = "bya"},
  {.key = KB_V|KB_I        , .kana = "byu"},
  {.key = KB_V|KB_O        , .kana = "byo"},

  {.key = KB_C|KB_Y        , .kana = "je"},
  {.key = KB_C|KB_U        , .kana = "ja"},
  {.key = KB_C|KB_I        , .kana = "ju"},
  {.key = KB_C|KB_O        , .kana = "jo"},

  {.key = KB_I|KB_W        , .kana = "nya"},
  {.key = KB_I|KB_E        , .kana = "nyu"},
  {.key = KB_I|KB_R        , .kana = "nyo"},
  {.key = KB_I|KB_T        , .kana = "nye"},

  {.key = KB_L|KB_W        , .kana = "rya"},
  {.key = KB_L|KB_E        , .kana = "ryu"},
  {.key = KB_L|KB_R        , .kana = "ryo"},
  {.key = KB_L|KB_T        , .kana = "rye"},

  {.key = KB_N|KB_W        , .kana = "mya"},
  {.key = KB_N|KB_E        , .kana = "myu"},
  {.key = KB_N|KB_R        , .kana = "myo"},
  {.key = KB_N|KB_T        , .kana = "mye"},

  // 外来語
  {.key = KB_2|KB_5        , .kana = "fe"},
  {.key = KB_W|KB_5        , .kana = "texe"},
  {.key = KB_Q|KB_5        , .kana = "dexe"},
  {.key = KB_W|KB_E        , .kana = "texi"},
  {.key = KB_Q|KB_E        , .kana = "dexi"},
  {.key = KB_W|KB_J        , .kana = "texyu"},
  {.key = KB_Q|KB_J        , .kana = "dexyu"},
  {.key = KB_2|KB_3        , .kana = "fa"},
  {.key = KB_2|KB_E        , .kana = "fi"},
  {.key = KB_2|KB_6        , .kana = "fo"},
  {.key = KB_2|KB_J        , .kana = "fuxyu"},
  {.key = KB_J|KB_E        , .kana = "uxi"},
  {.key = KB_J|KB_6        , .kana = "uxo"},
  {.key = KB_J|KB_5        , .kana = "uxe"},

  // others
  // [RF]・[RG]・　[HU]／
  // [FG]「」　　　[HJ]（）
  // [FV]！[FB]！　[NJ]？

  {.key = KB_R|KB_F     , .kana = "/"},
  {.key = KB_R|KB_G     , .kana = "/"},
  {.key = KB_F|KB_G     , .kana = "()"SS_TAP(X_LEFT) }, // （）←
  {.key = KB_F|KB_V     , .kana = "!"},
  {.key = KB_F|KB_B     , .kana = "!"},
  {.key = KB_N|KB_J     , .kana = "?"},
  {.key = KB_H|KB_J     , .kana = "[]"SS_TAP(X_LEFT) }, // 「」←
  {.key = KB_H|KB_U     , .kana = "/"},
};

const PROGMEM naginata_keymap ngmap2[] = { // 薙刀式
  // 単独
  {.key = KB_1               , .kana = "1"},
  {.key = KB_2               , .kana = "2"},
  {.key = KB_3               , .kana = "3"},
  {.key = KB_4               , .kana = "4"},
  {.key = KB_5               , .kana = "5"},
  {.key = KB_6               , .kana = "6"},
  {.key = KB_7               , .kana = "7"},
  {.key = KB_8               , .kana = "8"},
  {.key = KB_9               , .kana = "9"},
  {.key = KB_0               , .kana = "0"},
  {.key = KB_MINS            , .kana = "-"},
  {.key = KB_Q               , .kana = "vu"},

  // 清音
  {.key = KB_J                      , .kana = "a"       }, // あ
  {.key = KB_K                      , .kana = "i"       }, // い
  {.key = KB_L                      , .kana = "u"       }, // う
  {.key = KB_SHFT|KB_O               , .kana = "e"       }, // え
  {.key = KB_SHFT|KB_N               , .kana = "o"       }, // お
  {.key = KB_F                      , .kana = "ka"      }, // か
  {.key = KB_W                      , .kana = "ki"      }, // き
  {.key = KB_H                      , .kana = "ku"      }, // く
  {.key = KB_S                      , .kana = "ke"      }, // け
  {.key = KB_V                      , .kana = "ko"      }, // こ
  {.key = KB_SHFT|KB_U               , .kana = "sa"      }, // さ
  {.key = KB_R                      , .kana = "si"      }, // し
  {.key = KB_O                      , .kana = "su"      }, // す
  {.key = KB_SHFT|KB_A               , .kana = "se"      }, // せ
  {.key = KB_B                      , .kana = "so"      }, // そ
  {.key = KB_N                      , .kana = "ta"      }, // た
  {.key = KB_SHFT|KB_G               , .kana = "ti"      }, // ち
  {.key = KB_SHFT|KB_SCLN            , .kana = "fu"      }, // ふ
  {.key = KB_E                      , .kana = "te"      }, // て
  {.key = KB_D                      , .kana = "to"      }, // と
  {.key = KB_M                      , .kana = "na"      }, // な
  {.key = KB_SHFT|KB_D               , .kana = "ni"      }, // に
  {.key = KB_SHFT|KB_R               , .kana = "ne"      }, // ね
  {.key = KB_SHFT|KB_COMM            , .kana = "mu"      }, // む
  {.key = KB_SHFT|KB_J               , .kana = "no"      }, // の
  {.key = KB_C                      , .kana = "ha"      }, // は
  {.key = KB_X                      , .kana = "hi"      }, // ひ
  {.key = KB_SHFT|KB_X               , .kana = "hi"      }, // ひ
  {.key = KB_SHFT|KB_DOT             , .kana = "wa"      }, // わ
  {.key = KB_P                      , .kana = "he"      }, // へ
  {.key = KB_Z                      , .kana = "ho"      }, // ほ
  {.key = KB_SHFT|KB_Z               , .kana = "ho"      }, // ほ
  {.key = KB_SHFT|KB_F               , .kana = "ma"      }, // ま
  {.key = KB_SHFT|KB_B               , .kana = "mi"      }, // み
  {.key = KB_SHFT|KB_W               , .kana = "nu"      }, // ぬ
  {.key = KB_SHFT|KB_S               , .kana = "me"      }, // め
  {.key = KB_SHFT|KB_K               , .kana = "mo"      }, // も
  {.key = KB_SHFT|KB_H               , .kana = "ya"      }, // や
  {.key = KB_SHFT|KB_P               , .kana = "yu"      }, // ゆ
  {.key = KB_SHFT|KB_I               , .kana = "yo"      }, // よ
  {.key = KB_DOT                    , .kana = "ra"      }, // ら
  {.key = KB_SHFT|KB_E               , .kana = "ri"      }, // り
  {.key = KB_I                      , .kana = "ru"      }, // る
  {.key = KB_SLSH                   , .kana = "re"      }, // れ
  {.key = KB_SHFT|KB_SLSH            , .kana = "re"      }, // れ
  {.key = KB_A                      , .kana = "ro"      }, // ろ
  {.key = KB_SHFT|KB_L               , .kana = "tsu"      }, // つ
  {.key = KB_SHFT|KB_C               , .kana = "wo"      }, // を
  {.key = KB_COMM                   , .kana = "nn"      }, // ん
  {.key = KB_SCLN                   , .kana = "-"       }, // ー

  // 濁音
  {.key = KB_J|KB_F                  , .kana = "ga"      }, // が
  {.key = KB_J|KB_W                  , .kana = "gi"      }, // ぎ
  {.key = KB_F|KB_H                  , .kana = "gu"      }, // ぐ
  {.key = KB_J|KB_S                  , .kana = "ge"      }, // げ
  {.key = KB_J|KB_V                  , .kana = "go"      }, // ご
  {.key = KB_F|KB_U                  , .kana = "za"      }, // ざ
  {.key = KB_J|KB_R                  , .kana = "zi"      }, // じ
  {.key = KB_F|KB_O                  , .kana = "zu"      }, // ず
  {.key = KB_J|KB_A                  , .kana = "ze"      }, // ぜ
  {.key = KB_J|KB_B                  , .kana = "zo"      }, // ぞ
  {.key = KB_F|KB_N                  , .kana = "da"      }, // だ
  {.key = KB_J|KB_G                  , .kana = "di"      }, // ぢ
  {.key = KB_F|KB_SCLN               , .kana = "bu"      }, // ぶ
  {.key = KB_J|KB_E                  , .kana = "de"      }, // で
  {.key = KB_J|KB_D                  , .kana = "do"      }, // ど
  {.key = KB_J|KB_C                  , .kana = "ba"      }, // ば
  {.key = KB_J|KB_X                  , .kana = "bi"      }, // び
  {.key = KB_F|KB_P                  , .kana = "be"      }, // べ
  {.key = KB_J|KB_Z                  , .kana = "bo"      }, // ぼ
  {.key = KB_F|KB_L                  , .kana = "du"      }, // づ

  // 半濁音
  {.key = KB_M|KB_C                  , .kana = "pa"      }, // ぱ
  {.key = KB_M|KB_X                  , .kana = "pi"      }, // ぴ
  {.key = KB_V|KB_SCLN                , .kana = "pu"      }, // ぷ
  {.key = KB_V|KB_P                  , .kana = "pe"      }, // ぺ
  {.key = KB_M|KB_Z                  , .kana = "po"      }, // ぽ

  // 小書き
  {.key = KB_Q|KB_H                  , .kana = "xya"     }, // ゃ
  {.key = KB_Q|KB_P                  , .kana = "xyu"     }, // ゅ
  {.key = KB_Q|KB_I                  , .kana = "xyo"     }, // ょ
  {.key = KB_Q|KB_J                  , .kana = "xa"      }, // ぁ
  {.key = KB_Q|KB_K                  , .kana = "xi"      }, // ぃ
  {.key = KB_Q|KB_L                  , .kana = "xu"      }, // ぅ
  {.key = KB_Q|KB_O                  , .kana = "xe"      }, // ぇ
  {.key = KB_Q|KB_N                  , .kana = "xo"      }, // ぉ
  {.key = KB_Q|KB_SHFT|KB_L           , .kana = "xwa"     }, // ゎ
  {.key = KB_G                      , .kana = "xtu"     }, // っ

  // 清音拗音 濁音拗音 半濁拗音
  {.key = KB_R|KB_H                  , .kana = "sya"     }, // しゃ
  {.key = KB_R|KB_P                  , .kana = "syu"     }, // しゅ
  {.key = KB_R|KB_I                  , .kana = "syo"     }, // しょ
  {.key = KB_J|KB_R|KB_H              , .kana = "zya"     }, // じゃ
  {.key = KB_J|KB_R|KB_P              , .kana = "zyu"     }, // じゅ
  {.key = KB_J|KB_R|KB_I              , .kana = "zyo"     }, // じょ
  {.key = KB_W|KB_H                  , .kana = "kya"     }, // きゃ
  {.key = KB_W|KB_P                  , .kana = "kyu"     }, // きゅ
  {.key = KB_W|KB_I                  , .kana = "kyo"     }, // きょ
  {.key = KB_J|KB_W|KB_H              , .kana = "gya"     }, // ぎゃ
  {.key = KB_J|KB_W|KB_P              , .kana = "gyu"     }, // ぎゅ
  {.key = KB_J|KB_W|KB_I              , .kana = "gyo"     }, // ぎょ
  {.key = KB_G|KB_H                  , .kana = "tya"     }, // ちゃ
  {.key = KB_G|KB_P                  , .kana = "tyu"     }, // ちゅ
  {.key = KB_G|KB_I                  , .kana = "tyo"     }, // ちょ
  {.key = KB_J|KB_G|KB_H              , .kana = "dya"     }, // ぢゃ
  {.key = KB_J|KB_G|KB_P              , .kana = "dyu"     }, // ぢゅ
  {.key = KB_J|KB_G|KB_I              , .kana = "dyo"     }, // ぢょ
  {.key = KB_D|KB_H                  , .kana = "nya"     }, // にゃ
  {.key = KB_D|KB_P                  , .kana = "nyu"     }, // にゅ
  {.key = KB_D|KB_I                  , .kana = "nyo"     }, // にょ
  {.key = KB_X|KB_H                  , .kana = "hya"     }, // ひゃ
  {.key = KB_X|KB_P                  , .kana = "hyu"     }, // ひゅ
  {.key = KB_X|KB_I                  , .kana = "hyo"     }, // ひょ
  {.key = KB_J|KB_X|KB_H              , .kana = "bya"     }, // びゃ
  {.key = KB_J|KB_X|KB_P              , .kana = "byu"     }, // びゅ
  {.key = KB_J|KB_X|KB_I              , .kana = "byo"     }, // びょ
  {.key = KB_M|KB_X|KB_H              , .kana = "pya"     }, // ぴゃ
  {.key = KB_M|KB_X|KB_P              , .kana = "pyu"     }, // ぴゅ
  {.key = KB_M|KB_X|KB_I              , .kana = "pyo"     }, // ぴょ
  {.key = KB_B|KB_H                  , .kana = "mya"     }, // みゃ
  {.key = KB_B|KB_P                  , .kana = "myu"     }, // みゅ
  {.key = KB_B|KB_I                  , .kana = "myo"     }, // みょ
  {.key = KB_E|KB_H                  , .kana = "rya"     }, // りゃ
  {.key = KB_E|KB_P                  , .kana = "ryu"     }, // りゅ
  {.key = KB_E|KB_I                  , .kana = "ryo"     }, // りょ

  // 清音外来音 濁音外来音
  {.key = KB_M|KB_E|KB_K              , .kana = "thi"     }, // てぃ
  {.key = KB_M|KB_E|KB_P              , .kana = "thu"     }, // てゅ
  {.key = KB_J|KB_E|KB_K              , .kana = "dhi"     }, // でぃ
  {.key = KB_J|KB_E|KB_P              , .kana = "dhu"     }, // でゅ
  {.key = KB_M|KB_D|KB_L              , .kana = "toxu"    }, // とぅ
  {.key = KB_J|KB_D|KB_L              , .kana = "doxu"    }, // どぅ
  {.key = KB_M|KB_R|KB_O              , .kana = "sye"     }, // しぇ
  {.key = KB_M|KB_G|KB_O              , .kana = "tye"     }, // ちぇ
  {.key = KB_J|KB_R|KB_O              , .kana = "zye"     }, // じぇ
  {.key = KB_J|KB_G|KB_O              , .kana = "dye"     }, // ぢぇ
  {.key = KB_V|KB_SCLN|KB_J            , .kana = "fa"      }, // ふぁ
  {.key = KB_V|KB_SCLN|KB_K            , .kana = "fi"      }, // ふぃ
  {.key = KB_V|KB_SCLN|KB_O            , .kana = "fe"      }, // ふぇ
  {.key = KB_V|KB_SCLN|KB_N            , .kana = "fo"      }, // ふぉ
  {.key = KB_V|KB_SCLN|KB_P            , .kana = "fyu"     }, // ふゅ
  {.key = KB_V|KB_K|KB_O              , .kana = "ixe"     }, // いぇ
  {.key = KB_V|KB_L|KB_K              , .kana = "wi"      }, // うぃ
  {.key = KB_V|KB_L|KB_O              , .kana = "we"      }, // うぇ
  {.key = KB_V|KB_L|KB_N              , .kana = "uxo"     }, // うぉ
  {.key = KB_V|KB_H|KB_J              , .kana = "kuxa"    }, // くぁ
  {.key = KB_V|KB_H|KB_K              , .kana = "kuxi"    }, // くぃ
  {.key = KB_V|KB_H|KB_O              , .kana = "kuxe"    }, // くぇ
  {.key = KB_V|KB_H|KB_N              , .kana = "kuxo"    }, // くぉ
  {.key = KB_V|KB_H|KB_L              , .kana = "kuxwa"   }, // くゎ
  {.key = KB_F|KB_H|KB_J              , .kana = "guxa"    }, // ぐぁ
  {.key = KB_F|KB_H|KB_K              , .kana = "guxi"    }, // ぐぃ
  {.key = KB_F|KB_H|KB_O              , .kana = "guxe"    }, // ぐぇ
  {.key = KB_F|KB_H|KB_N              , .kana = "guxo"    }, // ぐぉ
  {.key = KB_F|KB_H|KB_L              , .kana = "guxwa"   }, // ぐゎ
  {.key = KB_V|KB_L|KB_J           , .kana = "tsa"     }, // つぁ
  {.key = KB_V|KB_L|KB_K           , .kana = "tsi"     }, // つぃ
  {.key = KB_V|KB_L|KB_O           , .kana = "tse"     }, // つぇ
  {.key = KB_V|KB_L|KB_N           , .kana = "tso"     }, // つぉ

  // 追加
  {.key = KB_SHFT            , .kana = " "},
  {.key = KB_Q               , .kana = ""},
  {.key = KB_V|KB_SHFT        , .kana = ","},
  {.key = KB_M|KB_SHFT        , .kana = "."SS_TAP(X_ENTER)},
  {.key = KB_U               , .kana = SS_TAP(X_BSPC)},

  // enter
  {.key = KB_V|KB_M           , .kana = SS_TAP(X_ENTER)},
  // enter+シフト(連続シフト)
  {.key = KB_SHFT|KB_V|KB_M    , .kana = SS_TAP(X_ENTER)},
  // left
  {.key = KB_T               , .kana = SS_TAP(X_LEFT)},
  // right
  {.key = KB_Y               , .kana = SS_TAP(X_RIGHT)},

  {.key = KB_J|KB_K|KB_T       , .kana = "/"},  // ・
  {.key = KB_J|KB_K|KB_F       , .kana = "[]"}, // 「」
  {.key = KB_J|KB_K|KB_D       , .kana = "?"},  // ？
  {.key = KB_J|KB_K|KB_C       , .kana = "!"},  // ！
  {.key = KB_J|KB_K|KB_S       , .kana = "()"},  //（）
  {.key = KB_J|KB_K|KB_X       , .kana = "{}"},  //｛｝
  {.key = KB_J|KB_K|KB_A       , .kana = "///"},  // ・

  // 非標準の変換
  {.key = KB_V|KB_DOT|KB_COMM  , .kana = "fe"      }, // ふぇ
  {.key = KB_X|KB_C|KB_M       , .kana = "pyu"     }, // ピュ
};

 // 新下駄
// const PROGMEM naginata_keymap_long ngmapl[] = {
//   {.key = KB_SHFT|KB_T        , .kana = SS_DOWN(X_LSHIFT)SS_TAP(X_DOWN)SS_UP(X_LSHIFT)},
//   {.key = KB_SHFT|KB_Y        , .kana = SS_DOWN(X_LSHIFT)SS_TAP(X_UP)SS_UP(X_LSHIFT)},

//   // 編集モード1
//   {.key = KB_D|KB_F|KB_P       , .kana = SS_TAP(X_ESCAPE)SS_TAP(X_ESCAPE)SS_TAP(X_ESCAPE)},

//   {.key = KB_D|KB_F|KB_K       , .kana = SS_DOWN(X_LSHIFT)SS_TAP(X_LEFT)SS_UP(X_LSHIFT)},
//   {.key = KB_D|KB_F|KB_L       , .kana = SS_TAP(X_LEFT)SS_TAP(X_LEFT)SS_TAP(X_LEFT)SS_TAP(X_LEFT)SS_TAP(X_LEFT)},

//   {.key = KB_D|KB_F|KB_COMM    , .kana = SS_DOWN(X_LSHIFT)SS_TAP(X_RIGHT)SS_UP(X_LSHIFT)},
//   {.key = KB_D|KB_F|KB_DOT     , .kana = SS_TAP(X_RIGHT)SS_TAP(X_RIGHT)SS_TAP(X_RIGHT)SS_TAP(X_RIGHT)SS_TAP(X_RIGHT)},

// #ifdef NAGINATA_EDIT_WIN
//   {.key = KB_J|KB_K|KB_Q       , .kana = SS_DOWN(X_LCTRL)SS_TAP(X_END)SS_UP(X_LCTRL)},
//   {.key = KB_J|KB_K|KB_W       , .kana = SS_DOWN(X_LCTRL)SS_TAP(X_HOME)SS_UP(X_LCTRL)},
//   {.key = KB_D|KB_F|KB_U       , .kana = SS_DOWN(X_LSHIFT)SS_TAP(X_END)SS_UP(X_LSHIFT)SS_TAP(X_BSPACE)},
//   {.key = KB_D|KB_F|KB_H       , .kana = SS_TAP(X_ENTER)SS_TAP(X_END)},
//   {.key = KB_D|KB_F|KB_Y       , .kana = SS_TAP(X_HOME)},
//   {.key = KB_D|KB_F|KB_N       , .kana = SS_TAP(X_END)},
// #endif
// #ifdef NAGINATA_EDIT_MAC
//   {.key = KB_J|KB_K|KB_Q       , .kana = SS_DOWN(X_LGUI)SS_TAP(X_DOWN)SS_UP(X_LGUI)},
//   {.key = KB_J|KB_K|KB_W       , .kana = SS_DOWN(X_LGUI)SS_TAP(X_UP)SS_UP(X_LGUI)},
//   {.key = KB_D|KB_F|KB_U       , .kana = SS_DOWN(X_LSHIFT)SS_DOWN(X_LGUI)SS_TAP(X_RIGHT)SS_UP(X_LGUI)SS_UP(X_LSHIFT)SS_LGUI("x")},
//   {.key = KB_D|KB_F|KB_H       , .kana = SS_TAP(X_ENTER)SS_DOWN(X_LGUI)SS_TAP(X_RIGHT)SS_UP(X_LGUI)},
//   {.key = KB_D|KB_F|KB_Y       , .kana = SS_DOWN(X_LGUI)SS_TAP(X_LEFT)SS_UP(X_LGUI)},
//   {.key = KB_D|KB_F|KB_N       , .kana = SS_DOWN(X_LGUI)SS_TAP(X_RIGHT)SS_UP(X_LGUI)},
// #endif

//   // 編集モード2
// #ifdef NAGINATA_EDIT_WIN
//   {.key = KB_M|KB_COMM|KB_T    , .kana = SS_TAP(X_HOME)" "SS_TAP(X_END)},
//   {.key = KB_M|KB_COMM|KB_G    , .kana = SS_TAP(X_HOME)"   "SS_TAP(X_END)},
//   {.key = KB_C|KB_V|KB_U       , .kana = SS_DOWN(X_LSHIFT)SS_TAP(X_HOME)SS_UP(X_LSHIFT)SS_LCTRL("x")},
//   {.key = KB_C|KB_V|KB_I       , .kana = SS_DOWN(X_LCTRL)SS_TAP(X_BSPACE)SS_UP(X_LCTRL)},
// #endif
// #ifdef NAGINATA_EDIT_MAC
//   {.key = KB_M|KB_COMM|KB_T    , .kana = SS_DOWN(X_LGUI)SS_TAP(X_LEFT)SS_UP(X_LGUI)" "SS_DOWN(X_LGUI)SS_TAP(X_RIGHT)SS_UP(X_LGUI)},
//   {.key = KB_M|KB_COMM|KB_G    , .kana = SS_DOWN(X_LGUI)SS_TAP(X_LEFT)SS_UP(X_LGUI)"   "SS_DOWN(X_LGUI)SS_TAP(X_RIGHT)SS_UP(X_LGUI)},
//   {.key = KB_C|KB_V|KB_U       , .kana = SS_DOWN(X_LSHIFT)SS_DOWN(X_LGUI)SS_TAP(X_LEFT)SS_UP(X_LGUI)SS_UP(X_LSHIFT)SS_LGUI("x")},
//   {.key = KB_C|KB_V|KB_I       , .kana = ""},
// #endif

// };

 // 新下駄
// const PROGMEM naginata_keymap_unicode ngmapu[] = {
  // 編集モード2 // 新下駄
// #ifdef NAGINATA_EDIT_WIN
//   {.key = KB_SHFT|KB_W    , .kana = "309C"},
//   {.key = KB_L           , .kana = "309B"},
// #endif
// };

// 薙刀式のレイヤー、シフトキーを設定
void set_naginata(uint8_t layer) {
  naginata_layer = layer;
}

// 薙刀式をオンオフ
void naginata_on(void) {
  is_naginata = true;
  keycomb = (uint64_t)0; // 新下駄
  naginata_clear();
  layer_on(naginata_layer);

  tap_code(KC_LNG1); // Mac
  tap_code(KC_INT4); // Win
}

void naginata_off(void) {
  is_naginata = false;
  keycomb = (uint64_t)0; // 新下駄
  naginata_clear();
  layer_off(naginata_layer);

  tap_code(KC_LNG2); // Mac
  tap_code(KC_INT5); // Win
}

// 薙刀式の状態
bool naginata_state(void) {
  return is_naginata;
}


// キー入力を文字に変換して出力する
void naginata_type(void) {
 // 新下駄
// #ifdef NAGINATA_JDOUJI
//   naginata_keymap_ordered bngmapo; // PROGMEM buffer
// #endif
  naginata_keymap bngmap; // PROGMEM buffer
   // 新下駄
  // naginata_keymap_long bngmapl; // PROGMEM buffer
  // naginata_keymap_unicode bngmapu; // PROGMEM buffer

  uint32_t skey = 0; // 連続押しの場合のバッファ

  switch (keycomb) {
    // send_stringできないキー、長すぎるマクロはここで定義
 // 新下駄
    // case KB_F|KB_G:
    //   naginata_off();
    //   break;
    // case KB_J|KB_K|KB_T:
// #ifdef NAGINATA_EDIT_WIN
//       tap_code(KC_HOME);
//       for (int i = 0; i < 10; i++) tap_code(KC_RGHT);
// #endif
// #ifdef NAGINATA_EDIT_MAC
//       register_code(KC_LGUI);
//       tap_code(KC_LEFT);
//       unregister_code(KC_LGUI);
//       for (int i = 0; i < 10; i++) tap_code(KC_RGHT);
// #endif
//       break;
//     case KB_J|KB_K|KB_G:
// #ifdef NAGINATA_EDIT_WIN
//       tap_code(KC_HOME);
//       for (int i = 0; i < 20; i++) tap_code(KC_RGHT);
// #endif
// #ifdef NAGINATA_EDIT_MAC
//       register_code(KC_LGUI);
//       tap_code(KC_LEFT);
//       unregister_code(KC_LGUI);
//       for (int i = 0; i < 20; i++) tap_code(KC_RGHT);
// #endif
//       break;
//     case KB_J|KB_K|KB_B:
// #ifdef NAGINATA_EDIT_WIN
//       tap_code(KC_HOME);
//       for (int i = 0; i < 30; i++) tap_code(KC_RGHT);
// #endif
// #ifdef NAGINATA_EDIT_MAC
//       register_code(KC_LGUI);
//       tap_code(KC_LEFT);
//       unregister_code(KC_LGUI);
//       for (int i = 0; i < 30; i++) tap_code(KC_RGHT);
// #endif
//       break;
// #ifdef NAGINATA_EDIT_WIN
//     case KB_C|KB_V|KB_P:
//       send_unicode_hex_string("FF5C");
//       tap_code(KC_ENT);
//       tap_code(KC_END);
//       send_unicode_hex_string("300A 300B");
//       tap_code(KC_ENT);
//       tap_code(KC_LEFT);
//       break;
//     case KB_C|KB_V|KB_Y:
//       send_unicode_hex_string("300D");
//       tap_code(KC_ENT);
//       tap_code(KC_ENT);
//       tap_code(KC_SPC);
//       break;
//     case KB_C|KB_V|KB_H:
//       send_unicode_hex_string("300D");
//       tap_code(KC_ENT);
//       tap_code(KC_ENT);
//       send_unicode_hex_string("300C");
//       tap_code(KC_ENT);
//       break;
//     case KB_C|KB_V|KB_N:
//       send_unicode_hex_string("300D");
//       tap_code(KC_ENT);
//       tap_code(KC_ENT);
//       break;
// #endif
    default:
      // キーから仮名に変換して出力する。
      // 同時押しの場合 ngmapに定義されている
      // 順序つき
      // #ifdef NAGINATA_JDOUJI // 新下駄
      // for (int i = 0; i < sizeof ngmapo / sizeof bngmapo; i++) {
      //   memcpy_P(&bngmapo, &ngmapo[i], sizeof(bngmapo));
      //   if (ninputs[0] == bngmapo.key[0] && ninputs[1] == bngmapo.key[1] && ninputs[2] == bngmapo.key[2]) {
      //     send_string(bngmapo.kana);
      //     naginata_clear();
      //     return;
      //   }
      // }
      // #endif
      // 順序なし
      if(is_shingeta){
        for (int i = 0; i < sizeof ngmap / sizeof bngmap; i++) {
            memcpy_P(&bngmap, &ngmap[i], sizeof(bngmap));
            if (keycomb == bngmap.key) {
            send_string(bngmap.kana);
            naginata_clear();
            return;
            }
        }
      }else{
        for (int i = 0; i < sizeof ngmap2 / sizeof bngmap; i++) {
            memcpy_P(&bngmap, &ngmap2[i], sizeof(bngmap));
            if (keycomb == bngmap.key) {
            send_string(bngmap.kana);
            naginata_clear();
            return;
            }
        }
      }
      // // 順序なしロング // 新下駄
      // for (int i = 0; i < sizeof ngmapl / sizeof bngmapl; i++) {
      //   memcpy_P(&bngmapl, &ngmapl[i], sizeof(bngmapl));
      //   if (keycomb == bngmapl.key) {
      //     send_string(bngmapl.kana);
      //     naginata_clear();
      //     return;
      //   }
      // }
      // 順序なしUNICODE // 新下駄
      // for (int i = 0; i < sizeof ngmapu / sizeof bngmapu; i++) {
      //   memcpy_P(&bngmapu, &ngmapu[i], sizeof(bngmapu));
      //   if (keycomb == bngmapu.key) {
      //     send_unicode_hex_string(bngmapu.kana);
      //     // tap_code(KC_ENT);
      //     naginata_clear();
      //     return;
      //   }
      // }
      // 連続押しの場合 ngmapに定義されていない
      if(is_shingeta){
        for (int j = 0; j < ng_chrcount; j++) {
            skey = ng_key[ninputs[j] - NG_Q];
            if ((keycomb & KB_LSFT) > 0) skey |= KB_LSFT; // シフトキー状態を反映 // 新下駄
            for (int i = 0; i < sizeof ngmap / sizeof bngmap; i++) {
                memcpy_P(&bngmap, &ngmap[i], sizeof(bngmap));
                if (skey == bngmap.key) {
                    send_string(bngmap.kana);
                    break;
                }
            }
        }
      }else{
        for (int j = 0; j < ng_chrcount; j++) {
            skey = ng_key[ninputs[j] - NG_Q];
            if ((keycomb & KB_LSFT) > 0) skey |= KB_LSFT; // シフトキー状態を反映 // 新下駄
            for (int i = 0; i < sizeof ngmap2 / sizeof bngmap; i++) {
                memcpy_P(&bngmap, &ngmap2[i], sizeof(bngmap));
                if (skey == bngmap.key) {
                    send_string(bngmap.kana);
                    break;
                }
            }
        }
      }
  }

  naginata_clear(); // バッファを空にする
}

// バッファをクリアする
void naginata_clear(void) {
  for (int i = 0; i < NGBUFFER; i++) {
    ninputs[i] = 0;
  }
  ng_chrcount = 0;
}

// 入力モードか編集モードかを確認する
void naginata_mode(uint16_t keycode, keyrecord_t *record) {
  if (!is_naginata) return;

  // modifierが押されたらレイヤーをオフ
  switch (keycode) {
    case KC_LCTL:
    case KC_LSFT:
    case KC_LALT:
    case KC_LGUI:
    case KC_RCTL:
    case KC_RSFT:
    case KC_RALT:
    case KC_RGUI:
      if (record->event.pressed) {
        n_modifier++;
        layer_off(naginata_layer);
      } else {
        n_modifier--;
        if (n_modifier == 0) {
          layer_on(naginata_layer);
        }
      }
      break;
  }

}

// 薙刀式の入力処理
bool process_naginata(uint16_t keycode, keyrecord_t *record) {
  // if (!is_naginata || n_modifier > 0) return true;

  if (record->event.pressed) {
    switch (keycode) {
      case NG_Q ... NG_LSFT: // 新下駄
        ninputs[ng_chrcount] = keycode; // キー入力をバッファに貯める
        ng_chrcount++;
        keycomb |= ng_key[keycode - NG_Q]; // キーの重ね合わせ
        if(is_shingeta){
            // 2文字押したら処理を開始 // 新下駄
            if (ng_chrcount > 1) { // 新下駄
                naginata_type();
            }
        }
        return false;
        break;
    }
  } else { // key release
    switch (keycode) {
      case NG_Q ... NG_LSFT: // 新下駄
        // 3文字入力していなくても、どれかキーを離したら処理を開始する
        if (ng_chrcount > 0) {
          naginata_type();
        }
        keycomb &= ~ng_key[keycode - NG_Q]; // キーの重ね合わせ
        return false;
        break;
    }
  }
  return true;
}


// END OF naginata.c

NGKEYS naginata_keys;

// clang-format off
enum layers{
  _QWERTY,
  WIN_BASE,
  MAC_ASTARTE,
  MAC_ASTARTE_FN,
  WIN_ASTARTE,
  WIN_ASTARTE_FN,
  MAC_FN,
  WIN_FN,
  _EUCALYN,
  _WORKMAN,
// 薙刀式
  _NAGINATA, // 薙刀式入力レイヤー
// 薙刀式
  _LOWER,
  _RAISE,
  _ADJUST,
};


// // 薙刀式 // 新下駄
// enum combo_events {
//   NAGINATA_ON_CMB,
//   NAGINATA_OFF_CMB,
// };

// #if defined(DQWERTY)
// const uint16_t PROGMEM ngon_combo[] = {KC_H, KC_J, COMBO_END};
// const uint16_t PROGMEM ngoff_combo[] = {KC_F, KC_G, COMBO_END};
// #endif
// #if defined(DEUCALYN)
// const uint16_t PROGMEM ngon_combo[] = {KC_G, KC_T, COMBO_END};
// const uint16_t PROGMEM ngoff_combo[] = {KC_I, KC_U, COMBO_END};
// #endif
// #if defined(DWORKMAN)
// const uint16_t PROGMEM ngon_combo[] = {KC_Y, KC_N, COMBO_END};
// const uint16_t PROGMEM ngoff_combo[] = {KC_T, KC_G, COMBO_END};
// #endif

// combo_t key_combos[COMBO_COUNT] = {
//   [NAGINATA_ON_CMB] = COMBO_ACTION(ngon_combo),
//   [NAGINATA_OFF_CMB] = COMBO_ACTION(ngoff_combo),
// };

// // IME ONのcombo
// void process_combo_event(uint8_t combo_index, bool pressed) {
//   switch(combo_index) {
//     case NAGINATA_ON_CMB:
//       if (pressed) {
//         naginata_on();
//       }
//       break;
//     case NAGINATA_OFF_CMB:
//       if (pressed) {
//         naginata_off();
//       }
//       break;
//   }
// }
// // 薙刀式

enum custom_keycodes {
  QWERTY = NG_SAFE_RANGE,
  EUCALYN,
  WORKMAN,
  EISU,
  LOWER,
  RAISE,
  ADJUST,
  BACKLIT,
  KANA2,
  UNDGL,
  RGBRST,
  SWSGT // 新下駄 or 薙刀式のトグル
};

#define CTLTB CTL_T(KC_TAB)
#define GUITB GUI_T(KC_TAB)
#define ABLS    LALT(KC_BSLS)
#define CMDENT  CMD_T(KC_ENT)
#define SFTSPC  LSFT_T(KC_SPC)
#define CTLSPC  CTL_T(KC_SPC)
#define ALTSPC  ALT_T(KC_SPC)
#define ALTENT  ALT_T(KC_ENT)
#define CTLBS   CTL_T(KC_BSPC)
#define CTLENT  CTL_T(KC_ENT)

#define MO_MACB MO(_QWERTY)
#define MO_WINB MO(WIN_BASE)
#define MO_MAST MO(MAC_ASTARTE)
#define MO_WAST MO(WIN_ASTARTE)
#define MO_MAFN MO(MAC_ASTARTE_FN)
#define MO_WIFN MO(WIN_ASTARTE_FN)

#define TO_MACB TO(_QWERTY)
#define TO_WINB TO(WIN_BASE)
#define TO_MAST TO(MAC_ASTARTE)
#define TO_WAST TO(WIN_ASTARTE)
#define TO_MAFN TO(MAC_ASTARTE_FN)
#define TO_WIFN TO(WIN_ASTARTE_FN)

// this is K3 Pro default layer

// const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
//     [MAC_BASE] = LAYOUT(
//         KC_ESC,   KC_BRID,  KC_BRIU,  KC_MCTL,  KC_LPAD,  BL_DOWN,  BL_UP,    KC_MPRV,  KC_MPLY,  KC_MNXT,  KC_MUTE,  KC_VOLD,  KC_VOLU,  KC_SNAP,  KC_DEL,   BL_STEP,
//         KC_GRV,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_INT3,  KC_BSPC,  KC_PGUP,
//         KC_TAB,   KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,                      KC_PGDN,
//         KC_LCTL,  KC_A,     KC_S,     KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,  KC_NUHS,  KC_ENT,             KC_HOME,
//         KC_LSFT,            KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,  KC_INT1,  KC_RSFT,            KC_END,
//         KC_LCTL,  KC_LOPTN, KC_LCMMD, KC_LNG2,                      KC_SPC,                       KC_LNG1,  KC_RCMMD,MO(MAC_FN),KC_LEFT,  KC_UP,    KC_DOWN,  KC_RGHT),

//     [MAC_FN] = LAYOUT(
//         _______,  KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,   KC_F11,   KC_F12,   TO_MAST,  _______,  BL_TOGG,
//         _______,  BT_HST1,  BT_HST2,  BT_HST3,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,
//         BL_TOGG,  BL_STEP,  BL_UP,    _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,                      _______,
//         _______,  _______,  BL_DOWN,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,
//         _______,            _______,  _______,  _______,  _______,  BAT_LVL,  NK_TOGG,  _______,  _______,  _______,  _______,  _______,  _______,            _______,
//         _______,  _______,  _______,  _______,                      _______,                      _______,  _______,  _______,  _______,  _______,  _______,  _______),

//     [WIN_BASE] = LAYOUT(
//         KC_ESC,   KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,   KC_F11,   KC_F12,   KC_SNAP,  KC_DEL,   BL_STEP,
//         KC_GRV,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_INT3,  KC_BSPC,  KC_PGUP,
//         KC_TAB,   KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,                      KC_PGDN,
//         KC_LCTL,  KC_A,     KC_S,     KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,  KC_NUHS,  KC_ENT,             KC_HOME,
//         KC_LSFT,            KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,  KC_INT1,  KC_RSFT,            KC_END,
//         KC_LCTL,  KC_LGUI,  KC_LALT,  KC_INT5,                      KC_SPC,                       KC_INT4,  KC_RALT, MO(WIN_FN),KC_LEFT,  KC_UP,    KC_DOWN,  KC_RGHT),

//     [WIN_FN] = LAYOUT(
//         _______,  KC_BRID,  KC_BRIU,  KC_TASK,  KC_FILE,  BL_DOWN,  BL_UP,    KC_MPRV,  KC_MPLY,  KC_MNXT,  KC_MUTE,  KC_VOLD,  KC_VOLU,  TO_WAST,  _______,  BL_TOGG,
//         _______,  BT_HST1,  BT_HST2,  BT_HST3,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,
//         BL_TOGG,  BL_STEP,  BL_UP,    _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,                      _______,
//         _______,  _______,  BL_DOWN,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,
//         _______,            _______,  _______,  _______,  _______,  BAT_LVL,  NK_TOGG,  _______,  _______,  _______,  _______,  _______,  _______,            _______,
//         _______,  _______,  _______,  _______,                      _______,                      _______,  _______,  _______,  _______,  _______,  _______,  _______),

//     [MAC_ASTARTE] = LAYOUT(
//         KC_ESC,   KC_BRID,  KC_BRIU,  KC_MCTL,  KC_LPAD,  BL_DOWN,  BL_UP,    KC_MPRV,  KC_MPLY,  KC_MNXT,  KC_MUTE,  KC_VOLD,  KC_VOLU,  TO_MACB,  KC_DEL,   BL_STEP,
//         KC_GRV,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_INT3,  KC_BSPC,  KC_PGUP,
//         KC_TAB,   KC_Q,     KC_X,     KC_U,     KC_Y,     KC_SLSH,  KC_J,     KC_D,     KC_H,     KC_G,     KC_W,     KC_Z,     KC_LBRC,                      KC_PGDN,
//         KC_LCTL,  KC_I,     KC_O,     KC_E,     KC_A,     KC_DOT,   KC_K,     KC_T,     KC_N,     KC_S,     KC_R,     KC_P,     KC_NUHS,  KC_ENT,             KC_HOME,
//         KC_LSFT,            KC_QUOT,  KC_MINS,  KC_COMM,  KC_C,     KC_SCLN,  KC_M,     KC_L,     KC_L,     KC_B,     KC_V,     KC_INT1,  KC_RSFT,            KC_END,
//         KC_LCTL,  KC_LOPTN, KC_LCMMD, KC_LNG2,                      KC_SPC,                       KC_LNG1,  KC_RCMMD, MO_MAFN,  KC_LEFT,  KC_UP,    KC_DOWN,  KC_RGHT),

//     [MAC_ASTARTE_FN] = LAYOUT(
//         _______,  KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,   KC_F11,   KC_F12,   TO_MACB,  _______,  BL_TOGG,
//         _______,  BT_HST1,  BT_HST2,  BT_HST3,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,
//         BL_TOGG,  BL_STEP,  BL_UP,    _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,                      _______,
//         _______,  _______,  BL_DOWN,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,
//         _______,            _______,  _______,  _______,  _______,  BAT_LVL,  NK_TOGG,  _______,  _______,  _______,  _______,  _______,  _______,            _______,
//         _______,  _______,  _______,  _______,                      _______,                      _______,  _______,  _______,  _______,  _______,  _______,  _______),

//     [WIN_ASTARTE] = LAYOUT(
//         KC_ESC,   KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,   KC_F11,   KC_F12,   TO_WINB,  KC_DEL,   BL_STEP,
//         KC_GRV,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_INT3,  KC_BSPC,  KC_PGUP,
//         KC_TAB,   KC_Q,     KC_X,     KC_U,     KC_Y,     KC_SLSH,  KC_J,     KC_D,     KC_H,     KC_G,     KC_W,     KC_Z,     KC_LBRC,                      KC_PGDN,
//         KC_LCTL,  KC_I,     KC_O,     KC_E,     KC_A,     KC_DOT,   KC_K,     KC_T,     KC_N,     KC_S,     KC_R,     KC_P,     KC_NUHS,  KC_ENT,             KC_HOME,
//         KC_LSFT,            KC_QUOT,  KC_MINS,  KC_COMM,  KC_C,     KC_SCLN,  KC_M,     KC_L,     KC_L,     KC_B,     KC_V,     KC_INT1,  KC_RSFT,            KC_END,
//         KC_LCTL,  KC_LGUI,  KC_LALT,  KC_INT5,                      KC_SPC,                       KC_INT4,  KC_RALT,  MO_WIFN,  KC_LEFT,  KC_UP,    KC_DOWN,  KC_RGHT),

//     [WIN_ASTARTE_FN] = LAYOUT(
//         _______,  KC_BRID,  KC_BRIU,  KC_TASK,  KC_FILE,  BL_DOWN,  BL_UP,    KC_MPRV,  KC_MPLY,  KC_MNXT,  KC_MUTE,  KC_VOLD,  KC_VOLU,  TO_WINB,  _______,  BL_TOGG,
//         _______,  BT_HST1,  BT_HST2,  BT_HST3,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,
//         BL_TOGG,  BL_STEP,  BL_UP,    _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,                      _______,
//         _______,  _______,  BL_DOWN,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,
//         _______,            _______,  _______,  _______,  _______,  BAT_LVL,  NK_TOGG,  _______,  _______,  _______,  _______,  _______,  _______,            _______,
//         _______,  _______,  _______,  _______,                      _______,                      _______,  _______,  _______,  _______,  _______,  _______,  _______),
// };


// this is shingeta layout original

// const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
// /* _QWERTY
//   +------+------+------+------+------+------+------+------+------+------+------+------+------+------+
//   | ESC  |  Q   |  W   |  E   |  R   |  T   |      |      |  Y   |  U   |  I   |  O   |  P   | BSPC |
//   +------+------+------+------+------+------+------+------+------+------+------+------+------+------+
//   | TAB  |  A   |  S   |  D   |  F   |  G   |      |      |  H   |  J   |  K   |  L   |  ;   | RGUI |
//   +------+------+------+------+------+------+------+------+------+------+------+------+------+------+
//   | LSFT |  Z   |  X   |  C   |  V   |  B   |      |      |  N   |  M   |  ,   |  .   |  /   | RCMD |
//   +------+------+------+------+------+------+------+------+------+------+------+------+------+------+
//   |      |      |      |      |LOWER | LSFT |CTLSPC|ALTENT| RSFT |RAISE |      |      |      |      |
//   +------+------+------+------+------+------+------+------+------+------+------+------+------+------+
// */
//   [_QWERTY] = LAYOUT(
//     KC_ESC ,KC_Q   ,KC_W   ,KC_E   ,KC_R   ,KC_T   ,                KC_Y   ,KC_U   ,KC_I   ,KC_O   ,KC_P   ,KC_BSPC,
//     KC_TAB ,KC_A   ,KC_S   ,KC_D   ,KC_F   ,KC_G   ,                KC_H   ,KC_J   ,KC_K   ,KC_L   ,JP_SCLN,KC_RGUI,
//     KC_LSFT,KC_Z   ,KC_X   ,KC_C   ,KC_V   ,KC_B   ,                KC_N   ,KC_M   ,JP_COMM,JP_DOT ,JP_SLSH,KC_RCMD,
//                                     LOWER  ,KC_LSFT,CTLSPC ,ALTENT ,KC_RSFT,RAISE
//   ),

// /* _LOWER
//   +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
//   | ESC |  ~  |  @  |  #  |  $  |  %  |     |     |  /  |  7  |  8  |  9  |  -  |BSPC |
//   +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
//   | TAB |  ^  |  &  |  !  |  ?  |  \  |     |     |  *  |  4  |  5  |  6  |  +  |  .  |
//   +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
//   |LSFT |  |  |  `  |  '  |  "  |  _  |     |     |  0  |  1  |  2  |  3  |  =  |  ,  |
//   +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
//   |     |     |     |     | __  | __  | __  | __  | __  | __  |     |     |     |     |
//   +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
// */
//   [_LOWER] = LAYOUT(
//     KC_ESC ,JP_TILD,JP_AT  ,JP_HASH,JP_DLR ,JP_PERC,                JP_SLSH,KC_7   ,KC_8   ,KC_9   ,JP_MINS,KC_BSPC,
//     KC_TAB ,JP_CIRC,JP_AMPR,JP_EXLM,JP_QUES,JP_BSLS,                JP_ASTR,KC_4   ,KC_5   ,KC_6   ,JP_PLUS,JP_DOT ,
//     KC_LSFT,JP_PIPE,JP_GRV ,JP_QUOT,JP_DQUO ,JP_UNDS,                KC_0   ,KC_1   ,KC_2   ,KC_3   ,JP_EQL ,JP_COMM,
//                                     _______,_______,_______,_______,KANA2  ,_______
//   ),

// /* _RAISE
//   +-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
//   |  ESC  |       |       |       |       |       |       |       |       |       |  UP   |       | PGUP  |       |
//   +-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
//   |  TAB  |       |   [   |   {   |   (   |   <   |       |       | HOME  | LEFT  | DOWN  | RGHT  | PGDN  |       |
//   +-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
//   | LSFT  |       |   ]   |   }   |   )   |   >   |       |       |  END  |S(LEFT)|S(DOWN)|S(RGHT)|       |       |
//   +-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
//   |       |       |       |       |  __   |  __   |  __   |  __   |  __   |  __   |       |       |       |       |
//   +-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
// */
//   [_RAISE] = LAYOUT(
//     KC_ESC    ,XXXXXXX   ,XXXXXXX   ,XXXXXXX   ,XXXXXXX   ,XXXXXXX   ,                      XXXXXXX   ,XXXXXXX   ,KC_UP     ,XXXXXXX   ,KC_PGUP   ,XXXXXXX   ,
//     KC_TAB    ,XXXXXXX   ,JP_LBRC   ,JP_LCBR   ,JP_LPRN   ,KC_LT     ,                      KC_HOME   ,KC_LEFT   ,KC_DOWN   ,KC_RGHT   ,KC_PGDN   ,XXXXXXX   ,
//     KC_LSFT   ,XXXXXXX   ,JP_RBRC   ,JP_RCBR   ,JP_RPRN   ,KC_GT     ,                      KC_END    ,S(KC_LEFT),S(KC_DOWN),S(KC_RGHT),XXXXXXX   ,XXXXXXX   ,
//                                                 _______   ,EISU      ,_______   ,_______   ,_______   ,_______
//   ),

// /* _ADJUST
//   +-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
//   |RGB_HUI|RGB_HUD| WAKE  |       | RESET | UNDGL |       |       |RGB_TOG|       |KC_VOLU|       |KC_BRIU|QWERTY |
//   +-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
//   |RGB_SAI|RGB_SAD|       |       |       |       |       |       |RGB_MOD|KC_MRWD|KC_VOLD|KC_MFFD|KC_BRID|EUCALYN|
//   +-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
//   |RGB_VAI|RGB_VAD| SLEP  |       |       |       |       |       |RGBRST |       |KC_MPLY|       |       |WORKMAN|
//   +-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
//   |       |       |       |       |  __   |  __   |  __   |  __   |  __   |  __   |       |       |       |       |
//   +-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
// */
//   [_ADJUST] = LAYOUT(
//     RGB_HUI,RGB_HUD,KC_WAKE,XXXXXXX,RESET  ,UNDGL  ,                RGB_TOG,XXXXXXX,KC_VOLU,XXXXXXX,KC_BRIU,QWERTY ,
//     RGB_SAI,RGB_SAD,XXXXXXX,XXXXXXX,XXXXXXX,XXXXXXX,                RGB_MOD,KC_MRWD,KC_VOLD,KC_MFFD,KC_BRID,EUCALYN,
//     RGB_VAI,RGB_VAD,KC_SLEP,XXXXXXX,XXXXXXX,XXXXXXX,                RGBRST ,XXXXXXX,KC_MPLY,XXXXXXX,XXXXXXX,WORKMAN,
//                                     _______,_______,_______,_______,_______,_______
//   ),

// /* _EUCALYN
//   +------+------+------+------+------+------+------+------+------+------+------+------+------+------+
//   | ESC  |  Q   |  W   |  ,   |  .   |  ;   |      |      |  M   |  R   |  D   |  Y   |  P   | BSPC |
//   +------+------+------+------+------+------+------+------+------+------+------+------+------+------+
//   | TAB  |  A   |  O   |  E   |  I   |  U   |      |      |  G   |  T   |  K   |  S   |  N   | RGUI |
//   +------+------+------+------+------+------+------+------+------+------+------+------+------+------+
//   | LSFT |  Z   |  X   |  C   |  V   |  F   |      |      |  B   |  H   |  J   |  L   |  /   | RCMD |
//   +------+------+------+------+------+------+------+------+------+------+------+------+------+------+
//   |      |      |      |      |LOWER | LSFT |CTLSPC|ALTENT| RSFT |RAISE |      |      |      |      |
//   +------+------+------+------+------+------+------+------+------+------+------+------+------+------+
// */
//   [_EUCALYN] = LAYOUT(
//     KC_ESC ,KC_Q   ,KC_W   ,JP_COMM,JP_DOT ,JP_SCLN,                KC_M   ,KC_R   ,KC_D   ,KC_Y   ,KC_P   ,KC_BSPC,
//     KC_TAB ,KC_A   ,KC_O   ,KC_E   ,KC_I   ,KC_U   ,                KC_G   ,KC_T   ,KC_K   ,KC_S   ,KC_N   ,KC_RGUI,
//     KC_LSFT,KC_Z   ,KC_X   ,KC_C   ,KC_V   ,KC_F   ,                KC_B   ,KC_H   ,KC_J   ,KC_L   ,JP_SLSH,KC_RCMD,
//                                     LOWER  ,KC_LSFT,CTLSPC ,ALTENT ,KC_RSFT,RAISE
//   ),

// /* _WORKMAN
//   +------+------+------+------+------+------+------+------+------+------+------+------+------+------+
//   | ESC  |  Q   |  D   |  R   |  W   |  B   |      |      |  J   |  F   |  U   |  P   |  ;   | BSPC |
//   +------+------+------+------+------+------+------+------+------+------+------+------+------+------+
//   | TAB  |  A   |  S   |  H   |  T   |  G   |      |      |  Y   |  N   |  E   |  O   |  I   | RGUI |
//   +------+------+------+------+------+------+------+------+------+------+------+------+------+------+
//   | LSFT |  Z   |  X   |  M   |  C   |  V   |      |      |  K   |  L   |  ,   |  .   |  /   | RCMD |
//   +------+------+------+------+------+------+------+------+------+------+------+------+------+------+
//   |      |      |      |      |LOWER | LSFT |CTLSPC|ALTENT| RSFT |RAISE |      |      |      |      |
//   +------+------+------+------+------+------+------+------+------+------+------+------+------+------+
// */
//   [_WORKMAN] = LAYOUT(
//     KC_ESC ,KC_Q   ,KC_D   ,KC_R   ,KC_W   ,KC_B   ,                KC_J   ,KC_F   ,KC_U   ,KC_P   ,JP_SCLN,KC_BSPC,
//     KC_TAB ,KC_A   ,KC_S   ,KC_H   ,KC_T   ,KC_G   ,                KC_Y   ,KC_N   ,KC_E   ,KC_O   ,KC_I   ,KC_RGUI,
//     KC_LSFT,KC_Z   ,KC_X   ,KC_M   ,KC_C   ,KC_V   ,                KC_K   ,KC_L   ,JP_COMM,JP_DOT ,JP_SLSH,KC_RCMD,
//                                     LOWER  ,KC_LSFT,CTLSPC ,ALTENT ,KC_RSFT,RAISE
//   ),

// /* _NAGINATA
//   +-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
//   |  __   | NG_Q  | NG_W  | NG_E  | NG_R  | NG_T  |       |       | NG_Y  | NG_U  | NG_I  | NG_O  | NG_P  |  __   |
//   +-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
//   |  __   | NG_A  | NG_S  | NG_D  | NG_F  | NG_G  |       |       | NG_H  | NG_J  | NG_K  | NG_L  |NG_SCLN|  __   |
//   +-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
//   |  __   | NG_Z  | NG_X  | NG_C  | NG_V  | NG_B  |       |       | NG_N  | NG_M  |NG_COMM|NG_DOT |NG_SLSH|  __   |
//   +-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
//   |       |       |       |       | LOWER |NG_SHFT|  __   |  __   |NG_SHFT| RAISE |       |       |       |       |
//   +-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
// */
// // 薙刀式 // 新下駄
//   [_NAGINATA] = LAYOUT(
//     _______,NG_Q   ,NG_W   ,NG_E   ,NG_R   ,NG_T   ,                NG_Y   ,NG_U   ,NG_I   ,NG_O   ,NG_P   ,NG_X1  ,
//     _______,NG_A   ,NG_S   ,NG_D   ,NG_F   ,NG_G   ,                NG_H   ,NG_J   ,NG_K   ,NG_L   ,NG_SCLN,KC_BSPC,
//     _______,NG_Z   ,NG_X   ,NG_C   ,NG_V   ,NG_B   ,                NG_N   ,NG_M   ,NG_COMM,NG_DOT ,NG_SLSH,KC_BSLS,
//                                     _______,_______,_______,_______,_______,_______
//   ),
// // 薙刀式

// };

// mix above for K3 Pro

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
//     [MAC_BASE] = LAYOUT(
//         KC_ESC,   KC_BRID,  KC_BRIU,  KC_MCTL,  KC_LPAD,  BL_DOWN,  BL_UP,    KC_MPRV,  KC_MPLY,  KC_MNXT,  KC_MUTE,  KC_VOLD,  KC_VOLU,  KC_SNAP,  KC_DEL,   BL_STEP,
//         KC_GRV,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_INT3,  KC_BSPC,  KC_PGUP,
//         KC_TAB,   KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,                      KC_PGDN,
//         KC_LCTL,  KC_A,     KC_S,     KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,  KC_NUHS,  KC_ENT,             KC_HOME,
//         KC_LSFT,            KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,  KC_INT1,  KC_RSFT,            KC_END,
//         KC_LCTL,  KC_LOPTN, KC_LCMMD, KC_LNG2,                      KC_SPC,                       KC_LNG1,  KC_RCMMD,MO(MAC_FN),KC_LEFT,  KC_UP,    KC_DOWN,  KC_RGHT),     

    [_QWERTY] = LAYOUT(
        KC_ESC,   KC_BRID,  KC_BRIU,  KC_MCTL,  KC_LPAD,  BL_DOWN,  BL_UP,    KC_MPRV,  KC_MPLY,  KC_MNXT,  KC_MUTE,  KC_VOLD,  KC_VOLU,  KC_SNAP,  KC_DEL,   BL_STEP,
        KC_GRV,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_INT3,  KC_BSPC,  KC_PGUP,
        KC_TAB,   KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,                      KC_PGDN,
        KC_LCTL,  KC_A,     KC_S,     KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,  KC_NUHS,  KC_ENT,             KC_HOME,
        KC_LSFT,            KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,  KC_INT1,  KC_RSFT,            KC_END,
        KC_LCTL,  KC_LOPTN, KC_LCMMD, KC_LNG2,                      KC_SPC,                       KC_LNG1,  KC_RCMMD,MO(MAC_FN),KC_LEFT,  KC_UP,    KC_DOWN,  KC_RGHT),
    
    [MAC_ASTARTE] = LAYOUT(
        KC_ESC,   KC_BRID,  KC_BRIU,  KC_MCTL,  KC_LPAD,  BL_DOWN,  BL_UP,    KC_MPRV,  KC_MPLY,  KC_MNXT,  KC_MUTE,  KC_VOLD,  KC_VOLU,  TO_MACB,  KC_DEL,   BL_STEP,
        KC_GRV,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_INT3,  KC_BSPC,  KC_PGUP,
        KC_TAB,   KC_Q,     KC_X,     KC_U,     KC_Y,     KC_SLSH,  KC_J,     KC_D,     KC_H,     KC_G,     KC_W,     KC_Z,     KC_LBRC,                      KC_PGDN,
        KC_LCTL,  KC_I,     KC_O,     KC_E,     KC_A,     KC_DOT,   KC_K,     KC_T,     KC_N,     KC_S,     KC_R,     KC_P,     KC_NUHS,  KC_ENT,             KC_HOME,
        KC_LSFT,            KC_QUOT,  KC_MINS,  KC_COMM,  KC_C,     KC_SCLN,  KC_M,     KC_L,     KC_F,     KC_B,     KC_V,     KC_INT1,  KC_RSFT,            KC_END,
        KC_LCTL,  KC_LOPTN, KC_LCMMD, KC_LNG2,                      KC_SPC,                       KC_LNG1,  KC_RCMMD, MO_MAFN,  KC_LEFT,  KC_UP,    KC_DOWN,  KC_RGHT),

    [MAC_ASTARTE_FN] = LAYOUT(
        _______,  KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,   KC_F11,   KC_F12,   TO_MACB,  TO_WINB,  SWSGT,
        _______,  BT_HST1,  BT_HST2,  BT_HST3,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,
        BL_TOGG,  BL_STEP,  BL_UP,    _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,                      _______,
        _______,  _______,  BL_DOWN,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,
        _______,            _______,  _______,  _______,  _______,  BAT_LVL,  NK_TOGG,  _______,  _______,  _______,  _______,  _______,  _______,            _______,
        _______,  _______,  _______,  _______,                      _______,                      _______,  _______,  _______,  _______,  _______,  _______,  _______),

    // WORKAROUND: just define as empty for test
    [_LOWER] = LAYOUT(
        _______, _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,
        _______, _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,
        _______, _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,                      _______,
        _______, _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,
        _______,           _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,
        _______, _______,  _______,  _______,                      _______,                      _______,  _______,  _______,  _______,  _______,  _______,  _______),

    [_RAISE] = LAYOUT(
        _______, _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,
        _______, _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,
        _______, _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,                      _______,
        _______, _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,
        _______,           _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,
        _______, _______,  _______,  _______,                      _______,                      _______,  _______,  _______,  _______,  _______,  _______,  _______),

    [_ADJUST] = LAYOUT(
        _______, _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,
        _______, _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,
        _______, _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,                      _______,
        _______, _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,
        _______,           _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,
        _______, _______,  _______,  _______,                      _______,                      _______,  _______,  _______,  _______,  _______,  _______,  _______),

    [_EUCALYN] = LAYOUT(
        _______, _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,
        _______, _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,
        _______, _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,                      _______,
        _______, _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,
        _______,           _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,
        _______, _______,  _______,  _______,                      _______,                      _______,  _______,  _______,  _______,  _______,  _______,  _______),

    [_WORKMAN] = LAYOUT(
        _______, _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,
        _______, _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,
        _______, _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,                      _______,
        _______, _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,
        _______,           _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,
        _______, _______,  _______,  _______,                      _______,                      _______,  _______,  _______,  _______,  _______,  _______,  _______),

// /* _NAGINATA
//   +-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
//   |  __   | NG_Q  | NG_W  | NG_E  | NG_R  | NG_T  |       |       | NG_Y  | NG_U  | NG_I  | NG_O  | NG_P  |  __   |
//   +-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
//   |  __   | NG_A  | NG_S  | NG_D  | NG_F  | NG_G  |       |       | NG_H  | NG_J  | NG_K  | NG_L  |NG_SCLN|  __   |
//   +-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
//   |  __   | NG_Z  | NG_X  | NG_C  | NG_V  | NG_B  |       |       | NG_N  | NG_M  |NG_COMM|NG_DOT |NG_SLSH|  __   |
//   +-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
//   |       |       |       |       | LOWER |NG_SHFT|  __   |  __   |NG_SHFT| RAISE |       |       |       |       |
//   +-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
// */

//     [MAC_BASE] = LAYOUT(
//         KC_ESC,   KC_BRID,  KC_BRIU,  KC_MCTL,  KC_LPAD,  BL_DOWN,  BL_UP,    KC_MPRV,  KC_MPLY,  KC_MNXT,  KC_MUTE,  KC_VOLD,  KC_VOLU,  KC_SNAP,  KC_DEL,   BL_STEP,
//         KC_GRV,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_INT3,  KC_BSPC,  KC_PGUP,
//         KC_TAB,   KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,                      KC_PGDN,
//         KC_LCTL,  KC_A,     KC_S,     KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,  KC_NUHS,  KC_ENT,             KC_HOME,
//         KC_LSFT,            KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,  KC_INT1,  KC_RSFT,            KC_END,
//         KC_LCTL,  KC_LOPTN, KC_LCMMD, KC_LNG2,                      KC_SPC,                       KC_LNG1,  KC_RCMMD,MO(MAC_FN),KC_LEFT,  KC_UP,    KC_DOWN,  KC_RGHT),  

    [_NAGINATA] = LAYOUT(
        _______, _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,
        _______, NG_1,     NG_2,     NG_3,     NG_4,     NG_5,     NG_6,     NG_7,     NG_8,     NG_9,     NG_0,     NG_MINS,  NG_HAT,   NG_YEN,   KC_BSPC,  KC_PGUP,
        KC_TAB,  NG_Q,     NG_W,     NG_E,     NG_R,     NG_T,     NG_Y,     NG_U,     NG_I,     NG_O,     NG_P,     NG_X1,    NG_LB,                        KC_PGDN,
        KC_LCTL, NG_A,     NG_S,     NG_D,     NG_F,     NG_G,     NG_H,     NG_J,     NG_K,     NG_L,     NG_SCLN,  NG_COLO,  NG_RB,    _______,            _______,
        NG_LSFT,           NG_Z,     NG_X,     NG_C,     NG_V,     NG_B,     NG_N,     NG_M,     NG_COMM,  NG_DOT,   NG_SLSH,  NG_BSLSH, _______,            _______,
        KC_LCTL, _______,  _______,  _______,                      NG_SHFT,                      _______,  _______,  _______,  _______,  _______,  _______,  _______),

    [MAC_FN] = LAYOUT(
        _______,  KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,   KC_F11,   KC_F12,   TO_MAST,  TO_WINB,  SWSGT,
        _______,  BT_HST1,  BT_HST2,  BT_HST3,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,
        BL_TOGG,  BL_STEP,  BL_UP,    _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,                      _______,
        _______,  _______,  BL_DOWN,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,
        _______,            _______,  _______,  _______,  _______,  BAT_LVL,  NK_TOGG,  _______,  _______,  _______,  _______,  _______,  _______,            _______,
        _______,  _______,  _______,  _______,                      _______,                      _______,  _______,  _______,  _______,  _______,  _______,  _______),

    [WIN_BASE] = LAYOUT(
        KC_ESC,   KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,   KC_F11,   KC_F12,   KC_SNAP,  KC_DEL,   BL_STEP,
        KC_GRV,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_INT3,  KC_BSPC,  KC_PGUP,
        KC_TAB,   KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,                      KC_PGDN,
        KC_LCTL,  KC_A,     KC_S,     KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,  KC_NUHS,  KC_ENT,             KC_HOME,
        KC_LSFT,            KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,  KC_INT1,  KC_RSFT,            KC_END,
        KC_LCTL,  KC_LGUI,  KC_LALT,  KC_INT5,                      KC_SPC,                       KC_INT4,  KC_RALT, MO(WIN_FN),KC_LEFT,  KC_UP,    KC_DOWN,  KC_RGHT),

    [WIN_FN] = LAYOUT(
        _______,  KC_BRID,  KC_BRIU,  KC_TASK,  KC_FILE,  BL_DOWN,  BL_UP,    KC_MPRV,  KC_MPLY,  KC_MNXT,  KC_MUTE,  KC_VOLD,  KC_VOLU,  TO_WAST,  TO_MACB,  SWSGT,
        _______,  BT_HST1,  BT_HST2,  BT_HST3,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,
        BL_TOGG,  BL_STEP,  BL_UP,    _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,                      _______,
        _______,  _______,  BL_DOWN,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,
        _______,            _______,  _______,  _______,  _______,  BAT_LVL,  NK_TOGG,  _______,  _______,  _______,  _______,  _______,  _______,            _______,
        _______,  _______,  _______,  _______,                      _______,                      _______,  _______,  _______,  _______,  _______,  _______,  _______),

    [WIN_ASTARTE] = LAYOUT(
        KC_ESC,   KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,   KC_F11,   KC_F12,   TO_WINB,  KC_DEL,   BL_STEP,
        KC_GRV,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_INT3,  KC_BSPC,  KC_PGUP,
        KC_TAB,   KC_Q,     KC_X,     KC_U,     KC_Y,     KC_SLSH,  KC_J,     KC_D,     KC_H,     KC_G,     KC_W,     KC_Z,     KC_LBRC,                      KC_PGDN,
        KC_LCTL,  KC_I,     KC_O,     KC_E,     KC_A,     KC_DOT,   KC_K,     KC_T,     KC_N,     KC_S,     KC_R,     KC_P,     KC_NUHS,  KC_ENT,             KC_HOME,
        KC_LSFT,            KC_QUOT,  KC_MINS,  KC_COMM,  KC_C,     KC_SCLN,  KC_M,     KC_L,     KC_L,     KC_B,     KC_V,     KC_INT1,  KC_RSFT,            KC_END,
        KC_LCTL,  KC_LGUI,  KC_LALT,  KC_INT5,                      KC_SPC,                       KC_INT4,  KC_RALT,  MO_WIFN,  KC_LEFT,  KC_UP,    KC_DOWN,  KC_RGHT),

    [WIN_ASTARTE_FN] = LAYOUT(
        _______,  KC_BRID,  KC_BRIU,  KC_TASK,  KC_FILE,  BL_DOWN,  BL_UP,    KC_MPRV,  KC_MPLY,  KC_MNXT,  KC_MUTE,  KC_VOLD,  KC_VOLU,  TO_WINB,  TO_MACB,  SWSGT,
        _______,  BT_HST1,  BT_HST2,  BT_HST3,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,
        BL_TOGG,  BL_STEP,  BL_UP,    _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,                      _______,
        _______,  _______,  BL_DOWN,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,
        _______,            _______,  _______,  _______,  _______,  BAT_LVL,  NK_TOGG,  _______,  _______,  _______,  _______,  _______,  _______,            _______,
        _______,  _______,  _______,  _______,                      _______,                      _______,  _______,  _______,  _______,  _______,  _______,  _______),
};

int RGB_current_mode;

void persistent_default_layer_set(uint16_t default_layer) {
  eeconfig_update_default_layer(default_layer);
  default_layer_set(default_layer);
}

// Setting ADJUST layer RGB back to default
void update_tri_layer_RGB(uint8_t layer1, uint8_t layer2, uint8_t layer3) {
  if (IS_LAYER_ON(layer1) && IS_LAYER_ON(layer2)) {
    layer_on(layer3);
  } else {
    layer_off(layer3);
  }
}

void matrix_init_user(void) {
  // 薙刀式 // 新下駄
  set_naginata(_NAGINATA);
  // #ifdef NAGINATA_EDIT_MAC
  // set_unicode_input_mode(UC_OSX);
  // #endif
  // #ifdef NAGINATA_EDIT_WIN
  // set_unicode_input_mode(UC_WINC);
  // #endif
  // 薙刀式
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  if (record->event.pressed) {
    // set_timelog();
  }

  switch (keycode) {
    case SWSGT: // toggle is_shingeta
        if (record->event.pressed) {
            is_shingeta = !is_shingeta;
        }
        return false;
        break;
    case QWERTY:
      if (record->event.pressed) {
        persistent_default_layer_set(1UL<<_QWERTY);
      }
      return false;
      break;
    case EUCALYN:
      if (record->event.pressed) {
        persistent_default_layer_set(1UL<<_EUCALYN);
      }
      return false;
      break;
    case WORKMAN:
      if (record->event.pressed) {
        persistent_default_layer_set(1UL<<_WORKMAN);
      }
      return false;
      break;
    case KC_LNG2: // EISU:
      if (record->event.pressed) {
        // 薙刀式
        naginata_off();
      }
      return false;
      break;
    case KC_LNG1: // KANA
      if (record->event.pressed) {
        // 薙刀式
        naginata_on();
      }
      return false;
      break;
    case ADJUST:
      if (record->event.pressed) {
        layer_on(_ADJUST);
      } else {
        layer_off(_ADJUST);
      }
      return false;
      break;
    case LOWER:
      if (record->event.pressed) {
        layer_on(_LOWER);
      } else {
        layer_off(_LOWER);
      }
      update_tri_layer(_LOWER, _RAISE, _ADJUST);
      return false;
      break;
    case RAISE:
      if (record->event.pressed) {
        layer_on(_RAISE);
      } else {
        layer_off(_RAISE);
      }
      update_tri_layer(_LOWER, _RAISE, _ADJUST);
      return false;
      break;
  }

  // 薙刀式
  bool a = true;
  if (naginata_state()) {
    naginata_mode(keycode, record);
    a = process_naginata(keycode, record);
  }
  if (a == false) return false;
  // 薙刀式

  return true;
}
