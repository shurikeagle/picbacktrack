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

rmc_data_t rmc_data = { .latitude = NAN, .longitude = NAN };

// TODO: Move into separated module =========
void init_button(void);

void on_button_pressed(uint gpio, uint32_t events);
// ==========================================

void process_existing_dst_point(void);

inline void printf_rmc_data(const rmc_data_t *rmc_data);

inline void set_topbar_data(disp_topbar_data_t *topbar_data, const rmc_data_t *rmc_data);

int main() {

    stdio_init_all();

    gps_uart_init(GPS_UART_ID, GPS_UART_TX_PIN, GPS_UART_RX_PIN, BE_220_BAUDRATE);

    disp_i2c_init(DISPLAY_I2C_ID, DISPLAY_I2C_SDA_GP, DISPLAY_I2C_SCL_GP, DISPLAY_BAUDRATE);

    init_button();

    sleep_ms(100);

    gps_uart_res_t get_rmc_res;
    disp_topbar_data_t topbar_data;
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
            printf_rmc_data(&rmc_data);
            
            set_topbar_data(&topbar_data, &rmc_data);
            disp_i2c_update_topbar(topbar_data);

            disp_i2c_update_coords(rmc_data.latitude, rmc_data.longitude);

            // TODO: fix logic -- dst point coords must be shown always even if no gps
            if (!isnanf(rmc_data.latitude) && !isnanf(rmc_data.longitude) && geo_dst_point_exists()) {
                process_existing_dst_point();
            } else {
                disp_i2c_clear_dst_point();
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

    // TODO: the next logic has to be invoked on the 2nd core to avoid waiting freezes

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

    if (!isnanf(rmc_data.latitude) && !isnanf(rmc_data.longitude)) {
        geo_point_t pt_to_save = { .lat = rmc_data.latitude, .lng = rmc_data.longitude };
        geo_save_point_as_dst(pt_to_save);
    } else {
        printf("Couldn't save current position as it's undefined\n");
    }

    restore_interrupts(flags);
}

void process_existing_dst_point(void)
{
    disp_dst_point_info_t dst_pt_disp_info;
    geo_point_t current_position = { .lat = rmc_data.latitude, .lng = rmc_data.longitude };
    geo_point_t dst_point = geo_get_dst_point();

    float distance_to_dst_point = geo_dst_point_distance_haversine_meters(current_position);
    dst_pt_disp_info.lat = dst_point.lat;
    dst_pt_disp_info.lng = dst_point.lng;
    // TODO: Bad logic! Add check with short max/min values
    dst_pt_disp_info.distance_m = (unsigned short) round(distance_to_dst_point);
    
    char direction_buff[3];
    geo_dst_point_cardinal_direction(dst_pt_disp_info.absolute_direction, current_position);

    disp_i2c_show_dst_point(dst_pt_disp_info);
}

inline void printf_rmc_data(const rmc_data_t *rmc_data)
{
    printf("Got rmc data (%ssignal):\n\tlat: %f\n\tlon: %f\n\tspeed: %f\n\ttime: %02d:%02d:%02d\n",
        rmc_data->has_signal ? "" : "no ",
        rmc_data->latitude,
        rmc_data->longitude,
        rmc_data->speed,
        rmc_data->time.hours,
        rmc_data->time.minutes,
        rmc_data->time.seconds);
}

inline void set_topbar_data(disp_topbar_data_t *topbar_data, const rmc_data_t *rmc_data)
{
    topbar_data->time_hour = rmc_data->time.hours;
    topbar_data->time_min = rmc_data->time.minutes;
    topbar_data->has_signal = rmc_data->has_signal;
}
