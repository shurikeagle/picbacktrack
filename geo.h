#pragma once

/// @brief Represents geopoint coordinates
typedef struct {
    float lat;
    float lng;
} geo_point_t;

// TODO: Add method which returns the struct with both results below

/// @brief Calculates a distance in meters between two geo points by Haversine formula
float geo_distance_haversine_meters(geo_point_t a, geo_point_t b);

/// @brief Calculates a bearing (from North) direction in degrees from one geo point to other clockwise
float geo_directrion_bearing_degrees(geo_point_t src, geo_point_t lng);

/// @brief Calculates the cardinal direction with one or two chars (e.g. NE, N, SW, E, etc)
/// from one point to other and saves it into provided buff
/// Note that buff must be at least 3-char length (2 char and str end)
void geo_cardinal_direction(char *buff, geo_point_t src, geo_point_t dst);

