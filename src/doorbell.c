#include "doorbell.h"
#include "lpc24xx.h"
#include "lcd/lcd_hw.h"
#include "lcd/lcd_grph.h"
#include "lcd/lcd_cfg.h"
#include "lcd/sdram.h"
#include "delay.h"
#include "touch.h"
#include "songs.h"

extern struct tone song_data[];
extern int song_duration;
extern int get_doorbell_length(void);
volatile int playing = 0;

static int note_index = 0;
static int note_time_left = 0;    // time remaining for this note
static int half_period_counter = 0; // toggling counter

// Constants for doorbell behavior
#define OFF 100

// LCD UI constants
#define BTN_X 60
#define BTN_Y 180
#define BTN_W 160
#define BTN_H 40

// State to avoid redrawing
static int button_drawn = 0;
static int prev_button_state = 1;

// Initialize LCD
void init_lcd(void) {
    sdramInit();
    lcdInit(&lcd_config);
    lcdTurnOn();
}

// Check if touch input hits the button, and stop playback
static void check_stop_button_touch(char x, char y) {
    int point_x = x * 240 / 255;
    int point_y = y * 320 / 255;

    if (point_x >= BTN_X && point_x <= (BTN_X + BTN_W) &&
        point_y >= BTN_Y && point_y <= (BTN_Y + BTN_H)) {
        playing = 0;
    }
}

// One call to run doorbell behavior
void doorbell(void) {
    char x = 0, y = 0;

    // --- Toggle doorbell on physical button press ---
    int curr_button_state = (FIO0PIN & (1 << 10)) ? 1 : 0;
    if (prev_button_state == 1 && curr_button_state == 0) {
        playing = !playing;
    }
    prev_button_state = curr_button_state;

    // --- Check touchscreen ---
    touch_read_xy(&x, &y);
    if (touch_get_pressure() > 1) {
        check_stop_button_touch(x, y);
    }

    // --- LCD display update ---
    lcd_fontColor(WHITE, BLACK);
    if (playing) {
        lcd_fillRect(0, 100, 320, 130, BLACK);
        lcd_putString(60, 100, "Doorbell ringing");

        if (!button_drawn) {
            lcd_drawRect(BTN_X, BTN_Y, BTN_X + BTN_W, BTN_Y + BTN_H, WHITE);
            lcd_putString(BTN_X + 10, BTN_Y + 10, "Stop Doorbell");
            button_drawn = 1;
        }
    } else {
        lcd_fillRect(0, 100, 320, 130, BLACK);
        if (button_drawn) {
            lcd_fillRect(BTN_X, BTN_Y, BTN_X + BTN_W, BTN_Y + BTN_H, BLACK);
            button_drawn = 0;
        }
    }

    // --- Output waveform if playing ---
    if (playing) {
        struct tone current = song_data[note_index];

        if (current.pitch == OFF) {
            // silence
            DACR = 0;
        } else {
            // Generate square wave manually
            half_period_counter++;
            if (half_period_counter >= current.pitch / 100) {
                // scale: adjust division for your call rate
                static int high = 0;
                high = !high;
                DACR = high ? (current.volume << 6) : 0;
                half_period_counter = 0;
            }
        }

        // Decrement remaining time
        note_time_left--;
        if (note_time_left <= 0) {
            // Move to next note
            note_index++;
            if (note_index >= song_duration) {
                // Restart or stop
                note_index = 0;
                playing = 0; // stop after one play
            }
            note_time_left = song_data[note_index].duration * 100;
            // Scale 100 to adjust tempo
        }
	} else {
				DACR = 0;
				note_index = 0;
				note_time_left = 0;
				half_period_counter = 0;
	}
}
