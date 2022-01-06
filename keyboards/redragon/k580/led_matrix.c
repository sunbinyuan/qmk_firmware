#include "rgb_matrix.h"

LED_TYPE led_state[LED_MATRIX_ROWS * LED_MATRIX_COLS];
uint8_t led_pos[DRIVER_LED_TOTAL + 2];
#ifdef UNDERGLOW_IDX
static const unsigned int underglow_led_idx[UNDERGLOW_LEDS] = UNDERGLOW_IDX;
#endif


void init(void) {
    unsigned int i = 0;
    for (unsigned int y = 0; y < LED_MATRIX_ROWS; y++) {
        for (unsigned int x = 0; x < LED_MATRIX_COLS; x++) {
            if (g_led_config.matrix_co[y][x] != NO_LED) {
                led_pos[g_led_config.matrix_co[y][x]] = i;
            }
            i++;
        }
    }
    led_pos[DRIVER_LED_TOTAL] = 77;
    led_pos[DRIVER_LED_TOTAL + 1] = 78;
#ifdef UNDERGLOW_IDX
    i = 0;
    for (unsigned int x = DRIVER_LED_TOTAL - UNDERGLOW_LEDS; x < DRIVER_LED_TOTAL; x++) {
        led_pos[x] = underglow_led_idx[i];
        i++;
    }
#endif
}

static void flush(void) {}

void set_color(int index, uint8_t r, uint8_t g, uint8_t b) {
    int corrected_index = led_pos[index];
    for (unsigned int i = 0; i < UNDERGLOW_LEDS; i++) {
        if (corrected_index == underglow_led_idx[i]) {
            led_state[corrected_index].r = r;
            led_state[corrected_index].g = b;
            led_state[corrected_index].b = g;
            return;
        }
    }
    led_state[corrected_index].r = r;
    led_state[corrected_index].g = g;
    led_state[corrected_index].b = b;
}

static void set_color_all(uint8_t r, uint8_t g, uint8_t b) {
    for (int i=0; i<DRIVER_LED_TOTAL; i++)
        set_color(i, r, g, b);
}

const rgb_matrix_driver_t rgb_matrix_driver = {
    .init          = init,
    .flush         = flush,
    .set_color     = set_color,
    .set_color_all = set_color_all,
};
