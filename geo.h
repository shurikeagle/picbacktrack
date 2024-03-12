#pragma once

/// @brief Represents geopoint coordinates
typedef struct {
    float lat;
    float lng;
} geo_point_t;

/// @brief Calculates a distance in meters between two geo points by Haversine formula
float geo_distance_haversine_meters(geo_point_t a, geo_point_t b);

/// @brief Calculates a distance in meters between provided src and saved destination point
float geo_dst_point_distance_haversine_meters(geo_point_t src);

/// @brief Calculates a bearing (from North) direction in degrees from one geo point to other clockwise
unsigned short geo_directrion_bearing_degrees(geo_point_t src, geo_point_t dst);

/// @brief Calculates the cardinal direction with one or two chars (e.g. NE, N, SW, E, etc)
/// from one point to other and saves it into provided buff
/// Note that buff must be at least 3-char length (2 char and str end)
void geo_cardinal_direction(char *buff, geo_point_t src, geo_point_t dst);

/// @brief Calculates the cardinal direction with one or two chars (e.g. NE, N, SW, E, etc)
/// from provided point to the saved destination point and saves it into provided buff
/// Note that buff must be at least 3-char length (2 char and str end)
void geo_dst_point_cardinal_direction(char *buff, geo_point_t src);

/// @brief Calculates the relative direction with one or two chars (e.g. NE, N, SW, E, etc)
/// from two provided points of currrent position to other one and saves it into provided buff
/// @param src_prev first (previous) point of src position
/// @param src_current second (current) point of src position
void geo_relative_direction(char *buff, geo_point_t src_prev, geo_point_t src_current, geo_point_t dst);

/// @brief Calculates the relative direction with one or two chars (e.g. NE, N, SW, E, etc)
/// from two provided points to the saved destination point and saves it into provided buff
/// @param src_prev first (previous) point of src position
/// @param src_current second (current) point of src position
void geo_dst_point_relative_direction(char *buff, geo_point_t src_prev, geo_point_t src_current);

/// @brief Gets the cardinal direction with one or two chars (e.g. NE, N, SW, E, etc)
/// by provided degrees and saves it into provided buff
void geo_direction_by_degrees(char *buff, unsigned short degrees);

/// @brief Saves provided geo point as destination point
void geo_save_point_as_dst(geo_point_t pt);

/// @brief Returns destination point
geo_point_t geo_get_dst_point(void);

/// @brief Checks if point is valid
bool geo_point_is_valid(float lat, float lng);

/// @brief Checks if any dst point was saved before and has valid values
bool geo_dst_point_exists(void);

