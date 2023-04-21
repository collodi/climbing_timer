#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "display.hpp"
#include "credentials.h"

display_stat d_stat;

char data[100];
uint8_t data_size;

int connect_to_internet(void);
void config_clock(void);
void wait_for_wifi_module(void);

void setup() {
	Serial.begin(115200);
	Serial.setTimeout(200);

	delay(3000);

	init_display();
	WiFi.mode(WIFI_STA);
	
	while (true) {
		delay(2000);
		int conn = connect_to_internet();
		if (conn == 0) {
			config_clock();
			break;
		}

		if (conn == 2) {
			d_stat.mode = ERROR;
			d_stat.error = NO_INTERNET;
			break;
		}
	}

	FastLED.clear(true);
	WiFi.mode(WIFI_OFF);

	wait_for_wifi_module();
}

void loop() {
	display(d_stat);

	if (d_stat.mode == ERROR && d_stat.error == NO_INTERNET)
		return;

	memset(data, 0, 100);
	Serial.println("OK");
	
	data_size = Serial.readBytes(data, 100);
	if (data_size == 0)
		return;

	if(strncmp(data, "TIMER", 5) == 0) {
		d_stat.mode = TIMER;
		memcpy(&(d_stat.timer_start), &data[5], sizeof(time_t));
		memcpy(&(d_stat.timer_top), &data[13], sizeof(time_t));
		memcpy(&(d_stat.timer_transition), &data[21], sizeof(time_t));
	} else if(strncmp(data, "CLOCK", 5) == 0) {
		d_stat.mode = CLOCK;
		memcpy(&(d_stat.clock_offset), &data[5], 4);
	} else if(strncmp(data, "NMBRS", 5) == 0) {
		d_stat.mode = NUMBERS;

		for (int i = 0; i < 4; i++) {
			d_stat.numbers[i] = data[5 + i] - 0x30;
			memcpy(&(d_stat.num_colors[i]), &data[9 + 3 * i], 3);
		}
	} else if (strncmp(data, "SGMTS", 5) == 0) {
		d_stat.mode = SEGMENTS;

		for (int i = 0; i < 28; i++) {
			memcpy(&(d_stat.seg_colors[i]), &data[5 + 3 * i], 3);
		}
	} else if (strncmp(data, "ERROR", 5) == 0) {
		d_stat.mode = ERROR;
		d_stat.error = WIFI_MODULE_ERROR;
		d_stat.error_code = data[5];
	}
}

/*
	ret 0 = connected
	1 = error, retry
	2 = error, don't retry
*/
int connect_to_internet(void) {
	WiFi.begin(SSID, PWD);

	while (WiFi.status() != WL_CONNECTED) {
		delay(1000);
		wl_status_t stat = WiFi.status();
		display_loading(CRGB::Green);

		if (stat == 4) {
			return 1;
		}

		// TODO no ssid error
		// TODO connection lost error
		// TODO wrong password error
	}

	return 0;
}

void config_clock(void) {
	configTime(0, 0, "pool.ntp.org", "time.nist.gov");
	time_t now = time(nullptr);

	// wait for clock to be set up
	while (now < 1510592825) {
		delay(1000);
		display_loading(CRGB::Yellow);
		time(&now);
	}
	FastLED.clear(true);
}

void wait_for_wifi_module(void) {
	while (true) {
		delay(1000);
		display_loading(CRGB::Red);

		data_size = Serial.readBytes(data, 100);
		if (data_size == 0)
			continue;

		if (strncmp(data, "WIFI OK", 7) == 0)
			break;
	}
}