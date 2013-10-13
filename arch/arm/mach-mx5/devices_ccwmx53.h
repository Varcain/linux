/*
  * Copyright 2011 Digi International, Inc. All Rights Reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

#ifndef __DEVICES_CCWMX53_H_
#define __DEVICES_CCWMX53_H_

#include "linux/fsl_devices.h"

#define DIO0_POWER_KEY				175
//#define DIO1_SEARCH_KEY				82
//#define DIO3_POWER_DELAYED_KEY		74
#define DIO4_HOME_KEY				198
#define DIO5_MENU_KEY				176
#define DIO6_BACK_KEY				174
#define USER_KEY1					96
#define USER_KEY2					97

extern struct resource mxcfb_resources[];
extern struct mxc_spi_master mxcspi1_data ;
#if defined(CONFIG_I2C_MXC) || defined(CONFIG_I2C_MXC_MODULE)
extern struct mxc_i2c_platform_data mxci2c1_data;
extern struct mxc_i2c_platform_data mxci2c2_data;
extern struct mxc_i2c_platform_data mxci2c3_data;
extern struct mxc_i2c_platform_data mxci2c_hs_data;
#elif defined(CONFIG_I2C_IMX) || defined(CONFIG_I2C_IMX_MODULE)
extern struct imxi2c_platform_data mxci2c1_data;
extern struct imxi2c_platform_data mxci2c2_data;
extern struct imxi2c_platform_data mxci2c3_data;
#endif
extern struct mxc_iim_data iim_data;

void ccwmx53_register_sdio(int interface);
void ccwmx53_register_nand(void);
void ccwmx53_register_ext_eth(void);
void ccwmx53_register_sgtl5000(void);
void ccwmx53_register_fec(void);
void ccwmx53_register_can(int interface);
int __init ccwmx5x_init_fb(void);
int __init ccwmx53_init_i2c_devices(void);
extern void ccwmx53_init_spidevices(void);
void ccxmx53_register_sata(void);
void ccwmx53_register_fusion_touch(void);

void gpio_smsc911x_active(void);
void gpio_sdhc_active(int interface);
void gpio_fec_active(void);
void gpio_can_active(int interface);
void ccxmx53_user_led(int led, int val);
extern struct gpio_keys_platform_data ccwmx53js_gpio_key_info;
extern struct platform_device ccwmx53js_keys_gpio;
void ccxmx53_user_led(int led, int val);
extern void i2c_recover_from_stuck(void);

/* framebuffer settings */
#if defined(CONFIG_CCXMX5X_DISP0) && defined(CONFIG_CCXMX5X_DISP1)
#define FB_MEM_SIZE             (SZ_1M * 48)    /* 1920x1080x32bpp x 3 buffers x 2 interfaces*/
#else
#define FB_MEM_SIZE             (SZ_1M * 24)    /* 1920x1080x32bpp x 3 buffers */
#endif

#define MX53_GPIO(port,pin)		((port - 1) * 32 + pin)
#define CCWMX53_EXT_IRQ_GPIO		MX53_GPIO(4,2)		/* GPIO_4_2 */
#define SECOND_TS_IRQ_PIN		MX53_GPIO(5,28)		/* GPIO_5_28 */
#define SECOND_TS_SPI_SS_PIN		MX53_GPIO(5,25)		/* GPIO_5_25 */
#define CCWMX53_MMA7455_IRQ_GPIO	MX53_GPIO(4,4)		/* GPIO_4_4 */
#define CCWMX53_DA9052_IRQ_GPIO		MX53_GPIO(7,11)		/* GPIO7_11 */
#define CCWMX53_I2C3_SCL_GPIO		MX53_GPIO(1,5)		/* GPIO1_5 */
#define CCWMX53_I2C3_SDA_GPIO		MX53_GPIO(1,6)		/* GPIO1_6 */

#endif /* __DEVICES_CCWMX53_H_ */
