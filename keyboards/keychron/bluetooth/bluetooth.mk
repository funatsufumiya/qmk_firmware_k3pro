
OPT_DEFS += -DKC_BLUETOOTH_ENABLE

BLUETOOTH_DIR = bluetooth
SRC += \
     $(BLUETOOTH_DIR)/bluetooth.c \
     $(BLUETOOTH_DIR)/report_buffer.c \
     $(BLUETOOTH_DIR)/ckbt51.c \
     $(BLUETOOTH_DIR)/indicator.c \
     $(BLUETOOTH_DIR)/bluetooth_main.c \
     $(BLUETOOTH_DIR)/transport.c \
     $(BLUETOOTH_DIR)/lpm.c \
     $(BLUETOOTH_DIR)/lpm_stm32l432.c \
     $(BLUETOOTH_DIR)/battery.c \
     $(BLUETOOTH_DIR)/factory_test.c \
     $(BLUETOOTH_DIR)/bat_level_animation.c \
     $(BLUETOOTH_DIR)/rtc_timer.c

VPATH += $(TOP_DIR)/keyboards/keychron/$(BLUETOOTH_DIR)

# Work around RTC clock issue without touching chibios, refer to the link for this bug
# https://forum.chibios.org/viewtopic.php?f=35&t=6197
OPT_DEFS += -DRCC_APBENR1_RTCAPBEN
