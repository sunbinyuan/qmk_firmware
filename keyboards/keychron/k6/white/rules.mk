# project specific files
SRC += rand.c
SRC += config_led.c

# MCU name
MCU = SN32F268F
EEPROM_DRIVER = transient

# Build Options
#   comment out to disable the options.
#
MAGIC_ENABLE = yes
MAGIC_KEYCODE_ENABLE = yes
BOOTMAGIC_ENABLE = yes 	# Virtual DIP switch configuration
EXTRAKEY_ENABLE = yes   	# Audio control and System control
NKRO_ENABLE = yes       	# USB Nkey Rollover
DIP_SWITCH_ENABLE = yes

# Custom Key and LED matrix handling
CUSTOM_MATRIX = yes
RGB_MATRIX_ENABLE = yes
RGB_MATRIX_DRIVER = SN32F26x

# process stack size of 0x1c0 crashes during SEND_STRING
USE_EXCEPTIONS_STACKSIZE = 0x180
USE_PROCESS_STACKSIZE = 0x210

# Reduce code size
USE_PROCESS_STACKSIZE = 0x1E0
USE_EXCEPTIONS_STACKSIZE = 0xF0