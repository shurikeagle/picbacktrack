#include "hardware/flash.h"
#include "flashmem.h"

// define flash size for custom rp2040 based board
#define FLASH_SIZE 16 * 1024 * 1024

#ifndef FLASH_SIZE
#define FLASH_SIZE PICO_FLASH_SIZE_BYTES
#endif

#define DATA_FLASH_OFFSET (FLASH_SIZE - FLASH_SECTOR_SIZE)
#define DST_POINT_DATA_OFFSET DATA_FLASH_OFFSET
#define DST_POINT_EXISTS_CODE 1

typedef struct
{    
    uint8_t dst_point_exists;
    float lat;
    float lng;
} dst_point_inmem_t;


void flashmem_save_dst_point(float lat, float lng)
{
    dst_point_inmem_t dst_info = { .dst_point_exists = DST_POINT_EXISTS_CODE, .lat = lat, .lng = lng };

    flash_range_erase(DST_POINT_DATA_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(DST_POINT_DATA_OFFSET, (uint8_t *)&dst_info, FLASH_SECTOR_SIZE);
}

bool flashmem_get_dst_point(float *const lat_out, float *const lng_out)
{
    dst_point_inmem_t *dst_point_info_ptr = (dst_point_inmem_t *)(XIP_BASE + DST_POINT_DATA_OFFSET);

    if (dst_point_info_ptr->dst_point_exists != DST_POINT_EXISTS_CODE) {
        return false;
    }

    *lat_out = dst_point_info_ptr->lat;
    *lng_out = dst_point_info_ptr->lng;

    return true;
}
