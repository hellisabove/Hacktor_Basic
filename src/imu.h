#ifndef IMU_H_
#define IMU_H_

#include <stdint.h>
#include <stdbool.h>

#define IMU_I2C_ADDR 0x6A

// Register map constants
#define REG_WHO_AM_I        0x0F
#define REG_CTRL1_XL        0x10
#define REG_CTRL3_C         0x12
#define REG_CTRL10_C        0x19
#define REG_FUNC_CFG_ACCESS 0x01
#define REG_EMB_FUNC_EN_A   0x04
#define REG_EMB_FUNC_EN_B   0x05
#define REG_MD1_CFG         0x5E
#define REG_MD2_CFG         0x5F
#define REG_FUNC_SRC        0x53
#define REG_TILT_SRC        0x52
#define REG_STEP_COUNTER_L  0x4B
#define REG_PEDO_DEB_REG    0x2F
#define REG_WAKE_UP_THS     0x5B
#define REG_WAKE_UP_DUR     0x5C

// Bit definitions
#define CTRL3_C_BDU         (1 << 6)
#define CTRL3_C_IF_INC      (1 << 2)
#define CTRL3_C_SW_RESET    (1 << 0)
#define EMB_PEDO_EN_A_BIT   0x40
#define EMB_STEP_DET_EN_B   0x80
#define EMB_TILT_EN_B       0x20
#define MD1_INT1_STEP_DET   0x08
#define MD2_INT2_TILT       0x02

void imu_init(void);
uint8_t imu_read8(uint8_t reg);
void imu_write8(uint8_t reg, uint8_t val);
bool imu_read16(uint8_t reg, uint16_t *out);
bool imu_wait_who_am_i(uint16_t timeout_ms);
void imu_soft_reset(void);
bool imu_enable_hardware_pedometer(uint16_t *initial_step_count);
bool imu_enable_tilt_on_int2(void);
void imu_set_accel_odr(uint8_t odr_bits);

#endif /* IMU_H_ */
