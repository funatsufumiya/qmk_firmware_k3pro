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
  #define memcpy_P(des, src, len) memcpy(des, src, len)
#endif

#define NGBUFFER 5 // バッファのサイズ

static uint8_t ng_chrcount = 0; // 文字キー入力のカウンタ (シフトキーを除く)
static bool is_naginata = false; // 薙刀式がオンかオフか
static uint8_t naginata_layer = 0; // レイヤー番号
static uint8_t n_modifier = 0; // 押しているmodifierキーの数
static uint64_t keycomb = (uint64_t)0; // 新下駄 // 同時押しの状態を示す。64bitの各ビットがキーに対応する。

static bool is_shingeta = true;

// 43ーを64bitの各ビットに割り当てる // 新下駄
#define B_Q    ((uint64_t)1<<0)
#define B_W    ((uint64_t)1<<1)
#define B_E    ((uint64_t)1<<2)
#define B_R    ((uint64_t)1<<3)
#define B_T    ((uint64_t)1<<4)

#define B_Y    ((uint64_t)1<<5)
#define B_U    ((uint64_t)1<<6)
#define B_I    ((uint64_t)1<<7)
#define B_O    ((uint64_t)1<<8)
#define B_P    ((uint64_t)1<<9)

#define B_A    ((uint64_t)1<<10)
#define B_S    ((uint64_t)1<<11)
#define B_D    ((uint64_t)1<<12)
#define B_F    ((uint64_t)1<<13)
#define B_G    ((uint64_t)1<<14)

#define B_H    ((uint64_t)1<<15)
#define B_J    ((uint64_t)1<<16)
#define B_K    ((uint64_t)1<<17)
#define B_L    ((uint64_t)1<<18)
#define B_SCLN ((uint64_t)1<<19)

#define B_Z    ((uint64_t)1<<20)
#define B_X    ((uint64_t)1<<21)
#define B_C    ((uint64_t)1<<22)
#define B_V    ((uint64_t)1<<23)
#define B_B    ((uint64_t)1<<24)

#define B_N    ((uint64_t)1<<25)
#define B_M    ((uint64_t)1<<26)
#define B_COMM ((uint64_t)1<<27)
#define B_DOT  ((uint64_t)1<<28)
#define B_SLSH ((uint64_t)1<<29)

#define B_1    ((uint64_t)1<<30) // 新下駄
#define B_2    ((uint64_t)1<<31) // 新下駄
#define B_3    ((uint64_t)1<<32) // 新下駄
#define B_4    ((uint64_t)1<<33) // 新下駄
#define B_5    ((uint64_t)1<<34) // 新下駄
#define B_6    ((uint64_t)1<<35) // 新下駄
#define B_7    ((uint64_t)1<<36) // 新下駄
#define B_8    ((uint64_t)1<<37) // 新下駄
#define B_9    ((uint64_t)1<<38) // 新下駄
#define B_0    ((uint64_t)1<<39) // 新下駄
#define B_MINS ((uint64_t)1<<40) // 新下駄

#define B_X1   ((uint64_t)1<<41) // 新下駄

#define B_SHFT ((uint64_t)1<<42)

// 文字入力バッファ
static uint16_t ninputs[NGBUFFER];

// キーコードとキービットの対応
// メモリ削減のため配列はNG_Qを0にしている
const uint64_t ng_key[] = { // 新下駄
  [NG_Q    - NG_Q] = B_Q,
  [NG_W    - NG_Q] = B_W,
  [NG_E    - NG_Q] = B_E,
  [NG_R    - NG_Q] = B_R,
  [NG_T    - NG_Q] = B_T,

  [NG_Y    - NG_Q] = B_Y,
  [NG_U    - NG_Q] = B_U,
  [NG_I    - NG_Q] = B_I,
  [NG_O    - NG_Q] = B_O,
  [NG_P    - NG_Q] = B_P,

  [NG_A    - NG_Q] = B_A,
  [NG_S    - NG_Q] = B_S,
  [NG_D    - NG_Q] = B_D,
  [NG_F    - NG_Q] = B_F,
  [NG_G    - NG_Q] = B_G,

  [NG_H    - NG_Q] = B_H,
  [NG_J    - NG_Q] = B_J,
  [NG_K    - NG_Q] = B_K,
  [NG_L    - NG_Q] = B_L,
  [NG_SCLN - NG_Q] = B_SCLN,

  [NG_Z    - NG_Q] = B_Z,
  [NG_X    - NG_Q] = B_X,
  [NG_C    - NG_Q] = B_C,
  [NG_V    - NG_Q] = B_V,
  [NG_B    - NG_Q] = B_B,

  [NG_N    - NG_Q] = B_N,
  [NG_M    - NG_Q] = B_M,
  [NG_COMM - NG_Q] = B_COMM,
  [NG_DOT  - NG_Q] = B_DOT,
  [NG_SLSH - NG_Q] = B_SLSH,

  [NG_1    - NG_Q] = B_1, // 新下駄
  [NG_2    - NG_Q] = B_2, // 新下駄
  [NG_3    - NG_Q] = B_3, // 新下駄
  [NG_4    - NG_Q] = B_4, // 新下駄
  [NG_5    - NG_Q] = B_5, // 新下駄
  [NG_6    - NG_Q] = B_6, // 新下駄
  [NG_7    - NG_Q] = B_7, // 新下駄
  [NG_8    - NG_Q] = B_8, // 新下駄
  [NG_9    - NG_Q] = B_9, // 新下駄
  [NG_0    - NG_Q] = B_0, // 新下駄
  [NG_MINS - NG_Q] = B_MINS, // 新下駄

  [NG_X1   - NG_Q] = B_X1, // 新下駄

  [NG_SHFT - NG_Q] = B_SHFT, // 新下駄
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
  {.key = B_1               , .kana = "1"},
  {.key = B_2               , .kana = "2"},
  {.key = B_3               , .kana = "3"},
  {.key = B_4               , .kana = "4"},
  {.key = B_5               , .kana = "5"},
  {.key = B_6               , .kana = "6"},
  {.key = B_7               , .kana = "7"},
  {.key = B_8               , .kana = "8"},
  {.key = B_9               , .kana = "9"},
  {.key = B_0               , .kana = "0"},
  {.key = B_MINS            , .kana = "-"},
  {.key = B_SHFT            , .kana = " "},

  {.key = B_Q               , .kana = "-"},
  {.key = B_W               , .kana = "ni"},
  {.key = B_E               , .kana = "ha"},
  {.key = B_R               , .kana = ","},
  {.key = B_T               , .kana = "ti"},
  {.key = B_Y               , .kana = "gu"},
  {.key = B_U               , .kana = "ba"},
  {.key = B_I               , .kana = "ko"},
  {.key = B_O               , .kana = "ga"},
  {.key = B_P               , .kana = "hi"},
  {.key = B_X1              , .kana = "ge"},

  {.key = B_A               , .kana = "no"},
  {.key = B_S               , .kana = "to"},
  {.key = B_D               , .kana = "ka"},
  {.key = B_F               , .kana = "nn"},
  {.key = B_G               , .kana = "ltu"},
  {.key = B_H               , .kana = "ku"},
  {.key = B_J               , .kana = "u"},
  {.key = B_K               , .kana = "i"},
  {.key = B_L               , .kana = "si"},
  {.key = B_SCLN            , .kana = "na"},

  {.key = B_Z               , .kana = "su"},
  {.key = B_X               , .kana = "ma"},
  {.key = B_C               , .kana = "ki"},
  {.key = B_V               , .kana = "ru"},
  {.key = B_B               , .kana = "tu"},
  {.key = B_N               , .kana = "te"},
  {.key = B_M               , .kana = "ta"},
  {.key = B_COMM            , .kana = "de"},
  {.key = B_DOT             , .kana = "."},
  {.key = B_SLSH            , .kana = "bu"},

  // 中指シフト
  {.key = B_K|B_Q        , .kana = "fa"},
  {.key = B_K|B_W        , .kana = "go"},
  {.key = B_K|B_E        , .kana = "hu"},
  {.key = B_K|B_R        , .kana = "fi"},
  {.key = B_K|B_T        , .kana = "fe"},
  {.key = B_D|B_Y        , .kana = "wi"},
  {.key = B_D|B_U        , .kana = "pa"},
  {.key = B_D|B_I        , .kana = "yo"},
  {.key = B_D|B_O        , .kana = "mi"},
  {.key = B_D|B_P        , .kana = "we"},
  {.key = B_D|B_X1       , .kana = "ulo"},

  {.key = B_K|B_A        , .kana = "ho"},
  {.key = B_K|B_S        , .kana = "ji"},
  {.key = B_K|B_D        , .kana = "re"},
  {.key = B_K|B_F        , .kana = "mo"},
  {.key = B_K|B_G        , .kana = "yu"},
  {.key = B_D|B_H        , .kana = "he"},
  {.key = B_D|B_J        , .kana = "a"},
  {.key = B_D|B_K        , .kana = ""},
  {.key = B_D|B_L        , .kana = "o"},
  {.key = B_D|B_SCLN     , .kana = "e"},

  {.key = B_K|B_Z        , .kana = "du"},
  {.key = B_K|B_X        , .kana = "zo"},
  {.key = B_K|B_C        , .kana = "bo"},
  {.key = B_K|B_V        , .kana = "mu"},
  {.key = B_K|B_B        , .kana = "fo"},
  {.key = B_D|B_N        , .kana = "se"},
  {.key = B_D|B_M        , .kana = "ne"},
  {.key = B_D|B_COMM     , .kana = "be"},
  {.key = B_D|B_DOT      , .kana = "pu"},
  {.key = B_D|B_SLSH     , .kana = "vu"},

  {.key = B_K|B_1        , .kana = "la"},
  {.key = B_K|B_2        , .kana = "li"},
  {.key = B_K|B_3        , .kana = "lu"},
  {.key = B_K|B_4        , .kana = "le"},
  {.key = B_K|B_5        , .kana = "lo"},

  // 薬指シフト
  {.key = B_L|B_Q        , .kana = "di"},
  {.key = B_L|B_W        , .kana = "me"},
  {.key = B_L|B_E        , .kana = "ke"},
  {.key = B_L|B_R        , .kana = "teli"},
  {.key = B_L|B_T        , .kana = "deli"},
  {.key = B_S|B_Y        , .kana = "sye"},
  {.key = B_S|B_U        , .kana = "pe"},
  {.key = B_S|B_I        , .kana = "do"},
  {.key = B_S|B_O        , .kana = "ya"},
  {.key = B_S|B_P        , .kana = "je"},

  {.key = B_L|B_A        , .kana = "wo"},
  {.key = B_L|B_S        , .kana = "sa"},
  {.key = B_L|B_D        , .kana = "o"},
  {.key = B_L|B_F        , .kana = "ri"},
  {.key = B_L|B_G        , .kana = "zu"},
  {.key = B_S|B_H        , .kana = "bi"},
  {.key = B_S|B_J        , .kana = "ra"},
  {.key = B_S|B_K        , .kana = ""},
  {.key = B_S|B_L        , .kana = ""},
  {.key = B_S|B_SCLN     , .kana = "so"},

  {.key = B_L|B_Z        , .kana = "ze"},
  {.key = B_L|B_X        , .kana = "za"},
  {.key = B_L|B_C        , .kana = "gi"},
  {.key = B_L|B_V        , .kana = "ro"},
  {.key = B_L|B_B        , .kana = "nu"},
  {.key = B_S|B_N        , .kana = "wa"},
  {.key = B_S|B_M        , .kana = "da"},
  {.key = B_S|B_COMM     , .kana = "pi"},
  {.key = B_S|B_DOT      , .kana = "po"},
  {.key = B_S|B_SLSH     , .kana = "tile"},

  {.key = B_L|B_1        , .kana = "xya"},
  {.key = B_L|B_2        , .kana = "mya"},
  {.key = B_L|B_3        , .kana = "myu"},
  {.key = B_L|B_4        , .kana = "myo"},
  {.key = B_L|B_5        , .kana = "xwa"},

  {.key = B_I|B_E     , .kana = "sho"},
  {.key = B_I|B_W     , .kana = "shu"},
  {.key = B_I|B_R     , .kana = "kyu"},
  {.key = B_I|B_F     , .kana = "kyo"},
  {.key = B_I|B_V     , .kana = "kya"},
  {.key = B_I|B_C     , .kana = "sha"},
  {.key = B_I|B_Q     , .kana = "hyu"},
  {.key = B_I|B_A     , .kana = "hyo"},
  {.key = B_I|B_Z     , .kana = "hya"},
  {.key = B_I|B_T     , .kana = "chu"},
  {.key = B_I|B_G     , .kana = "cho"},
  {.key = B_I|B_B     , .kana = "cha"},

  {.key = B_I|B_1     , .kana = "xyu"},
  {.key = B_I|B_2     , .kana = "bya"},
  {.key = B_I|B_3     , .kana = "byu"},
  {.key = B_I|B_4     , .kana = "byo"},

  {.key = B_O|B_E     , .kana = "jo"},
  {.key = B_O|B_W     , .kana = "ju"},
  {.key = B_O|B_R     , .kana = "gyu"},
  {.key = B_O|B_F     , .kana = "gyo"},
  {.key = B_O|B_V     , .kana = "gya"},
  {.key = B_O|B_C     , .kana = "ja"},
  {.key = B_O|B_Q     , .kana = "ryu"},
  {.key = B_O|B_A     , .kana = "ryo"},
  {.key = B_O|B_Z     , .kana = "rya"},
  {.key = B_O|B_T     , .kana = "nyu"},
  {.key = B_O|B_G     , .kana = "nyo"},
  {.key = B_O|B_B     , .kana = "nya"},

  {.key = B_O|B_1     , .kana = "xyo"},
  {.key = B_O|B_2     , .kana = "pya"},
  {.key = B_O|B_3     , .kana = "pyu"},
  {.key = B_O|B_4     , .kana = "pyo"},

  // others
  // [RF]・[RG]・　[HU]／
  // [FG]「」　　　[HJ]（）
  // [FV]！[FB]！　[NJ]？

  {.key = B_R|B_F     , .kana = "/"},
  {.key = B_R|B_G     , .kana = "/"},
  {.key = B_F|B_G     , .kana = "()"SS_TAP(X_LEFT) }, // （）←
  {.key = B_F|B_V     , .kana = "!"},
  {.key = B_F|B_B     , .kana = "!"},
  {.key = B_N|B_J     , .kana = "?"},
  {.key = B_H|B_J     , .kana = "[]"SS_TAP(X_LEFT) }, // 「」←
  {.key = B_H|B_U     , .kana = "/"},
};

const PROGMEM naginata_keymap ngmap2[] = { // 薙刀式
  // 単独
  {.key = B_1               , .kana = "1"},
  {.key = B_2               , .kana = "2"},
  {.key = B_3               , .kana = "3"},
  {.key = B_4               , .kana = "4"},
  {.key = B_5               , .kana = "5"},
  {.key = B_6               , .kana = "6"},
  {.key = B_7               , .kana = "7"},
  {.key = B_8               , .kana = "8"},
  {.key = B_9               , .kana = "9"},
  {.key = B_0               , .kana = "0"},
  {.key = B_MINS            , .kana = "-"},
  {.key = B_Q               , .kana = "vu"},

  // 清音
  {.key = B_J                      , .kana = "a"       }, // あ
  {.key = B_K                      , .kana = "i"       }, // い
  {.key = B_L                      , .kana = "u"       }, // う
  {.key = B_SHFT|B_O               , .kana = "e"       }, // え
  {.key = B_SHFT|B_N               , .kana = "o"       }, // お
  {.key = B_F                      , .kana = "ka"      }, // か
  {.key = B_W                      , .kana = "ki"      }, // き
  {.key = B_H                      , .kana = "ku"      }, // く
  {.key = B_S                      , .kana = "ke"      }, // け
  {.key = B_V                      , .kana = "ko"      }, // こ
  {.key = B_SHFT|B_U               , .kana = "sa"      }, // さ
  {.key = B_R                      , .kana = "si"      }, // し
  {.key = B_O                      , .kana = "su"      }, // す
  {.key = B_SHFT|B_A               , .kana = "se"      }, // せ
  {.key = B_B                      , .kana = "so"      }, // そ
  {.key = B_N                      , .kana = "ta"      }, // た
  {.key = B_SHFT|B_G               , .kana = "ti"      }, // ち
  {.key = B_SHFT|B_SCLN            , .kana = "fu"      }, // ふ
  {.key = B_E                      , .kana = "te"      }, // て
  {.key = B_D                      , .kana = "to"      }, // と
  {.key = B_M                      , .kana = "na"      }, // な
  {.key = B_SHFT|B_D               , .kana = "ni"      }, // に
  {.key = B_SHFT|B_R               , .kana = "ne"      }, // ね
  {.key = B_SHFT|B_COMM            , .kana = "mu"      }, // む
  {.key = B_SHFT|B_J               , .kana = "no"      }, // の
  {.key = B_C                      , .kana = "ha"      }, // は
  {.key = B_X                      , .kana = "hi"      }, // ひ
  {.key = B_SHFT|B_X               , .kana = "hi"      }, // ひ
  {.key = B_SHFT|B_DOT             , .kana = "wa"      }, // わ
  {.key = B_P                      , .kana = "he"      }, // へ
  {.key = B_Z                      , .kana = "ho"      }, // ほ
  {.key = B_SHFT|B_Z               , .kana = "ho"      }, // ほ
  {.key = B_SHFT|B_F               , .kana = "ma"      }, // ま
  {.key = B_SHFT|B_B               , .kana = "mi"      }, // み
  {.key = B_SHFT|B_W               , .kana = "nu"      }, // ぬ
  {.key = B_SHFT|B_S               , .kana = "me"      }, // め
  {.key = B_SHFT|B_K               , .kana = "mo"      }, // も
  {.key = B_SHFT|B_H               , .kana = "ya"      }, // や
  {.key = B_SHFT|B_P               , .kana = "yu"      }, // ゆ
  {.key = B_SHFT|B_I               , .kana = "yo"      }, // よ
  {.key = B_DOT                    , .kana = "ra"      }, // ら
  {.key = B_SHFT|B_E               , .kana = "ri"      }, // り
  {.key = B_I                      , .kana = "ru"      }, // る
  {.key = B_SLSH                   , .kana = "re"      }, // れ
  {.key = B_SHFT|B_SLSH            , .kana = "re"      }, // れ
  {.key = B_A                      , .kana = "ro"      }, // ろ
  {.key = B_SHFT|B_L               , .kana = "tsu"      }, // つ
  {.key = B_SHFT|B_C               , .kana = "wo"      }, // を
  {.key = B_COMM                   , .kana = "nn"      }, // ん
  {.key = B_SCLN                   , .kana = "-"       }, // ー

  // 濁音
  {.key = B_J|B_F                  , .kana = "ga"      }, // が
  {.key = B_J|B_W                  , .kana = "gi"      }, // ぎ
  {.key = B_F|B_H                  , .kana = "gu"      }, // ぐ
  {.key = B_J|B_S                  , .kana = "ge"      }, // げ
  {.key = B_J|B_V                  , .kana = "go"      }, // ご
  {.key = B_F|B_U                  , .kana = "za"      }, // ざ
  {.key = B_J|B_R                  , .kana = "zi"      }, // じ
  {.key = B_F|B_O                  , .kana = "zu"      }, // ず
  {.key = B_J|B_A                  , .kana = "ze"      }, // ぜ
  {.key = B_J|B_B                  , .kana = "zo"      }, // ぞ
  {.key = B_F|B_N                  , .kana = "da"      }, // だ
  {.key = B_J|B_G                  , .kana = "di"      }, // ぢ
  {.key = B_F|B_SCLN               , .kana = "bu"      }, // ぶ
  {.key = B_J|B_E                  , .kana = "de"      }, // で
  {.key = B_J|B_D                  , .kana = "do"      }, // ど
  {.key = B_J|B_C                  , .kana = "ba"      }, // ば
  {.key = B_J|B_X                  , .kana = "bi"      }, // び
  {.key = B_F|B_P                  , .kana = "be"      }, // べ
  {.key = B_J|B_Z                  , .kana = "bo"      }, // ぼ
  {.key = B_F|B_L                  , .kana = "du"      }, // づ

  // 半濁音
  {.key = B_M|B_C                  , .kana = "pa"      }, // ぱ
  {.key = B_M|B_X                  , .kana = "pi"      }, // ぴ
  {.key = B_V|B_SCLN                , .kana = "pu"      }, // ぷ
  {.key = B_V|B_P                  , .kana = "pe"      }, // ぺ
  {.key = B_M|B_Z                  , .kana = "po"      }, // ぽ

  // 小書き
  {.key = B_Q|B_H                  , .kana = "xya"     }, // ゃ
  {.key = B_Q|B_P                  , .kana = "xyu"     }, // ゅ
  {.key = B_Q|B_I                  , .kana = "xyo"     }, // ょ
  {.key = B_Q|B_J                  , .kana = "xa"      }, // ぁ
  {.key = B_Q|B_K                  , .kana = "xi"      }, // ぃ
  {.key = B_Q|B_L                  , .kana = "xu"      }, // ぅ
  {.key = B_Q|B_O                  , .kana = "xe"      }, // ぇ
  {.key = B_Q|B_N                  , .kana = "xo"      }, // ぉ
  {.key = B_Q|B_SHFT|B_L           , .kana = "xwa"     }, // ゎ
  {.key = B_G                      , .kana = "xtu"     }, // っ

  // 清音拗音 濁音拗音 半濁拗音
  {.key = B_R|B_H                  , .kana = "sya"     }, // しゃ
  {.key = B_R|B_P                  , .kana = "syu"     }, // しゅ
  {.key = B_R|B_I                  , .kana = "syo"     }, // しょ
  {.key = B_J|B_R|B_H              , .kana = "zya"     }, // じゃ
  {.key = B_J|B_R|B_P              , .kana = "zyu"     }, // じゅ
  {.key = B_J|B_R|B_I              , .kana = "zyo"     }, // じょ
  {.key = B_W|B_H                  , .kana = "kya"     }, // きゃ
  {.key = B_W|B_P                  , .kana = "kyu"     }, // きゅ
  {.key = B_W|B_I                  , .kana = "kyo"     }, // きょ
  {.key = B_J|B_W|B_H              , .kana = "gya"     }, // ぎゃ
  {.key = B_J|B_W|B_P              , .kana = "gyu"     }, // ぎゅ
  {.key = B_J|B_W|B_I              , .kana = "gyo"     }, // ぎょ
  {.key = B_G|B_H                  , .kana = "tya"     }, // ちゃ
  {.key = B_G|B_P                  , .kana = "tyu"     }, // ちゅ
  {.key = B_G|B_I                  , .kana = "tyo"     }, // ちょ
  {.key = B_J|B_G|B_H              , .kana = "dya"     }, // ぢゃ
  {.key = B_J|B_G|B_P              , .kana = "dyu"     }, // ぢゅ
  {.key = B_J|B_G|B_I              , .kana = "dyo"     }, // ぢょ
  {.key = B_D|B_H                  , .kana = "nya"     }, // にゃ
  {.key = B_D|B_P                  , .kana = "nyu"     }, // にゅ
  {.key = B_D|B_I                  , .kana = "nyo"     }, // にょ
  {.key = B_X|B_H                  , .kana = "hya"     }, // ひゃ
  {.key = B_X|B_P                  , .kana = "hyu"     }, // ひゅ
  {.key = B_X|B_I                  , .kana = "hyo"     }, // ひょ
  {.key = B_J|B_X|B_H              , .kana = "bya"     }, // びゃ
  {.key = B_J|B_X|B_P              , .kana = "byu"     }, // びゅ
  {.key = B_J|B_X|B_I              , .kana = "byo"     }, // びょ
  {.key = B_M|B_X|B_H              , .kana = "pya"     }, // ぴゃ
  {.key = B_M|B_X|B_P              , .kana = "pyu"     }, // ぴゅ
  {.key = B_M|B_X|B_I              , .kana = "pyo"     }, // ぴょ
  {.key = B_B|B_H                  , .kana = "mya"     }, // みゃ
  {.key = B_B|B_P                  , .kana = "myu"     }, // みゅ
  {.key = B_B|B_I                  , .kana = "myo"     }, // みょ
  {.key = B_E|B_H                  , .kana = "rya"     }, // りゃ
  {.key = B_E|B_P                  , .kana = "ryu"     }, // りゅ
  {.key = B_E|B_I                  , .kana = "ryo"     }, // りょ

  // 清音外来音 濁音外来音
  {.key = B_M|B_E|B_K              , .kana = "thi"     }, // てぃ
  {.key = B_M|B_E|B_P              , .kana = "thu"     }, // てゅ
  {.key = B_J|B_E|B_K              , .kana = "dhi"     }, // でぃ
  {.key = B_J|B_E|B_P              , .kana = "dhu"     }, // でゅ
  {.key = B_M|B_D|B_L              , .kana = "toxu"    }, // とぅ
  {.key = B_J|B_D|B_L              , .kana = "doxu"    }, // どぅ
  {.key = B_M|B_R|B_O              , .kana = "sye"     }, // しぇ
  {.key = B_M|B_G|B_O              , .kana = "tye"     }, // ちぇ
  {.key = B_J|B_R|B_O              , .kana = "zye"     }, // じぇ
  {.key = B_J|B_G|B_O              , .kana = "dye"     }, // ぢぇ
  {.key = B_V|B_SCLN|B_J            , .kana = "fa"      }, // ふぁ
  {.key = B_V|B_SCLN|B_K            , .kana = "fi"      }, // ふぃ
  {.key = B_V|B_SCLN|B_O            , .kana = "fe"      }, // ふぇ
  {.key = B_V|B_SCLN|B_N            , .kana = "fo"      }, // ふぉ
  {.key = B_V|B_SCLN|B_P            , .kana = "fyu"     }, // ふゅ
  {.key = B_V|B_K|B_O              , .kana = "ixe"     }, // いぇ
  {.key = B_V|B_L|B_K              , .kana = "wi"      }, // うぃ
  {.key = B_V|B_L|B_O              , .kana = "we"      }, // うぇ
  {.key = B_V|B_L|B_N              , .kana = "uxo"     }, // うぉ
  {.key = B_V|B_H|B_J              , .kana = "kuxa"    }, // くぁ
  {.key = B_V|B_H|B_K              , .kana = "kuxi"    }, // くぃ
  {.key = B_V|B_H|B_O              , .kana = "kuxe"    }, // くぇ
  {.key = B_V|B_H|B_N              , .kana = "kuxo"    }, // くぉ
  {.key = B_V|B_H|B_L              , .kana = "kuxwa"   }, // くゎ
  {.key = B_F|B_H|B_J              , .kana = "guxa"    }, // ぐぁ
  {.key = B_F|B_H|B_K              , .kana = "guxi"    }, // ぐぃ
  {.key = B_F|B_H|B_O              , .kana = "guxe"    }, // ぐぇ
  {.key = B_F|B_H|B_N              , .kana = "guxo"    }, // ぐぉ
  {.key = B_F|B_H|B_L              , .kana = "guxwa"   }, // ぐゎ
  {.key = B_V|B_L|B_J           , .kana = "tsa"     }, // つぁ
  {.key = B_V|B_L|B_K           , .kana = "tsi"     }, // つぃ
  {.key = B_V|B_L|B_O           , .kana = "tse"     }, // つぇ
  {.key = B_V|B_L|B_N           , .kana = "tso"     }, // つぉ

  // 追加
  {.key = B_SHFT            , .kana = " "},
  {.key = B_Q               , .kana = ""},
  {.key = B_V|B_SHFT        , .kana = ","},
  {.key = B_M|B_SHFT        , .kana = "."SS_TAP(X_ENTER)},
  {.key = B_U               , .kana = SS_TAP(X_BSPC)},

  // enter
  {.key = B_V|B_M           , .kana = SS_TAP(X_ENTER)},
  // enter+シフト(連続シフト)
  {.key = B_SHFT|B_V|B_M    , .kana = SS_TAP(X_ENTER)},
  // left
  {.key = B_T               , .kana = SS_TAP(X_LEFT)},
  // right
  {.key = B_Y               , .kana = SS_TAP(X_RIGHT)},

  {.key = B_J|B_K|B_T       , .kana = "/"},  // ・
  {.key = B_J|B_K|B_F       , .kana = "[]"}, // 「」
  {.key = B_J|B_K|B_D       , .kana = "?"},  // ？
  {.key = B_J|B_K|B_C       , .kana = "!"},  // ！
  {.key = B_J|B_K|B_S       , .kana = "()"},  //（）
  {.key = B_J|B_K|B_X       , .kana = "{}"},  //｛｝
  {.key = B_J|B_K|B_A       , .kana = "///"},  // ・

  // 非標準の変換
  {.key = B_V|B_DOT|B_COMM  , .kana = "fe"      }, // ふぇ
  {.key = B_X|B_C|B_M       , .kana = "pyu"     }, // ピュ
};

 // 新下駄
// const PROGMEM naginata_keymap_long ngmapl[] = {
//   {.key = B_SHFT|B_T        , .kana = SS_DOWN(X_LSHIFT)SS_TAP(X_DOWN)SS_UP(X_LSHIFT)},
//   {.key = B_SHFT|B_Y        , .kana = SS_DOWN(X_LSHIFT)SS_TAP(X_UP)SS_UP(X_LSHIFT)},

//   // 編集モード1
//   {.key = B_D|B_F|B_P       , .kana = SS_TAP(X_ESCAPE)SS_TAP(X_ESCAPE)SS_TAP(X_ESCAPE)},

//   {.key = B_D|B_F|B_K       , .kana = SS_DOWN(X_LSHIFT)SS_TAP(X_LEFT)SS_UP(X_LSHIFT)},
//   {.key = B_D|B_F|B_L       , .kana = SS_TAP(X_LEFT)SS_TAP(X_LEFT)SS_TAP(X_LEFT)SS_TAP(X_LEFT)SS_TAP(X_LEFT)},

//   {.key = B_D|B_F|B_COMM    , .kana = SS_DOWN(X_LSHIFT)SS_TAP(X_RIGHT)SS_UP(X_LSHIFT)},
//   {.key = B_D|B_F|B_DOT     , .kana = SS_TAP(X_RIGHT)SS_TAP(X_RIGHT)SS_TAP(X_RIGHT)SS_TAP(X_RIGHT)SS_TAP(X_RIGHT)},

// #ifdef NAGINATA_EDIT_WIN
//   {.key = B_J|B_K|B_Q       , .kana = SS_DOWN(X_LCTRL)SS_TAP(X_END)SS_UP(X_LCTRL)},
//   {.key = B_J|B_K|B_W       , .kana = SS_DOWN(X_LCTRL)SS_TAP(X_HOME)SS_UP(X_LCTRL)},
//   {.key = B_D|B_F|B_U       , .kana = SS_DOWN(X_LSHIFT)SS_TAP(X_END)SS_UP(X_LSHIFT)SS_TAP(X_BSPACE)},
//   {.key = B_D|B_F|B_H       , .kana = SS_TAP(X_ENTER)SS_TAP(X_END)},
//   {.key = B_D|B_F|B_Y       , .kana = SS_TAP(X_HOME)},
//   {.key = B_D|B_F|B_N       , .kana = SS_TAP(X_END)},
// #endif
// #ifdef NAGINATA_EDIT_MAC
//   {.key = B_J|B_K|B_Q       , .kana = SS_DOWN(X_LGUI)SS_TAP(X_DOWN)SS_UP(X_LGUI)},
//   {.key = B_J|B_K|B_W       , .kana = SS_DOWN(X_LGUI)SS_TAP(X_UP)SS_UP(X_LGUI)},
//   {.key = B_D|B_F|B_U       , .kana = SS_DOWN(X_LSHIFT)SS_DOWN(X_LGUI)SS_TAP(X_RIGHT)SS_UP(X_LGUI)SS_UP(X_LSHIFT)SS_LGUI("x")},
//   {.key = B_D|B_F|B_H       , .kana = SS_TAP(X_ENTER)SS_DOWN(X_LGUI)SS_TAP(X_RIGHT)SS_UP(X_LGUI)},
//   {.key = B_D|B_F|B_Y       , .kana = SS_DOWN(X_LGUI)SS_TAP(X_LEFT)SS_UP(X_LGUI)},
//   {.key = B_D|B_F|B_N       , .kana = SS_DOWN(X_LGUI)SS_TAP(X_RIGHT)SS_UP(X_LGUI)},
// #endif

//   // 編集モード2
// #ifdef NAGINATA_EDIT_WIN
//   {.key = B_M|B_COMM|B_T    , .kana = SS_TAP(X_HOME)" "SS_TAP(X_END)},
//   {.key = B_M|B_COMM|B_G    , .kana = SS_TAP(X_HOME)"   "SS_TAP(X_END)},
//   {.key = B_C|B_V|B_U       , .kana = SS_DOWN(X_LSHIFT)SS_TAP(X_HOME)SS_UP(X_LSHIFT)SS_LCTRL("x")},
//   {.key = B_C|B_V|B_I       , .kana = SS_DOWN(X_LCTRL)SS_TAP(X_BSPACE)SS_UP(X_LCTRL)},
// #endif
// #ifdef NAGINATA_EDIT_MAC
//   {.key = B_M|B_COMM|B_T    , .kana = SS_DOWN(X_LGUI)SS_TAP(X_LEFT)SS_UP(X_LGUI)" "SS_DOWN(X_LGUI)SS_TAP(X_RIGHT)SS_UP(X_LGUI)},
//   {.key = B_M|B_COMM|B_G    , .kana = SS_DOWN(X_LGUI)SS_TAP(X_LEFT)SS_UP(X_LGUI)"   "SS_DOWN(X_LGUI)SS_TAP(X_RIGHT)SS_UP(X_LGUI)},
//   {.key = B_C|B_V|B_U       , .kana = SS_DOWN(X_LSHIFT)SS_DOWN(X_LGUI)SS_TAP(X_LEFT)SS_UP(X_LGUI)SS_UP(X_LSHIFT)SS_LGUI("x")},
//   {.key = B_C|B_V|B_I       , .kana = ""},
// #endif

// };

 // 新下駄
// const PROGMEM naginata_keymap_unicode ngmapu[] = {
  // 編集モード2 // 新下駄
// #ifdef NAGINATA_EDIT_WIN
//   {.key = B_SHFT|B_W    , .kana = "309C"},
//   {.key = B_L           , .kana = "309B"},
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
    // case B_F|B_G:
    //   naginata_off();
    //   break;
    // case B_J|B_K|B_T:
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
//     case B_J|B_K|B_G:
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
//     case B_J|B_K|B_B:
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
//     case B_C|B_V|B_P:
//       send_unicode_hex_string("FF5C");
//       tap_code(KC_ENT);
//       tap_code(KC_END);
//       send_unicode_hex_string("300A 300B");
//       tap_code(KC_ENT);
//       tap_code(KC_LEFT);
//       break;
//     case B_C|B_V|B_Y:
//       send_unicode_hex_string("300D");
//       tap_code(KC_ENT);
//       tap_code(KC_ENT);
//       tap_code(KC_SPC);
//       break;
//     case B_C|B_V|B_H:
//       send_unicode_hex_string("300D");
//       tap_code(KC_ENT);
//       tap_code(KC_ENT);
//       send_unicode_hex_string("300C");
//       tap_code(KC_ENT);
//       break;
//     case B_C|B_V|B_N:
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
            // if ((keycomb & B_SHFT) > 0) skey |= B_SHFT; // シフトキー状態を反映 // 新下駄
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
            // if ((keycomb & B_SHFT) > 0) skey |= B_SHFT; // シフトキー状態を反映 // 新下駄
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
      case NG_Q ... NG_SHFT: // 新下駄
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
      case NG_Q ... NG_SHFT: // 新下駄
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
//     KC_ESC ,KC_Q   ,KC_W   ,KC_E   ,KC_R   ,KC_T   ,                KC_Y   ,KC_U   ,KC_I   ,KC_O   ,KC_P   ,KC_BSPC, \
//     KC_TAB ,KC_A   ,KC_S   ,KC_D   ,KC_F   ,KC_G   ,                KC_H   ,KC_J   ,KC_K   ,KC_L   ,JP_SCLN,KC_RGUI, \
//     KC_LSFT,KC_Z   ,KC_X   ,KC_C   ,KC_V   ,KC_B   ,                KC_N   ,KC_M   ,JP_COMM,JP_DOT ,JP_SLSH,KC_RCMD, \
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
//     KC_ESC ,JP_TILD,JP_AT  ,JP_HASH,JP_DLR ,JP_PERC,                JP_SLSH,KC_7   ,KC_8   ,KC_9   ,JP_MINS,KC_BSPC, \
//     KC_TAB ,JP_CIRC,JP_AMPR,JP_EXLM,JP_QUES,JP_BSLS,                JP_ASTR,KC_4   ,KC_5   ,KC_6   ,JP_PLUS,JP_DOT , \
//     KC_LSFT,JP_PIPE,JP_GRV ,JP_QUOT,JP_DQUO ,JP_UNDS,                KC_0   ,KC_1   ,KC_2   ,KC_3   ,JP_EQL ,JP_COMM, \
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
//     KC_ESC    ,XXXXXXX   ,XXXXXXX   ,XXXXXXX   ,XXXXXXX   ,XXXXXXX   ,                      XXXXXXX   ,XXXXXXX   ,KC_UP     ,XXXXXXX   ,KC_PGUP   ,XXXXXXX   , \
//     KC_TAB    ,XXXXXXX   ,JP_LBRC   ,JP_LCBR   ,JP_LPRN   ,KC_LT     ,                      KC_HOME   ,KC_LEFT   ,KC_DOWN   ,KC_RGHT   ,KC_PGDN   ,XXXXXXX   , \
//     KC_LSFT   ,XXXXXXX   ,JP_RBRC   ,JP_RCBR   ,JP_RPRN   ,KC_GT     ,                      KC_END    ,S(KC_LEFT),S(KC_DOWN),S(KC_RGHT),XXXXXXX   ,XXXXXXX   , \
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
//     RGB_HUI,RGB_HUD,KC_WAKE,XXXXXXX,RESET  ,UNDGL  ,                RGB_TOG,XXXXXXX,KC_VOLU,XXXXXXX,KC_BRIU,QWERTY , \
//     RGB_SAI,RGB_SAD,XXXXXXX,XXXXXXX,XXXXXXX,XXXXXXX,                RGB_MOD,KC_MRWD,KC_VOLD,KC_MFFD,KC_BRID,EUCALYN, \
//     RGB_VAI,RGB_VAD,KC_SLEP,XXXXXXX,XXXXXXX,XXXXXXX,                RGBRST ,XXXXXXX,KC_MPLY,XXXXXXX,XXXXXXX,WORKMAN, \
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
//     KC_ESC ,KC_Q   ,KC_W   ,JP_COMM,JP_DOT ,JP_SCLN,                KC_M   ,KC_R   ,KC_D   ,KC_Y   ,KC_P   ,KC_BSPC, \
//     KC_TAB ,KC_A   ,KC_O   ,KC_E   ,KC_I   ,KC_U   ,                KC_G   ,KC_T   ,KC_K   ,KC_S   ,KC_N   ,KC_RGUI, \
//     KC_LSFT,KC_Z   ,KC_X   ,KC_C   ,KC_V   ,KC_F   ,                KC_B   ,KC_H   ,KC_J   ,KC_L   ,JP_SLSH,KC_RCMD, \
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
//     KC_ESC ,KC_Q   ,KC_D   ,KC_R   ,KC_W   ,KC_B   ,                KC_J   ,KC_F   ,KC_U   ,KC_P   ,JP_SCLN,KC_BSPC, \
//     KC_TAB ,KC_A   ,KC_S   ,KC_H   ,KC_T   ,KC_G   ,                KC_Y   ,KC_N   ,KC_E   ,KC_O   ,KC_I   ,KC_RGUI, \
//     KC_LSFT,KC_Z   ,KC_X   ,KC_M   ,KC_C   ,KC_V   ,                KC_K   ,KC_L   ,JP_COMM,JP_DOT ,JP_SLSH,KC_RCMD, \
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
//     _______,NG_Q   ,NG_W   ,NG_E   ,NG_R   ,NG_T   ,                NG_Y   ,NG_U   ,NG_I   ,NG_O   ,NG_P   ,NG_X1  , \
//     _______,NG_A   ,NG_S   ,NG_D   ,NG_F   ,NG_G   ,                NG_H   ,NG_J   ,NG_K   ,NG_L   ,NG_SCLN,KC_BSPC, \
//     _______,NG_Z   ,NG_X   ,NG_C   ,NG_V   ,NG_B   ,                NG_N   ,NG_M   ,NG_COMM,NG_DOT ,NG_SLSH,KC_BSLS, \
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
        _______, NG_1,     NG_2,     NG_3,     NG_4,     NG_5,     NG_6,     NG_7,     NG_8,     NG_9,     NG_0,     NG_MINS,  KC_EQL,   KC_INT3,  KC_BSPC,  KC_PGUP,
        KC_TAB,  NG_Q,     NG_W,     NG_E,     NG_R,     NG_T,     NG_Y,     NG_U,     NG_I,     NG_O,     NG_P,     NG_X1,    KC_RBRC,                      KC_PGDN,
        KC_LCTL, NG_A,     NG_S,     NG_D,     NG_F,     NG_G,     NG_H,     NG_J,     NG_K,     NG_L,     NG_SCLN,  KC_BSPC,  _______,  _______,            _______,
        KC_LSFT,           NG_Z,     NG_X,     NG_C,     NG_V,     NG_B,     NG_N,     NG_M,     NG_COMM,  NG_DOT,   NG_SLSH,  _______,  _______,            _______,
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
