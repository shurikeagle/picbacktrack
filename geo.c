#include <math.h>
#include "geo.h"

#define EARTH_RADIUS_METERS 6371000
#define TO_RAD (3.1415926f / 180)

float get_distance_meters_haversine(float lat_a, float lng_a, float lat_b, float lng_b)
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

float get_directrion_degrees(float lat_src, float lng_src, float lat_dst, float lng_dst)
{
    // TODO: Change to more percise formula in terms of geo
    float dy = lat_dst - lat_src;
    float dx = lng_dst - lng_src;

    return atan2f(dy, dx) * (1 / TO_RAD);
}
