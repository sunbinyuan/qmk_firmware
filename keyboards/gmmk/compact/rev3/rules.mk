SRC  = ../config_led.c
SRC += ../led_matrix.c
SRC += backlight.c
SRC += SPI0.c

OPT_DEFS += -DSN32_SPI_CLKDIV=7 -DSN32_SPI_TXFIFO_THRESHOLD=7