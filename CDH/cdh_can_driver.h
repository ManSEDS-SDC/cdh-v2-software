#ifndef CDH_CAN_DRIVER_H
#define CDH_CAN_DRIVER_H

#include <stddef.h>
#include <stdint.h>
#include "csp/csp_types.h"
#include "stm32h7xx_hal_fdcan.h"

csp_iface_t *cdh_can_open_and_add_interface(FDCAN_HandleTypeDef *hfdcan, int fifo_n, const char *ifname, uint16_t address, bool is_default);

#endif // CDH_CAN_DRIVER_H