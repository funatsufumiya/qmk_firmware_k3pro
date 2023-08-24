# Keychron K3 Pro

※ K3 Pro (White) で新下駄配列＋薙刀式配列を打つために、KeychronのQMK Firmwareを用いて、[eswaiさんの新下駄配列・薙刀式配列](https://github.com/eswai/qmk_firmware/tree/master/keyboards/crkbd/keymaps)を移植しています。（keymap名は `naginata_v15_and_shingeta`）

Fn + LEDで新下駄・薙刀式の切り替え、Fn + スナップショットボタンでQwertyとAstarte+の切り替えができるようにしています。（ブログ記事: https://funatsufumiya.hatenablog.com/entry/2023/08/01/063522 ）

![Keychron K3 Pro](https://github.com/Keychron/ProductImage/blob/main/K_Pro/k3_pro.jpg?raw=true)

A customizable 84 keys TKL keyboard.

* Keyboard Maintainer: [Keychron](https://github.com/keychron)
* Hardware Supported: Keychron K3 Pro
* Hardware Availability: [Keychron K3 Pro QMK/VIA Wireless Custom Mechanical Keyboard](https://www.keychron.com/products/keychron-k3-pro-qmk-via-wireless-custom-mechanical-keyboard)

Make example for this keyboard (after setting up your build environment):

    make keychron/k3_pro/jis/white:naginata_v15_and_shingeta

Flashing example for this keyboard:

    make keychron/k3_pro/jis/white:naginata_v15_and_shingeta:flash

**Reset Key**: Connect the USB cable, toggle mode switch to "Off", hold down the *Esc* key or reset button underneath space bar, then toggle then switch to "Cable".

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).
