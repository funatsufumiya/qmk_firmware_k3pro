# Enter lower-power sleep mode when on the ChibiOS idle thread
OPT_DEFS += -DCORTEX_ENABLE_WFI_IDLE=TRUE
OPT_DEFS += -DNO_USB_STARTUP_CHECK -DENABLE_FACTORY_TEST

SRC += matrix.c

OPT_DEFS += -DKC_BLUETOOTH_ENABLE

BLUETOOTH_DIR = keyboards/keychron/bluetooth
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

VPATH += $(TOP_DIR)/$(BLUETOOTH_DIR)

