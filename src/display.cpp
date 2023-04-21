#include "display.hpp"

#define NUM_LEDS 280
#define NUM_SEGS (NUM_LEDS / 10)
#define DATA_PIN 0

CRGB leds[NUM_LEDS];

int segs_to_num[] = { 119, 68, 62, 110, 77, 107, 123, 70, 127, 111 };

void init_display(void) {
    pinMode(DATA_PIN, OUTPUT);

    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(255);
    FastLED.clear(true);

    delay(1000);

    fill_solid(leds, NUM_LEDS, CRGB::Black);
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB::Red;
        FastLED.show();
        delay(100);
    }

    FastLED.clear(true);
}

void display_loading(CRGB color) {
	for (int i = 0; i < 10; i++) {
		if (leds[i] == (CRGB) CRGB::Black) {
    		leds[i] = color;
	  		FastLED.show();
			return;
    	}
  	}

	FastLED.clear(true);
}

void display(display_stat stat) {
    switch (stat.mode) {
        case CLOCK:
            display_clock();
            break;
        case TIMER:
            display_timer(stat.timer_start, stat.timer_top, stat.timer_transition);
            break;
        case NUMBERS:
            display_numbers(stat.numbers, stat.num_colors);
            break;
        case SEGMENTS:
            display_segments(stat.seg_colors);
            break;
		case ERROR:
			display_error(stat.error, stat.error_code);
			break;
    }
}

void display_clock(void) {
    time_t now = time(nullptr);
    struct tm *t = localtime(&now);
    write_two_doubles(t->tm_hour, t->tm_min, CRGB::Green);
	FastLED.show();
}

void display_timer(time_t start, time_t top, time_t transition) {
    int set = top + transition;
	int elapsed = time(nullptr) - start;

    CRGB color = CRGB::Red;
	int rem_sec = set - (elapsed % set);
    bool in_transition = rem_sec <= transition;
    
    if (elapsed < 0) {
        color = CRGB::Yellow;
        rem_sec = -elapsed;
    } else if (!in_transition) {
        rem_sec -= transition;
    } else {
        color = CRGB::Green;
    }

    int h = rem_sec / 3600;
    int m = (rem_sec % 3600) / 60;
    int s = rem_sec % 60;

    if (h > 0)
        write_two_doubles(h, m, color);
    else
        write_two_doubles(m, s, color);
    
    FastLED.show();
}

void display_numbers(char nums[4], CRGB colors[4]) {
    for (int i = 0; i < 4; i++) {
        write_digit(i, nums[i], colors[i]);
    }

    FastLED.show();
}

void display_segments(CRGB colors[28]) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 7; j++) {
            write_segment(i, j, colors[i * 7 + j]);
        }
    }

    FastLED.show();
}

// TODO test
void display_error(error_type error, int error_code) {
    if (error == NO_INTERNET) {
	    write_segments(0, 0b1011000, CRGB::Yellow);
        write_segments(1, 0b1111000, CRGB::Yellow);
        write_segments(2, 0b1011000, CRGB::Yellow);
        write_segments(3, 0b0111001, CRGB::Yellow);
    } else if (error == WIFI_MODULE_ERROR) {
        write_segments(0, 0b0111011, CRGB::Yellow);
        write_segments(1, 0b0011000, CRGB::Yellow);
        write_segments(2, 0b0011000, CRGB::Yellow);
        write_digit(3, error_code, CRGB::Yellow);
    } else {
        write_segments(0, 0b0111011, CRGB::Yellow);
        write_segments(1, 0b0011000, CRGB::Yellow);
        write_segments(2, 0b0011000, CRGB::Yellow);
        write_segments(3, 0b0000000, CRGB::Yellow);
    }

    FastLED.show();
}

void write_two_doubles(int left, int right, CRGB color) {
	write_segments(0, segs_to_num[(left / 10) % 10], color);
    write_segments(1, segs_to_num[left % 10], color);
    write_segments(2, segs_to_num[(right / 10) % 10], color);
    write_segments(3, segs_to_num[right % 10], color);
}

/*
  i starts at 0 (digit index)
  n must be one of 0 - 9
*/
void write_digit(int i, int n, CRGB color) {
    write_segments(i, segs_to_num[n], color);
}

/*
    i = digit index (0 - 3)
    segs = LSB -> 0th segment
*/
void write_segments(int i, int segs, CRGB color) {
    for (int j = 0; j < 7; j++) {
        if (segs & (1 << j))
            write_segment(i, j, color);
        else
            write_segment(i, j, CRGB::Black);
    }
}

void write_segment(int i, int seg, CRGB color) {
    int j = (i * 7 + seg) * 10;
    fill_solid(&leds[j], 10, color);
}