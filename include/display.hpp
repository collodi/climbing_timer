#define FASTLED_ESP8266_RAW_PIN_ORDER

#include <FastLED.h>

enum display_mode {
    CLOCK,
    TIMER,
    NUMBERS,
    SEGMENTS,
    ERROR,
};

enum error_type {
    NONE,
    NO_INTERNET,
    WIFI_MODULE_ERROR,
};

struct display_stat {
    display_mode mode = CLOCK;

    time_t timer_start = 0;
    time_t timer_top = 0;
    time_t timer_transition = 0;

    char numbers[4];
    CRGB num_colors[4];

    CRGB seg_colors[28];

    error_type error = NONE;
    int error_code = 0;
};

void init_display(void);

void display_loading(CRGB color);
void display(display_stat stat);
void display_clock(void);
void display_timer(time_t start_ms, time_t top, time_t transition);
void display_numbers(char nums[4], CRGB colors[4]);
void display_segments(CRGB colors[28]);
void display_error(error_type error, int error_code);

void write_two_doubles(int left, int right, CRGB color);
void write_digit(int i, int n, CRGB color);
void write_segments(int i, int segs, CRGB color);
void write_segment(int i, int seg, CRGB color);