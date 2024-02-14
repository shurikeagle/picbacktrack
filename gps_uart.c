#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "gps_uart.h"

#include "vendor/minmea/minmea.h"

#define GPS_UART_ERR_MAX_LENGTH 100
#define GPS_UART_READING_INTERVAL_MS 200

uart_inst_t *uart_inst;
char *sentence_buff;
char *this_last_err;

#pragma region init free

void gps_uart_init(uart_inst_t *uart, uint tx_pin, uint rx_pin, uint baudrate)
{
    uart_inst = uart;

    uart_init(uart_inst, baudrate);

    gpio_set_function(tx_pin, GPIO_FUNC_UART);
    gpio_set_function(rx_pin, GPIO_FUNC_UART);

    sentence_buff = malloc(sizeof(char) * NMEA_SENTENCE_MAX_LENGTH);
    this_last_err = malloc(GPS_UART_ERR_MAX_LENGTH);
}

void gps_uart_free()
{
    free(this_last_err);
    free(sentence_buff);
}

#pragma endregion

gps_uart_res_t uart_read_nmea_sentence(char *buff, size_t buff_cnt)
{
    gps_uart_res_t result;

    if (buff_cnt < NMEA_SENTENCE_MAX_LENGTH) {
        return GPS_UART_BUFF_TOO_SMALL;
    }

    erase_buff(buff, buff_cnt);

    if ((result = read_until_first_symbol(buff)) != GPS_UART_OK) {
        return result;
    }

    if ((result = read_up_to_the_end(buff)) != GPS_UART_OK) {
        erase_buff(buff, buff_cnt);
    }

    return result;
}

gps_uart_res_t gps_uart_get_rmc_blocking(rmc_data_t *out_data) {
    gps_uart_res_t attempt_result;    
    
    while (true) {

        if (!uart_is_readable) {
            // just wait a bit to try again
            // TODO: attampt number logic maybe
            sleep_ms(GPS_UART_READING_INTERVAL_MS);

            continue;
        }

        attempt_result = uart_read_nmea_sentence(sentence_buff, NMEA_SENTENCE_MAX_LENGTH);
        // TODO: Think how to handle this situation, maybe one need to return bad result in some cases, probably also implement attemp number
        if (attempt_result != GPS_UART_OK) {
            printf("Bad attempt of reading nmea sentence: %s\n", gps_uart_last_err());

            continue;
        }

        int sentence_id = minmea_sentence_id(sentence_buff, false);

        if (sentence_id == MINMEA_INVALID) {
            printf("Got invalid sentence: (%s)\n", sentence_buff);

            continue;
        }

        if (sentence_id != MINMEA_SENTENCE_RMC) {
            continue;
        }

        struct minmea_sentence_rmc frame;
        if (!minmea_parse_rmc(&frame, sentence_buff)) {
            // TODO: Attempts
            printf("Couldn't parse RMC sentence: (%s)\n", sentence_buff);

            continue;
        }

        printf("DEBUG: Got RMC sentence raw: %s\n", sentence_buff);
        out_data->latitude = minmea_tocoord(&frame.latitude);
        out_data->longitude = minmea_tocoord(&frame.longitude);
        out_data->speed = minmea_tofloat(&frame.speed);
        out_data->time.hours = frame.time.hours;
        out_data->time.minutes = frame.time.minutes;
        out_data->time.seconds = frame.time.seconds;
        out_data->has_signal = frame.valid;

        return GPS_UART_OK;
    }
}

const char *gps_uart_last_err(void)
{
    return this_last_err;
}

static gps_uart_res_t read_until_first_symbol(char *buff)
{
    char cur_char;

    // whait until the nearest sentence starts, 
    // but if there aren't start symbol after sentence max length, 
    // probably there is some error
    for (size_t i = 0; ; i++) {
        if (i > NMEA_SENTENCE_MAX_LENGTH) {
            write_last_err("No start sentence symbol in sentence");

            return GPS_UART_UNEXPECTED;
        }

        cur_char = uart_getc(uart_inst);
        if (cur_char == '$') {
            buff[0] = cur_char;

            return GPS_UART_OK;
        }
    }
}

static gps_uart_res_t read_up_to_the_end(char *buff)
{
    char cur_char;
    size_t buf_pos = 1;

    while (true) {
        // again, sentence can't be longer than nmea max length in normal cases
        if (buf_pos > NMEA_SENTENCE_MAX_LENGTH) {
            write_last_err("Sentence is too long");

            return GPS_UART_BAD_SENTENCE;
        }

        cur_char = uart_getc(uart_inst);

        // there won't be two start-sentence symbols in one sentence
        // but sometimes there are such artifacts, so, let's rewrite buff, like this is a start of the sentence
        if (cur_char == '$') {
            buff[0] = cur_char;
            buf_pos = 1;

            continue;
        }

        // if we've got last two characters of NMEA sentence, replace them with null-terminated str
        // TODO: think if one need to make this replace or not
        if (cur_char == '\n' && buff[buf_pos - 1] == '\r') {
            buff[buf_pos - 1] = '\0';

            break;
        }

        buff[buf_pos] = cur_char;
        buf_pos++;
    }

    return GPS_UART_OK;
}

static inline void erase_buff(char *buff, size_t buff_cnt)
{
    memset(buff, 0, buff_cnt);
}

static void write_last_err(char *err)
{
    memset(gps_uart_last_err, 0, GPS_UART_ERR_MAX_LENGTH);

    strncpy(err, this_last_err, GPS_UART_ERR_MAX_LENGTH);
}
