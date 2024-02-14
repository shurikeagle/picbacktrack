#pragma once

#include "pico/stdlib.h"
#include "hardware/i2c.h"

/// @brief represents display topbar data (time, signal, etc)
typedef struct
{
    int time_min;
    int time_sec;
    bool has_signal;
} disp_topbar_data_t;


// TODO: public func descr

// TODO: Think if one need to add free (for an uart handle)
/// @brief Initializes display
/// @param i2c_inst i2c instance wich will be used for display
/// @param sda_pin sda gpio pin
/// @param scl_pin scl gpio pin
/// @param baudrate baudrate in Hz
void disp_init(i2c_inst_t *i2c_inst, uint sda_pin, uint scl_pin, uint baudrate);

// TODO: think if send ptr instead of value for mem economy
void disp_update_topbar(disp_topbar_data_t topbar_data);