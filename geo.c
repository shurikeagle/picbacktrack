#include <math.h>
#include <string.h>
#include "geo.h"

#define EARTH_RADIUS_METERS 6371e3
#define TO_RAD (3.1415926f / 180)
#define DIRECTION_DEVIATION_DEGREES 22 // 45 % 2
#define CARDINAL_DIRECTION_STR_LEN 3 // Including \0

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

// TODO: Change to usnigned short
float geo_directrion_bearing_degrees(geo_point_t src, geo_point_t dst)
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
    // TODO: It's not float at all
    float res = (int)fmodf((-bearing + 90), 360);

    // This solution is not good, one need to unserstand why the formula above 
    // returns negative values in the last quarter
    return res >= 0 ? res : 360 + res;
}

void geo_cardinal_direction(char *buff, geo_point_t src, geo_point_t dst) {
    memset(buff, 0, CARDINAL_DIRECTION_STR_LEN);

    float degrees = geo_directrion_bearing_degrees(src, dst);

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
