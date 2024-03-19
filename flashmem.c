#include <string.h>

#include "hardware/flash.h"
#include "hardware/sync.h"

#include "flashmem.h"

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
    uint8_t *data_ptr = (uint8_t *)&dst_info;

    uint32_t interrupt_status = save_and_disable_interrupts();

    flash_range_erase(DST_POINT_DATA_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(DST_POINT_DATA_OFFSET, data_ptr, FLASH_PAGE_SIZE);

    restore_interrupts(interrupt_status);
}

void flashmem_remove_dst_point()
{
    flash_range_erase(DST_POINT_DATA_OFFSET, FLASH_SECTOR_SIZE);
}

bool flashmem_get_dst_point(float *const lat_out, float *const lng_out)
{
    dst_point_inmem_t dst_info;
    
    uint8_t *dst_point_ptr = (uint8_t *)(XIP_BASE + DST_POINT_DATA_OFFSET);    
    memcpy(&dst_info, dst_point_ptr, sizeof(dst_info));    

    if (dst_info.dst_point_exists != DST_POINT_EXISTS_CODE) {
        return false;
    }

    *lat_out = dst_info.lat;
    *lng_out = dst_info.lng;

    return true;
}
