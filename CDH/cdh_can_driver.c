
#include <stdint.h>
#include <stdio.h>

#include "csp/csp.h"
#include "csp/interfaces/csp_if_can.h"
#include "csp/csp_error.h"

#include "main.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "portmacro.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_conf.h"
#include "stm32h7xx_hal_fdcan.h"

csp_iface_t can1_iface;
csp_iface_t can2_iface;

csp_can_interface_data_t can1_iface_data;
csp_can_interface_data_t can2_iface_data;

int dlc_to_len(uint32_t dlc)
{
    int len;

    if (dlc == FDCAN_DLC_BYTES_0){
        len = 0;
    } else if (dlc == FDCAN_DLC_BYTES_1) {
        len = 1;
    } else if (dlc == FDCAN_DLC_BYTES_2) {
        len = 2;
    } else if (dlc == FDCAN_DLC_BYTES_3) {
        len = 3;
    } else if (dlc == FDCAN_DLC_BYTES_4) {
        len = 4;
    } else if (dlc == FDCAN_DLC_BYTES_5) {
        len = 5;
    } else if (dlc == FDCAN_DLC_BYTES_6) {
        len = 6;
    } else if (dlc == FDCAN_DLC_BYTES_7) {
        len = 7;
    } else if (dlc == FDCAN_DLC_BYTES_8) {
        len = 8;
    } else if (dlc == FDCAN_DLC_BYTES_12) {
        len = 12;
    } else if (dlc == FDCAN_DLC_BYTES_16) {
        len = 16;
    } else if (dlc == FDCAN_DLC_BYTES_20) {
        len = 20;
    } else if (dlc == FDCAN_DLC_BYTES_24) {
        len = 24;
    } else if (dlc == FDCAN_DLC_BYTES_32) {
        len = 32;
    } else if (dlc == FDCAN_DLC_BYTES_48) {
        len = 48;
    } else if (dlc == FDCAN_DLC_BYTES_64) {
        len = 64;
    } else {
        len = -1;
    }

    return len;
}

uint32_t len_to_dlc(int len)
{
    uint32_t dlc;

    if (len == 0){
        dlc = FDCAN_DLC_BYTES_0;
    } else if (len == 1) {
        dlc = FDCAN_DLC_BYTES_1;
    } else if (len == 2) {
        dlc = FDCAN_DLC_BYTES_2;
    } else if (len == 3) {
        dlc = FDCAN_DLC_BYTES_3;
    } else if (len == 4) {
        dlc = FDCAN_DLC_BYTES_4;
    } else if (len == 5) {
        dlc = FDCAN_DLC_BYTES_5;
    } else if (len == 6) {
        dlc = FDCAN_DLC_BYTES_6;
    } else if (len == 7) {
        dlc = FDCAN_DLC_BYTES_7;
    } else if (len == 8) {
        dlc = FDCAN_DLC_BYTES_8;
    } else if (len == 12) {
        dlc = FDCAN_DLC_BYTES_12;
    } else if (len == 16) {
        dlc = FDCAN_DLC_BYTES_16;
    } else if (len == 20) {
        dlc = FDCAN_DLC_BYTES_20;
    } else if (len == 24) {
        dlc = FDCAN_DLC_BYTES_24;
    } else if (len == 32) {
        dlc = FDCAN_DLC_BYTES_32;
    } else if (len == 48) {
        dlc = FDCAN_DLC_BYTES_48;
    } else if (len == 64) {
        dlc = FDCAN_DLC_BYTES_64;
    } else {
        len = -1;
    }

    return dlc;
}

int cdh_can_tx_frame(void *driver_data, uint32_t id, const uint8_t * data, uint8_t dlc)
{
    // cast pointer to hardware can
    FDCAN_HandleTypeDef *hfdcan = driver_data;

    FDCAN_TxHeaderTypeDef TxHeader;
    
    /* Prepare Tx Header */
    TxHeader.Identifier = id;
    TxHeader.IdType = FDCAN_EXTENDED_ID;
    TxHeader.TxFrameType = FDCAN_DATA_FRAME;
    TxHeader.DataLength = len_to_dlc(dlc);
    TxHeader.ErrorStateIndicator = FDCAN_ESI_PASSIVE;
    TxHeader.BitRateSwitch = FDCAN_BRS_ON;
    TxHeader.FDFormat = FDCAN_FD_CAN;
    TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    TxHeader.MessageMarker = 0;

    if (HAL_FDCAN_AddMessageToTxFifoQ(hfdcan, &TxHeader, data) != HAL_OK){
        return CSP_ERR_BUSY;
    }

    return CSP_ERR_NONE;
}

csp_iface_t * cdh_can_open_and_add_interface(FDCAN_HandleTypeDef *hfdcan, int fifo_n, const char *ifname, uint16_t address, bool is_default)
{
    assert_param((fifo_n == 0) || (fifo_n == 1));

    // create filter
    FDCAN_FilterTypeDef sFilterConfig = {0};

    // mask the csp address bits
    uint32_t dst_mask = ((1U << CFP2_DST_SIZE) - 1U) << CFP2_DST_OFFSET;

    // only accept extended ID
    sFilterConfig.IdType = FDCAN_EXTENDED_ID;
    sFilterConfig.FilterIndex  = 0;
    sFilterConfig.FilterType   = FDCAN_FILTER_MASK;
    sFilterConfig.FilterID1    = address << CFP2_DST_OFFSET;
    sFilterConfig.FilterID2    = dst_mask;

    if (HAL_FDCAN_ConfigFilter(hfdcan, &sFilterConfig) != HAL_OK)
    {
        Error_Handler();
    }

    /* Start the FDCAN module */
    if (HAL_FDCAN_Start(hfdcan) != HAL_OK)
    {
        Error_Handler();
    }

    csp_iface_t *can_iface;
    csp_can_interface_data_t *can_iface_data;
    
    if (fifo_n == 0)
    {
        can_iface = &can1_iface;
        can_iface_data = &can1_iface_data;
    } else
    {
        can_iface = &can2_iface;
        can_iface_data = &can2_iface_data;
    }
    
    can_iface_data->cfp_packet_counter = 0;
    can_iface_data->pbufs = NULL;
    can_iface_data->tx_func = cdh_can_tx_frame;

    can_iface->name = ifname;
    can_iface->addr = address;
    can_iface->driver_data = hfdcan;
    can_iface->netmask = 0;
    can_iface->interface_data = can_iface_data;
    can_iface->is_default = is_default;

    if (csp_can_add_interface(can_iface) != CSP_ERR_NONE)
    {
        Error_Handler();
    }

    // start recieving messages
    if (HAL_FDCAN_ActivateNotification(hfdcan, (fifo_n == 0) ? FDCAN_IT_RX_FIFO0_NEW_MESSAGE: FDCAN_IT_RX_FIFO1_NEW_MESSAGE, 0) != HAL_OK)
    {
        Error_Handler();
    }

    return can_iface;
}

/// interupt callback for Fifo0
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    int xTaskWoken = pdFALSE;
    FDCAN_RxHeaderTypeDef RxHeader;
    uint8_t RxData[8];

    if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)
    {
        /* Retrieve Rx messages from RX FIFO0 */
        if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK)
        {
            Error_Handler();
        }

        csp_can_rx(&can1_iface, RxHeader.Identifier, RxData, dlc_to_len(RxHeader.DataLength), &xTaskWoken);
    }

    portYIELD_FROM_ISR(xTaskWoken);
}

/// interupt callback for Fifo1
void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo1ITs)
{
    int xTaskWoken = pdFALSE;
    FDCAN_RxHeaderTypeDef RxHeader;
    uint8_t RxData[8];

    if((RxFifo1ITs & FDCAN_IT_RX_FIFO1_NEW_MESSAGE) != RESET)
    {
        /* Retrieve Rx messages from RX FIFO1 */
        if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO1, &RxHeader, RxData) != HAL_OK)
        {
            Error_Handler();
        }

        csp_can_rx(&can2_iface, RxHeader.Identifier, RxData, dlc_to_len(RxHeader.DataLength), &xTaskWoken);
    }

    portYIELD_FROM_ISR(xTaskWoken);
}