//
// Created by wangy on 2026/10/29.
//

#ifndef OPTIDODGE_PAW3327_H
#define OPTIDODGE_PAW3327_H
#include <stdint.h>
typedef enum {
    PAW3327_INIT_NOT_STARTED = 0,
    PAW3327_INIT_OK,
    PAW3327_INIT_ID_ERROR,
    PAW3327_INIT_CAL_ERROR,
    PAW3327_INIT_SPI_ERROR
} PAW3327_InitStatus;

typedef struct {
    uint8_t product_id;
    uint8_t inverse_id;
    uint8_t calibration_status;
    uint8_t last_hal_status;
    PAW3327_InitStatus init_status;
    uint32_t spi_errors;
} PAW3327_Diagnostics;

const PAW3327_Diagnostics *PAW3327_GetDiagnostics(void);

void PAW3327_GetID(uint8_t *MID, uint16_t *DID);
uint8_t PAW3327_ReadRegister(uint8_t address);
uint8_t PAW3327_Init(void);
uint8_t PAW3327_ReadMotion(int16_t *dx, int16_t *dy);

#endif //OPTIDODGE_PAW3327_H
