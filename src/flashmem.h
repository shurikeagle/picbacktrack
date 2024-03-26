#pragma once

/// @brief Saves destination point with provided coordinates into flash
void flashmem_save_dst_point(float lat, float lng);

/// @brief Removes destination point from flash
void flashmem_remove_dst_point();

/// @brief Writes out destination point into provided buffers
/// @param lat_out buffer to write latitude
/// @param lng_out buffer to write longitude
/// @return if dst point exists in memory and was written out
bool flashmem_get_dst_point(float *const lat_out, float *const lng_out);