
#include <csp/csp.h>
#include "csp/csp_rtable.h"
#include "csp/csp_types.h"

#include "cmsis_os2.h"
#include "portmacro.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_iwdg.h"
#include "stm32h7xx_hal_sdram.h"

#include "cdh_main.h"
#include "cdh_can_driver.h"


extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern ADC_HandleTypeDef hadc3;

extern CRC_HandleTypeDef hcrc;

extern FDCAN_HandleTypeDef hfdcan1;
extern FDCAN_HandleTypeDef hfdcan2;

extern I2C_HandleTypeDef hi2c2;
extern I2C_HandleTypeDef hi2c4;

extern IWDG_HandleTypeDef hiwdg1;

extern RAMECC_HandleTypeDef hramecc1_m1;
extern RAMECC_HandleTypeDef hramecc1_m2;
extern RAMECC_HandleTypeDef hramecc1_m3;
extern RAMECC_HandleTypeDef hramecc1_m4;
extern RAMECC_HandleTypeDef hramecc1_m5;
extern RAMECC_HandleTypeDef hramecc2_m1;
extern RAMECC_HandleTypeDef hramecc2_m2;
extern RAMECC_HandleTypeDef hramecc2_m3;
extern RAMECC_HandleTypeDef hramecc2_m4;
extern RAMECC_HandleTypeDef hramecc2_m5;
extern RAMECC_HandleTypeDef hramecc3_m1;
extern RAMECC_HandleTypeDef hramecc3_m2;

extern SD_HandleTypeDef hsd1;
extern SD_HandleTypeDef hsd2;

extern SPI_HandleTypeDef hspi4;
extern SPI_HandleTypeDef hspi6;

extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim7;

extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart5;
extern UART_HandleTypeDef huart7;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart6;

extern SDRAM_HandleTypeDef hsdram1;

#define SDRAM_START_PTR ((void *)0xc0000000)

void cdh_main(){
    // initialise csp
    csp_init();

    // setup can1 interface
    csp_iface_t *can1_iface = cdh_can_open_and_add_interface(&hfdcan1, 0, "CAN1", CDH_CSP_ADDR, true);
    // setup can2 interface
    csp_iface_t *can2_iface = cdh_can_open_and_add_interface(&hfdcan2, 1, "CAN2", CDH_CSP_ADDR, false);

    // add route to battery
    csp_rtable_set(BAT_CSP_ADDR, 0, can1_iface, CSP_NO_VIA_ADDRESS);
    // add route to EPS
    csp_rtable_set(EPS_CSP_ADDR, 0, can1_iface, CSP_NO_VIA_ADDRESS);
    // add route to ADCS
    csp_rtable_set(ADCS_CSP_ADDR, 0, can1_iface, CSP_NO_VIA_ADDRESS);

    // add route to radio
    csp_rtable_set(RADIO_CSP_ADDR, 0, can2_iface, CSP_NO_VIA_ADDRESS);

    for(;;){
        // routing work
        csp_route_work();

        // refresh watchdog
        HAL_IWDG_Refresh(&hiwdg1);
    }
}