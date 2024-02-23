#pragma once

/// @brief Calculates a distance in meters between two geo points by Haversine formula
float get_distance_haversine_meters(float lat_a, float lng_a, float lat_b, float lng_b);

/// @brief Calculates a bearing (from North) direction in degrees from one geo point to other
float get_directrion_bearing_degrees(float lat_src, float lng_src, float lat_dst, float lng_dst);

