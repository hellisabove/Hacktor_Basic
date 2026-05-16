#include "imu.h"
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(hacktor, LOG_LEVEL_INF);

#define IMU_NODE DT_NODELABEL(imu)
static const struct device *imu_dev = DEVICE_DT_GET(IMU_NODE);
static const struct device *i2c_dev = DEVICE_DT_GET(DT_BUS(IMU_NODE));

void imu_init(void) {
    if (!device_is_ready(imu_dev)) {
        LOG_ERR("IMU sensor device not ready");
    }
}

uint8_t imu_read8(uint8_t reg) {
    uint8_t val = 0;
    i2c_reg_read_byte(i2c_dev, IMU_I2C_ADDR, reg, &val);
    return val;
}

void imu_write8(uint8_t reg, uint8_t val) {
    i2c_reg_write_byte(i2c_dev, IMU_I2C_ADDR, reg, val);
}

bool imu_read16(uint8_t reg, uint16_t *out) {
    uint8_t buf[2];
    if (i2c_burst_read(i2c_dev, IMU_I2C_ADDR, reg, buf, 2) != 0) {
        return false;
    }
    *out = (uint16_t)(buf[0] | (buf[1] << 8));
    return true;
}

bool imu_wait_who_am_i(uint16_t timeout_ms) {
    // Zephyr driver handles this, but for app logic:
    return device_is_ready(imu_dev);
}

void imu_soft_reset(void) {
    // Zephyr driver handles this at boot
}

bool imu_enable_hardware_pedometer(uint16_t *initial_step_count) {
    *initial_step_count = 0;
    
    // Set ODR using Sensor API if possible, or direct I2C
    struct sensor_value odr = { .val1 = 104, .val2 = 0 };
    sensor_attr_set(imu_dev, SENSOR_CHAN_ACCEL_XYZ, SENSOR_ATTR_SAMPLING_FREQUENCY, &odr);

    // Embedded functions setup (manual I2C as driver doesn't support)
    imu_write8(REG_FUNC_CFG_ACCESS, 0x80);
    imu_write8(REG_EMB_FUNC_EN_A, imu_read8(REG_EMB_FUNC_EN_A) | EMB_PEDO_EN_A_BIT);
    imu_write8(REG_EMB_FUNC_EN_B, imu_read8(REG_EMB_FUNC_EN_B) | EMB_STEP_DET_EN_B);
    imu_write8(REG_FUNC_CFG_ACCESS, 0x00);
    imu_write8(REG_CTRL10_C, 0x3C);
    imu_write8(REG_FUNC_CFG_ACCESS, 0x80);
    imu_write8(REG_PEDO_DEB_REG, imu_read8(REG_PEDO_DEB_REG) | 0x02);
    imu_write8(REG_FUNC_CFG_ACCESS, 0x00);
    imu_write8(REG_MD1_CFG, MD1_INT1_STEP_DET);
    (void)imu_read8(REG_FUNC_SRC);
    
    uint16_t s16;
    if (imu_read16(REG_STEP_COUNTER_L, &s16)) {
        *initial_step_count = s16;
    }
    return true;
}

bool imu_enable_tilt_on_int2(void) {
    imu_write8(REG_FUNC_CFG_ACCESS, 0x80);
    uint8_t enB = imu_read8(REG_EMB_FUNC_EN_B) | EMB_TILT_EN_B;
    imu_write8(REG_EMB_FUNC_EN_B, enB);
    imu_write8(REG_FUNC_CFG_ACCESS, 0x00);

    imu_write8(REG_WAKE_UP_THS, 0x04);
    imu_write8(REG_WAKE_UP_DUR, 0x00);

    imu_write8(REG_MD2_CFG, MD2_INT2_TILT);

    (void)imu_read8(REG_TILT_SRC);
    (void)imu_read8(REG_FUNC_SRC);

    return true;
}

void imu_set_accel_odr(uint8_t odr_bits) {
    // Use Sensor API
    uint32_t freq = 0;
    switch(odr_bits & 0xF0) {
        case 0x10: freq = 13; break;
        case 0x20: freq = 26; break;
        case 0x30: freq = 52; break;
        case 0x40: freq = 104; break;
        case 0x50: freq = 208; break;
        default: break;
    }
    if (freq > 0) {
        struct sensor_value odr = { .val1 = freq, .val2 = 0 };
        sensor_attr_set(imu_dev, SENSOR_CHAN_ACCEL_XYZ, SENSOR_ATTR_SAMPLING_FREQUENCY, &odr);
    }
}
