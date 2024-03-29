#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pico/multicore.h"

#include "gps_uart.h"
#include "geo.h"
#include "disp_i2c.h"

#pragma region GPS

#define GPS_UART_ID uart1

#define GPS_UART_TX_PIN 4
#define GPS_UART_RX_PIN 5
#define NEO_6M_BAUDRATE 9600
#define BE_220_BAUDRATE 38400

#pragma endregion


#pragma region DISPLAY

#define DISPLAY_I2C_ID i2c0
#define DISPLAY_I2C_SDA_GP 20
#define DISPLAY_I2C_SCL_GP 21
// TODO: Think probably one should to decrease
#define DISPLAY_BAUDRATE 40000

#pragma endregion

#pragma region controls

#define BUTTON_PIN 10
#define SAVE_POINT_BTN_DURATION_SEC 2

#pragma endregion

rmc_data_t rmc_data = { .latitude = NAN, .longitude = NAN };
geo_point_t current_position = { .lat = NAN, .lng = NAN };
geo_point_t prev_position = { .lat = NAN, .lng = NAN };

// TODO: Move into separated module =========
void init_controls(void);
// ==========================================

static void process_existing_dst_point(void);
static void save_or_clear_dst_point(void);
static inline void printf_rmc_data();
static inline void update_position();
static inline void set_topbar_data(disp_topbar_data_t *topbar_data);
static inline bool button_pressed();

/// @brief Controls processing
/// This core uses loop logic for controls instead of interrupts to avoid such issues as multiple interrupts / multiple control actions
void core1_main() {
    while (true)
    {
        // check if button pressed
        if (button_pressed()) {
            // wait required duration to check if button is still pressed to avoid occassional clicks
            for (size_t i = 0; i < SAVE_POINT_BTN_DURATION_SEC * 4; i++)
            {
                busy_wait_ms(250);
                if (!button_pressed()) {
                    // do nothing as button was released
                    goto continue_main;
                }
            }

            save_or_clear_dst_point();

            // wait to prevent double pushes before the first core updates data
            // TODO: Implement better logic to wait less than pre-configured time, e.g. cross-core events
            busy_wait_ms(2000);
        }

        continue_main:;
    }    
}

int main() {

    stdio_init_all();
    gps_uart_init(GPS_UART_ID, GPS_UART_TX_PIN, GPS_UART_RX_PIN, BE_220_BAUDRATE);
    geo_init();
    disp_i2c_init(DISPLAY_I2C_ID, DISPLAY_I2C_SDA_GP, DISPLAY_I2C_SCL_GP, DISPLAY_BAUDRATE);
    init_controls();
    
    // run controls logic on the second core
    multicore_launch_core1(&core1_main);

    sleep_ms(100);

    gps_uart_res_t get_rmc_res;
    disp_topbar_data_t topbar_data;
    while (true) {
        get_rmc_res = gps_uart_get_rmc_blocking(&rmc_data);
        if (get_rmc_res != GPS_UART_OK) {
            printf("%s\n", gps_uart_last_err());
        }
        else {
            printf_rmc_data();
            
            update_position();

            set_topbar_data(&topbar_data);
            disp_i2c_update_topbar(topbar_data);

            disp_i2c_update_coords(rmc_data.latitude, rmc_data.longitude);

            // TODO: fix logic -- dst point coords must be shown always even if gps is lost
            if (geo_point_is_valid(rmc_data.latitude, rmc_data.longitude) && geo_dst_point_exists()) {
                process_existing_dst_point();
            } else {
                disp_i2c_clear_dst_point();
            }
        }

        sleep_ms(1000);
    }
}

void init_controls()
{
    // init button
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);
}

void process_existing_dst_point(void)
{
    disp_dst_point_info_t disp_dst_pt_info;
    geo_point_t dst_point = geo_get_dst_point();

    float distance_to_dst_point = geo_dst_point_distance_haversine_meters(current_position);
    disp_dst_pt_info.lat = dst_point.lat;
    disp_dst_pt_info.lng = dst_point.lng;
    // TODO: Bad logic! Add check with short max/min values
    disp_dst_pt_info.distance_m = (unsigned short) round(distance_to_dst_point);
    
    geo_dst_point_cardinal_direction(disp_dst_pt_info.absolute_direction, current_position);
    if (geo_point_is_valid(prev_position.lat, prev_position.lng)) {
        geo_dst_point_relative_direction(disp_dst_pt_info.relative_direction, prev_position, current_position);
    } else { // handle situation when this is the first cycle with defined position
        strncpy(disp_dst_pt_info.relative_direction, "", 1);
    }

    disp_i2c_show_dst_point(&disp_dst_pt_info);
}

void save_or_clear_dst_point(void)
{
    // if exists -- remove ...
    if (geo_dst_point_exists()) {
        geo_clear_dst_point();

        return;
    }

    // ... if not exists -- add

    // check if rmc_data is defined
    if (geo_point_is_valid(rmc_data.latitude, rmc_data.longitude)) {
        geo_point_t pt_to_save = { .lat = rmc_data.latitude, .lng = rmc_data.longitude };
        geo_save_point_as_dst(pt_to_save);
    } else {
        // TODO: Think how to notify user that current position is not defined
    }
}

inline void printf_rmc_data()
{
    printf("Got rmc data (%ssignal):\n\tlat: %f\n\tlon: %f\n\tspeed: %f\n\ttime: %02d:%02d:%02d\n",
        rmc_data.has_signal ? "" : "no ",
        rmc_data.latitude,
        rmc_data.longitude,
        rmc_data.speed,
        rmc_data.time.hours,
        rmc_data.time.minutes,
        rmc_data.time.seconds);
}

inline void update_position()
{
    prev_position.lat = current_position.lat;
    prev_position.lng = current_position.lng;
    current_position.lat = rmc_data.latitude;
    current_position.lng = rmc_data.longitude;
}

inline void set_topbar_data(disp_topbar_data_t *topbar_data)
{
    topbar_data->time_hour = rmc_data.time.hours;
    topbar_data->time_min = rmc_data.time.minutes;
    topbar_data->has_signal = rmc_data.has_signal;
}

inline bool button_pressed()
{
    return !gpio_get(BUTTON_PIN);
}
