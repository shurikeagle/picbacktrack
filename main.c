#include <stdlib.h>
#include <stdio.h>

#include "gps_uart.h"
#include "display.h"

#pragma region GPS

#define GPS_UART_ID uart0

#define GPS_UART_TX_PIN 0
#define GPS_UART_RX_PIN 1
#define NEO_6M_DEFAULT_BAUD_RATE 9600

#pragma endregion


#pragma region DISPLAY

#define DISPLAY_I2C_ID i2c0
#define DISPLAY_I2C_SDA_GP 20
#define DISPLAY_I2C_SCL_GP 21

#pragma endregion

int main() {

    stdio_init_all();

    gps_uart_init(GPS_UART_ID, GPS_UART_TX_PIN, GPS_UART_RX_PIN, NEO_6M_DEFAULT_BAUD_RATE);

    sleep_ms(100);

    while (true) {
        // if (uart_is_writable(GPS_UART_ID)) {
        //     // poll the GPS module for a specific NMEA sentence
        //     uart_puts(GPS_UART_ID, "$EIGPQ,RMC*3A\r\n");
        // }

        rmc_data_t rmc_data;
        gps_uart_res_t get_rmc_res = gps_uart_get_rmc_blocking(&rmc_data);
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
        }

        sleep_ms(1000);
    }
}