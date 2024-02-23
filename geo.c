#include <math.h>
#include "geo.h"

#define EARTH_RADIUS_METERS 6371e3
#define TO_RAD (3.1415926f / 180)

float get_distance_haversine_meters(float lat_a, float lng_a, float lat_b, float lng_b)
{
    float dx, dy, dz;
	lng_a -= lng_b;
	lng_a *= TO_RAD;
    lat_a *= TO_RAD;
    lat_b *= TO_RAD;

	dz = sinf(lat_a) - sinf(lat_b);
	dx = cosf(lng_a) * cosf(lat_a) - cosf(lat_b);
	dy = sinf(lng_a) * cosf(lat_a);
    
	return 2 * EARTH_RADIUS_METERS * asinf(sqrtf(dx * dx + dy * dy + dz * dz) / 2);
}

float get_directrion_bearing_degrees(float lat_src, float lng_src, float lat_dst, float lng_dst)
{
    lat_src *= TO_RAD;
    lng_src *= TO_RAD;
    lat_dst *= TO_RAD;
    lng_dst *= TO_RAD;

    float delta = lng_dst - lng_src;
    float x = cosf(lat_dst) * sinf(delta);
    float y = cosf(lat_src) * sinf(lat_dst) - sinf(lat_src) * cosf(lat_dst) * cosf(delta);
    
    // convert to degrees
    float bearing = atan2f(y, x) * (1 / TO_RAD);

    // make 0...360 bearing fom North clockwise
    return (int)fmodf((-bearing + 90), 360);
}
