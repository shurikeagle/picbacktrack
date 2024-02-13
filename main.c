#include <stdlib.h>
#include <stdio.h>

#include "gps_uart.h"

#include "vendor/minmea/minmea.h"

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

    // char buff[NMEA_MAX_LENGTH];
    char *buff = malloc(sizeof(char) * NMEA_SENTENCE_MAX_LENGTH);
    int read_res;


    while (true) {
        // if (uart_is_writable(GPS_UART_ID)) {
        //     // poll the GPS module for a specific NMEA sentence
        //     uart_puts(GPS_UART_ID, "$EIGPQ,RMC*3A\r\n");
        // }

        while (uart_is_readable(GPS_UART_ID)) {
            
            // memset(buff, 0, NMEA_MAX_LENGTH);

            // uart_read_blocking(UART_ID, buff, NMEA_MAX_LENGTH);

            read_res = uart_read_nmea_sentence(buff, NMEA_SENTENCE_MAX_LENGTH);
            if (read_res != GPS_UART_OK) {
                const char *err = gps_uart_last_err();
                printf("Bad read sentence result:\n\t%s\n", err);
            }

            printf("== Raw string: ==\n%s\n===========\n", buff);

            // strict true to discard all incorrect sentences
            switch (minmea_sentence_id(buff, false)) {
                case MINMEA_SENTENCE_RMC: {
                    struct minmea_sentence_rmc frame;
                    if (minmea_parse_rmc(&frame, buff)) {
                        printf("[%d:%d:%d] ", frame.time.hours, frame.time.minutes, frame.time.seconds);

                        printf("$xxRMC: raw coordinates and speed: (%d/%d,%d/%d) %d/%d\n",
                                frame.latitude.value, frame.latitude.scale,
                                frame.longitude.value, frame.longitude.scale,
                                frame.speed.value, frame.speed.scale);
                        printf("$xxRMC fixed-point coordinates and speed scaled to three decimal places: (%d,%d) %d\n",
                                minmea_rescale(&frame.latitude, 1000),
                                minmea_rescale(&frame.longitude, 1000),
                                minmea_rescale(&frame.speed, 1000));
                        printf("$xxRMC floating point degree coordinates and speed: (%f,%f) %f\n",
                                minmea_tocoord(&frame.latitude),
                                minmea_tocoord(&frame.longitude),
                                minmea_tofloat(&frame.speed));
                    }
                    else {
                        printf("$xxRMC sentence is not parsed\n");
                    }
                } break;

                case MINMEA_SENTENCE_GGA:
                case MINMEA_SENTENCE_GSV:
                case MINMEA_SENTENCE_VTG:
                case MINMEA_SENTENCE_ZDA: {
                    printf("Skipping sentence\n");
                } break;

                case MINMEA_INVALID: {
                    printf("$xxxxx sentence is not valid\n");
                } break;

                default: {
                    printf("$xxxxx sentence is not parsed\n");
                } break;
            }

            sleep_ms(300);
        }

        printf("uart is not readable\n");
        sleep_ms(1000);
    }
    
    free(buff);
}