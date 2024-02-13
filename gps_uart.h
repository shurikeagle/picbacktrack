#pragma once

#include "pico/stdlib.h"

#define NMEA_SENTENCE_MAX_LENGTH 82

/// @brief Results of the gps_uart operations
typedef enum {
    GPS_UART_OK = 0,
    GPS_UART_UNEXPECTED = -1,
    GPS_UART_BUFF_TOO_SMALL = -2,
    GPS_UART_BAD_SENTENCE = -3
} gps_uart_res_t;

/// @brief Initializes gps_uart module
/// @param uart uart wich is used for gps
/// @param tx_pin uart tx pin
/// @param rx_pin uart rx pin
/// @param baudrate uart baudrate, one recommend to check correct baudrate in gps module datasheet
void gps_uart_init(uart_inst_t *uart, uint tx_pin, uint rx_pin, uint baudrate);

/// @brief Frees gps_uart module resources. This func DOESN'T frees uart, provided in init func
void gps_uart_free();

/// @brief Reads a sentence from nmea format from provided uart 
/// @param buff buffer to wtore sentence
/// @param buff_cnt Number of symbols to read from uart. Must be less than NMEA_SENTENCE_MAX_LENGTH
/// @return operation result
gps_uart_res_t uart_read_nmea_sentence(char *buff, size_t buff_cnt);

const char *gps_uart_last_err(void);

#pragma region private funcs

static gps_uart_res_t read_until_first_symbol(char *buff);
static gps_uart_res_t read_up_to_the_end(char *buff);
static inline void erase_buff(char *buff, size_t buff_cnt);
static void write_last_err(char *err);

#pragma endregion