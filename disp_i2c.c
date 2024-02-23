#include <stdio.h>
#include <string.h>
#include <math.h>

#include "disp_i2c.h"
#include "vendor/pico-ssd1306/ssd1306.h"

// All the logic is compatible only with ssd1306 128x64

#define DISP_WIDTH 128
#define DISP_HEIGHT 64
#define DISP_ADDR 0x3c

#define ONE_CHAR_WIDTH_PX (5 + 1) // 5 width + 1 spacing
#define ONE_CHAR_HEIGHT_PX 8

#define TOPBAR_TIME_NO_DATA "xx:xx"
#define TOPBAR_TIME_X 96
#define TOPBAR_TIME_Y 2
#define TOPBAR_TIME_WIDTH (sizeof(TOPBAR_TIME_NO_DATA) * ONE_CHAR_WIDTH_PX)

#define TOPBAR_GPS_NO_SIGNAL "no signal"
#define TOPBAR_GPS_HAS_SIGNAL "gps ok"
#define TOPBAR_GPS_X 2
#define TOPBAR_GPS_Y 2
#define TOPBAR_GPS_WIDTH (sizeof(TOPBAR_GPS_NO_SIGNAL) * ONE_CHAR_WIDTH_PX)

#define MAIN_LAT_NAME "lat:"
#define MAIN_LNG_NAME "lng:"
#define MAIN_COORDINATES_UNDEFINED "-xxx.xxxxx"
#define MAIN_COORD_NAME_X 2
#define MAIN_COORD_X (MAIN_COORD_NAME_X + (sizeof(MAIN_LAT_NAME) * ONE_CHAR_WIDTH_PX) + ONE_CHAR_WIDTH_PX)
#define MAIN_LAT_Y 16
#define MAIN_LNG_Y_PADDING 2
#define MAIN_LNG_Y (MAIN_LAT_Y + ONE_CHAR_HEIGHT_PX + MAIN_LNG_Y_PADDING)
#define MAIN_COORD_WIDTH sizeof(MAIN_COORDINATES_UNDEFINED) * ONE_CHAR_WIDTH_PX

static ssd1306_t display;

static void disp_i2c_show_topbar();
static void disp_i2c_show_main_screen();
static void disp_i2c_update_topbar_time_no_show(int hours, int minutes);
static void disp_i2c_update_topbar_gps_signal_no_show(bool has_signal);

void disp_i2c_init(i2c_inst_t *i2c, uint sda_pin, uint scl_pin, uint baudrate)
{
    i2c_init(i2c, baudrate);
    
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);

    gpio_pull_up(sda_pin);
    gpio_pull_up(scl_pin);

    display.external_vcc = false;
    ssd1306_init(&display, DISP_WIDTH, DISP_HEIGHT, DISP_ADDR, i2c);

    ssd1306_clear(&display);

    // TODO: init menu
    disp_i2c_show_topbar();
    disp_i2c_show_main_screen();
}

void disp_i2c_update_topbar(disp_topbar_data_t topbar_data)
{
    disp_i2c_update_topbar_time_no_show(topbar_data.time_hour, topbar_data.time_min);
    disp_i2c_update_topbar_gps_signal_no_show(topbar_data.has_signal);

    ssd1306_show(&display);
}

void disp_i2c_update_coords(float lat, float lng)
{
    char lat_str[sizeof(MAIN_COORDINATES_UNDEFINED)];
    char lng_str[sizeof(MAIN_COORDINATES_UNDEFINED)];

    // x == NAN is not correct as NAN always != NAN
    if (isnan(lat) || isnan(lng)) {
        strncpy(lat_str, MAIN_COORDINATES_UNDEFINED, sizeof(MAIN_COORDINATES_UNDEFINED));
        strncpy(lng_str, MAIN_COORDINATES_UNDEFINED, sizeof(MAIN_COORDINATES_UNDEFINED));
    } else {
        sprintf(lat_str, "%008.5f", lat);
        sprintf(lng_str, "%008.5f", lng);
        printf("Formatted coords: %s, %s\n", lat_str, lng_str);
    }

    // clear both lat and lng values
    ssd1306_clear_square(&display, MAIN_COORD_X, MAIN_LAT_Y, MAIN_COORD_WIDTH, (MAIN_LNG_Y - MAIN_LAT_Y) * ONE_CHAR_HEIGHT_PX);

    ssd1306_draw_string(&display, MAIN_COORD_X, MAIN_LAT_Y, 1, lat_str);
    ssd1306_draw_string(&display, MAIN_COORD_X, MAIN_LNG_Y, 1, lng_str);

    ssd1306_show(&display);
}

static void disp_i2c_show_topbar()
{
    ssd1306_draw_string(&display, TOPBAR_TIME_X, TOPBAR_TIME_Y, 1, TOPBAR_TIME_NO_DATA);
    ssd1306_draw_string(&display, TOPBAR_GPS_X, TOPBAR_GPS_Y, 1, TOPBAR_GPS_NO_SIGNAL);

    ssd1306_show(&display);   
}

static void disp_i2c_show_main_screen()
{
    // show coords
    ssd1306_draw_string(&display, MAIN_COORD_NAME_X, MAIN_LAT_Y, 1, MAIN_LAT_NAME);
    ssd1306_draw_string(&display, MAIN_COORD_NAME_X, MAIN_LNG_Y, 1, MAIN_LNG_NAME);

    ssd1306_draw_string(&display, MAIN_COORD_X, MAIN_LAT_Y, 1, MAIN_COORDINATES_UNDEFINED);
    ssd1306_draw_string(&display, MAIN_COORD_X, MAIN_LNG_Y, 1, MAIN_COORDINATES_UNDEFINED);

    ssd1306_show(&display);
}

static void disp_i2c_update_topbar_time_no_show(int hours, int minutes)
{
    char time_str[sizeof(TOPBAR_TIME_NO_DATA)];

    if (hours < 0) {
        strncpy(time_str, TOPBAR_TIME_NO_DATA, sizeof(TOPBAR_TIME_NO_DATA));
    } else {
        sprintf(time_str, "%02d:%02d", hours, minutes);
        printf("Formatted time: %s\n", time_str);
    }

    ssd1306_clear_square(&display, TOPBAR_TIME_X, TOPBAR_TIME_Y, TOPBAR_TIME_WIDTH, ONE_CHAR_HEIGHT_PX);

    ssd1306_draw_string(&display, TOPBAR_TIME_X, TOPBAR_TIME_Y, 1, time_str);
}

static void disp_i2c_update_topbar_gps_signal_no_show(bool has_signal)
{
    char gps_signal_str[sizeof(TOPBAR_GPS_NO_SIGNAL)];
    if (has_signal) {
        strncpy(gps_signal_str, TOPBAR_GPS_HAS_SIGNAL, sizeof(TOPBAR_GPS_HAS_SIGNAL));
    } else {
        strncpy(gps_signal_str, TOPBAR_GPS_NO_SIGNAL, sizeof(TOPBAR_GPS_NO_SIGNAL));
    }

    ssd1306_clear_square(&display, TOPBAR_GPS_X, TOPBAR_GPS_Y, TOPBAR_GPS_WIDTH, ONE_CHAR_HEIGHT_PX);

    ssd1306_draw_string(&display, TOPBAR_GPS_X, TOPBAR_GPS_Y, 1, gps_signal_str);
}
