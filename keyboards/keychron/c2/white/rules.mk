# project specific files
SRC += rand.c

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
RGB_MATRIX_CUSTOM_KB = yes

# Reduce code size
USE_PROCESS_STACKSIZE    = 0x1C0
USE_EXCEPTIONS_STACKSIZE = 0xF0
