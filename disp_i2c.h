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


// TODO: public func descr

// TODO: Think if one need to add free (for an uart handle)
/// @brief Initializes display
/// @param i2c_inst i2c instance wich will be used for display
/// @param sda_pin sda gpio pin
/// @param scl_pin scl gpio pin
/// @param baudrate baudrate in Hz
void disp_i2c_init(i2c_inst_t *i2c, uint sda_pin, uint scl_pin, uint baudrate);

/// @brief Updates information at the topbar
void disp_i2c_update_topbar(disp_topbar_data_t topbar_data);

static void disp_i2c_update_topbar_time(int hours, int minutes);

static inline void disp_i2c_update_topbar_gps_signal(bool has_signal);