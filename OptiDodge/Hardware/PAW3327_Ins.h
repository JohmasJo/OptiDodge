#ifndef OPTIDODGE_PAW3327_INS_H
#define OPTIDODGE_PAW3327_INS_H

#define PAW3327_REG_PRODUCT_ID       0x00U
#define PAW3327_REG_MOTION           0x02U
#define PAW3327_REG_DELTA_X_L        0x03U
#define PAW3327_REG_DELTA_X_H        0x04U
#define PAW3327_REG_DELTA_Y_L        0x05U
#define PAW3327_REG_DELTA_Y_H        0x06U
#define PAW3327_REG_MOTION_BURST     0x16U
#define PAW3327_REG_RESOLUTION       0x1BU
#define PAW3327_REG_AXIS_CONTROL     0x20U
#define PAW3327_REG_POWER_UP_RESET   0x3AU
#define PAW3327_REG_INV_PRODUCT_ID   0x3FU
#define PAW3327_REG_BANK_SELECT      0x7FU

/* 厂家初始化使用的地址；含义取决于当前Bank */
#define PAW3327_REG_INIT_1A          0x1AU
#define PAW3327_REG_INIT_1B          0x1BU
#define PAW3327_REG_INIT_1C          0x1CU
#define PAW3327_REG_INIT_49          0x49U
#define PAW3327_REG_INIT_4A          0x4AU
#define PAW3327_REG_INIT_4D          0x4DU
#define PAW3327_REG_INIT_62          0x62U
#define PAW3327_REG_INIT_6C          0x6CU
#define PAW3327_REG_INIT_6D          0x6DU
#define PAW3327_REG_INIT_6F          0x6FU
#define PAW3327_REG_INIT_7A          0x7AU

#define PAW3327_READ_ADDRESS_MASK    0x7FU
#define PAW3327_WRITE_FLAG           0x80U
#define PAW3327_DUMMY_BYTE           0x00U
#define PAW3327_MOTION_FLAG          0x80U
#define PAW3327_RESET_VALUE          0x5AU
#define PAW3327_INIT_OK_MASK         0x02U
#define PAW3327_EXPECTED_PRODUCT_ID  0x4CU
#define PAW3327_EXPECTED_INV_ID      0xB3U

#define PAW3327_BANK_0               0x00U
#define PAW3327_BANK_1               0x01U
#define PAW3327_BANK_3               0x03U

/* No explicit read/CS microsecond waits; writes retain a millisecond guard. */
#define PAW3327_WRITE_WAIT_MS        1U
#define PAW3327_RESET_WAIT_MS        50U
#define PAW3327_BURST_BYTES          6U

/* 手册第42页的分辨率编码 */
#define PAW3327_CPI_800              0x12U
#define PAW3327_CPI_1200             0x1BU
#define PAW3327_CPI_1500             0x21U
#define PAW3327_CPI_2400             0x36U
#define PAW3327_CPI_4800             0x6CU

#endif
