#pragma once

/* The OLED sits on I2C1 — D4/D5 on the XIAO are GP6/GP7. */
#define I2C_DRIVER I2CD1
#define I2C1_SDA_PIN GP6
#define I2C1_SCL_PIN GP7

#define OLED_DISPLAY_128X32
