#include <math.h>
#include <string.h>

#include "pico/mutex.h"

#include "geo.h"
#include "flashmem.h"

#define EARTH_RADIUS_METERS 6371e3
#define TO_RAD (3.1415926f / 180)
#define DIRECTION_DEVIATION_DEGREES 22 // 45 % 2
#define CARDINAL_DIRECTION_STR_LEN 3 // Including \0

static mutex_t dst_pt_mx;
static geo_point_t dst_pt;

void geo_init(void)
{
    mutex_init(&dst_pt_mx);

    float dst_lat, dst_lng;

    bool has_inflashmem_dst = flashmem_get_dst_point(&dst_lat, &dst_lng);
    if (has_inflashmem_dst) {
        dst_pt.lat = dst_lat;
        dst_pt.lng = dst_lng;
    } else {
        dst_pt.lat = NAN;
        dst_pt.lng = NAN;
    }
}

// TODO: Research if this formula neccessary for the distances < 10-20km
// TODO: this formula doesn't include the difference of sea level height, a research required
//  Such task is very complex, because, for example, 100m height of a first point and 200m height of a second point
//  doesn't guarantee that the difference is just 100m height (because there might be some mountains between these points)

float geo_distance_haversine_meters(geo_point_t a, geo_point_t b)
{
    float dx, dy, dz;
	a.lng -= b.lng;
	a.lng *= TO_RAD;
    a.lat *= TO_RAD;
    b.lat *= TO_RAD;

	dz = sinf(a.lat) - sinf(b.lat);
	dx = cosf(a.lng) * cosf(a.lat) - cosf(b.lat);
	dy = sinf(a.lng) * cosf(a.lat);

	return 2 * EARTH_RADIUS_METERS * asinf(sqrtf(dx * dx + dy * dy + dz * dz) / 2);
}

float geo_dst_point_distance_haversine_meters(geo_point_t src)
{
    return geo_dst_point_exists() ? geo_distance_haversine_meters(src, dst_pt) : NAN;
}

unsigned short geo_directrion_bearing_degrees(geo_point_t src, geo_point_t dst)
{
    src.lat *= TO_RAD;
    src.lng *= TO_RAD;
    dst.lat *= TO_RAD;
    dst.lng *= TO_RAD;

    float delta = dst.lng - src.lng;
    float x = cosf(dst.lat) * sinf(delta);
    float y = cosf(src.lat) * sinf(dst.lat) - sinf(src.lat) * cosf(dst.lat) * cosf(delta);
    
    // convert to degrees
    float bearing = atan2f(y, x) * (1 / TO_RAD);

    // make 0...360 bearing from North clockwise
    unsigned short res = (unsigned short)fmodf((-bearing + 90), 360);

    // This solution is not good, one need to unserstand why the formula above 
    // returns negative values in the last quarter
    return res >= 0 ? res : 360 + res;
}

void geo_cardinal_direction(char *buff, geo_point_t src, geo_point_t dst) {
    memset(buff, 0, CARDINAL_DIRECTION_STR_LEN);

    unsigned short degrees = geo_directrion_bearing_degrees(src, dst);

    geo_direction_by_degrees(buff, degrees);
}

void geo_dst_point_cardinal_direction(char *buff, geo_point_t src) {
    if (geo_dst_point_exists()) {
        return geo_cardinal_direction(buff, src, dst_pt);
    }

    strncpy(buff, "er", 2);
}

void geo_relative_direction(char *buff, geo_point_t src_prev, geo_point_t src_current, geo_point_t dst) {
    short prev_to_current_bearing = geo_directrion_bearing_degrees(src_prev, src_current);
    short current_to_dst_bearing = geo_directrion_bearing_degrees(src_current, dst);

    short result = current_to_dst_bearing - prev_to_current_bearing;
    if (result < 0) {
        result = 360 - result;
    }

    geo_direction_by_degrees(buff, (unsigned short) result);
}

void geo_dst_point_relative_direction(char *buff, geo_point_t src_prev, geo_point_t src_current) {
    if (geo_dst_point_exists()) {
        return geo_relative_direction(buff, src_prev, src_current, dst_pt);
    }

    strncpy(buff, "er", 2);
}

void geo_direction_by_degrees(char *buff, unsigned short degrees)
{
    if (degrees >= 360 - DIRECTION_DEVIATION_DEGREES || degrees <= DIRECTION_DEVIATION_DEGREES) {
        strncpy(buff, "N", 1);
    } else if (degrees <= 45 + DIRECTION_DEVIATION_DEGREES) {
        strncpy(buff, "NE", 2);
    } else if (degrees <= 90 + DIRECTION_DEVIATION_DEGREES) {
        strncpy(buff, "E", 1);
    } else if (degrees <= 135 + DIRECTION_DEVIATION_DEGREES) {
        strncpy(buff, "SE", 2);
    } else if (degrees <= 180 + DIRECTION_DEVIATION_DEGREES) {
        strncpy(buff, "S", 1);
    } else if (degrees <= 225 + DIRECTION_DEVIATION_DEGREES) {
        strncpy(buff, "SW", 2);
    } else if (degrees <= 270 + DIRECTION_DEVIATION_DEGREES) {
        strncpy(buff, "W", 1);
    } else if (degrees >= 315 + DIRECTION_DEVIATION_DEGREES) {
        strncpy(buff, "NW", 2);
    } else {
        strncpy(buff, "er", 2);
    }
}

void geo_save_point_as_dst(geo_point_t pt)
{
    mutex_enter_blocking(&dst_pt_mx);

    dst_pt = pt;
    flashmem_save_dst_point(pt.lat, pt.lng);

    mutex_exit(&dst_pt_mx);
}

void geo_clear_dst_point() 
{
    mutex_enter_blocking(&dst_pt_mx);

    dst_pt.lat = NAN;
    dst_pt.lng = NAN;
    flashmem_remove_dst_point();

    mutex_exit(&dst_pt_mx);
}

geo_point_t geo_get_dst_point() 
{
    mutex_enter_blocking(&dst_pt_mx);

    geo_point_t res = dst_pt;

    mutex_exit(&dst_pt_mx);

    return res;
}

bool geo_point_is_valid(float lat, float lng) 
{
    return !isnanf(lat) && !isnanf(lng);
}

bool geo_dst_point_exists(void)
{
    mutex_enter_blocking(&dst_pt_mx);

    bool res = geo_point_is_valid(dst_pt.lat, dst_pt.lng);

    mutex_exit(&dst_pt_mx);
    
    return res;
}
