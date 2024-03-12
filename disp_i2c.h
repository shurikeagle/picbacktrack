#pragma once

#include "pico/stdlib.h"
#include "hardware/i2c.h"

/// @brief represents display topbar data (time, signal, etc)
typedef struct
{
    int time_hour;
    int time_min;
    bool has_signal;
} disp_topbar_data_t;

/// @brief represents destination point info (coordinates, distance, direction to point)
typedef struct 
{
    float lat;
    float lng;
    unsigned short distance_m;
    char absolute_direction[3]; // e.g. N, NE, etc
    char relative_direction[3];
} disp_dst_point_info_t;

// TODO: Think if one need to add free (for an uart handle)
/// @brief Initializes display
/// @param i2c_inst i2c instance wich will be used for display
/// @param sda_pin sda gpio pin
/// @param scl_pin scl gpio pin
/// @param baudrate baudrate in Hz
void disp_i2c_init(i2c_inst_t *i2c, uint sda_pin, uint scl_pin, uint baudrate);

/// @brief Updates information at the topbar
void disp_i2c_update_topbar(disp_topbar_data_t topbar_data);

/// @brief Prints current coordinates on the screen
void disp_i2c_update_coords(float lat, float lng);

/// @brief Prints destination point info
void disp_i2c_show_dst_point(const disp_dst_point_info_t *point);

/// @brief Clears dst point info
void disp_i2c_clear_dst_point(void);