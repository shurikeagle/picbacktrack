#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "hardware/sync.h"

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
// TODO: Thinl probably one need to decrease
#define DISPLAY_BAUDRATE 40000

#pragma endregion

#pragma region controls

#define BUTTON_PIN 10
#define SAVE_POINT_BTN_DURATION_SEC 3

#pragma endregion

// TODO: Move into separated module
void init_button(void);

// TODO: Move into separated module =========
// TODO: Add struct in geo and use it instead
float saved_point_ltd;
float saved_point_lng;

void on_button_pressed(uint gpio, uint32_t events);
// ==========================================

int main() {

    stdio_init_all();

    gps_uart_init(GPS_UART_ID, GPS_UART_TX_PIN, GPS_UART_RX_PIN, BE_220_BAUDRATE);

    disp_i2c_init(DISPLAY_I2C_ID, DISPLAY_I2C_SDA_GP, DISPLAY_I2C_SCL_GP, DISPLAY_BAUDRATE);

    init_button();

    sleep_ms(100);

    rmc_data_t rmc_data;
    geo_point_t current_position;
    gps_uart_res_t get_rmc_res;
    disp_topbar_data_t topbar_data;
    geo_point_t fake_point = { .lat = 40.19996, .lng = 44.56827 };
    while (true) {
        // if (uart_is_writable(GPS_UART_ID)) {
        //     // poll the GPS module for a specific NMEA sentence
        //     uart_puts(GPS_UART_ID, "$EIGPQ,RMC*3A\r\n");
        // }
        
        get_rmc_res = gps_uart_get_rmc_blocking(&rmc_data);
        if (get_rmc_res != GPS_UART_OK) {
            printf("%s\n", gps_uart_last_err());
        }
        else {
            printf("Got rmc data (%ssignal):\n\tlat: %f\n\tlon: %f\n\tspeed: %f\n\ttime: %02d:%02d:%02d\n",
                rmc_data.has_signal ? "" : "no ",
                rmc_data.latitude,
                rmc_data.longitude,
                rmc_data.speed,
                rmc_data.time.hours,
                rmc_data.time.minutes,
                rmc_data.time.seconds);
            
            topbar_data.time_hour = rmc_data.time.hours;
            topbar_data.time_min = rmc_data.time.minutes;
            topbar_data.has_signal = rmc_data.has_signal;

            disp_i2c_update_topbar(topbar_data);
            disp_i2c_update_coords(rmc_data.latitude, rmc_data.longitude);

            // TODO: fix logic -- saved point coords must be showed always even if no gps
            if (!isnan(rmc_data.latitude) && !isnan(rmc_data.longitude)) {                
                current_position.lat = rmc_data.latitude;
                current_position.lng = rmc_data.longitude;

                float distance_to_fake_point = geo_distance_haversine_meters(
                    current_position,
                    fake_point);
                disp_saved_point_info_t save_pt_disp_info;
                save_pt_disp_info.lat = fake_point.lat;
                save_pt_disp_info.lng = fake_point.lng;
                // TODO: Bad logic! Add check with short max/min values
                save_pt_disp_info.distance_m = (unsigned short) round(distance_to_fake_point);
                char direction_buff[3];
                geo_cardinal_direction(
                    save_pt_disp_info.absolute_direction, 
                    current_position,
                    fake_point);

                disp_i2c_show_saved_point(save_pt_disp_info);
            } else {
                disp_i2c_clear_saved_point();
            }
        }

        sleep_ms(1000);
    }
}

void init_button(void)
{
    // init button
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    gpio_set_irq_enabled_with_callback(
        BUTTON_PIN,
        GPIO_IRQ_EDGE_FALL, 
        1,
        on_button_pressed);
}

void on_button_pressed(uint gpio, uint32_t events)
{
    // avoid double click caused interrupts
    uint32_t flags = save_and_disable_interrupts();

    // wait required duration to check if button is pressed
    for (size_t i = 0; i < SAVE_POINT_BTN_DURATION_SEC * 2; i++)
    {
        sleep_ms(500);
        if (gpio_get(gpio)) {
            // do nothing as button was released
            restore_interrupts(flags);
            return;
        }
    }

    // TODO: GET current coordinates from static field
    // use mutex!!!

    restore_interrupts(flags);
}
