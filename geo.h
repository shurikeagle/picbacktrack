#pragma once

// TODO: Add method which returns the struct with both results below

/// @brief Calculates a distance in meters between two geo points by Haversine formula
float geo_distance_haversine_meters(float lat_a, float lng_a, float lat_b, float lng_b);

/// @brief Calculates a bearing (from North) direction in degrees from one geo point to other clockwise
float geo_directrion_bearing_degrees(float lat_src, float lng_src, float lat_dst, float lng_dst);

/// @brief Calculates the cardinal direction with one or two chars (e.g. NE, N, SW, E, etc)
/// from one point to other and saves it into provided buff
/// Note that buff must be at least 3-char length (2 char and str end)
/// @param buff 
/// @param lat_src 
/// @param lng_src 
/// @param lat_dst 
/// @param lng_dst 
void geo_cardinal_direction(char *buff, float lat_src, float lng_src, float lat_dst, float lng_dst);

