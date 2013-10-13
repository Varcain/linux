/*
  * Copyright 2010 Digi International, Inc. All Rights Reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

#ifndef DEVICES_CCWMX51_H_
#define DEVICES_CCWMX51_H_

#include "linux/fsl_devices.h"

#if defined(CONFIG_JSCCWMX51_V2)
#define DIO0_POWER_KEY				75
#define DIO1_SEARCH_KEY				82
#define DIO3_POWER_DELAYED_KEY		74
#define DIO4_HOME_KEY				84
#define DIO5_MENU_KEY				85
#define DIO6_BACK_KEY				86
#define USER_KEY1					70
#define USER_KEY2					1
#else
#define DIO0_POWER_KEY				69
#define DIO1_SEARCH_KEY				70
#define DIO3_POWER_DELAYED_KEY		72
#define DIO4_HOME_KEY				84
#define DIO5_MENU_KEY				85
#define DIO6_BACK_KEY				86
#define USER_KEY1					8
#define USER_KEY2					1
#endif


extern struct flash_platform_data mxc_nand_data;
extern struct smc911x_platdata ccwmx51_smsc9118;
extern struct mxc_mmc_platform_data mmc1_data ;
extern struct mxc_mmc_platform_data mmc3_data;
extern struct resource mxcfb_resources[];
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
extern struct mxc_spi_master mxcspi1_data ;
extern struct mxc_spi_master mxcspi2_data ;
extern struct mxc_spi_master mxcspi3_data ;
extern struct mxc_ipu_config mxc_ipu_data;
extern struct mxc_vpu_platform_data mxc_vpu_data;
extern struct mxc_w1_config mxc_w1_data;
extern struct mxc_spdif_platform_data mxc_spdif_data;
extern struct tve_platform_data tve_data;
extern struct mxc_bus_freq_platform_data bus_freq_data;
extern struct platform_pwm_backlight_data mxc_pwm_backlight_data;
extern struct mxc_audio_platform_data wm8753_data;
extern struct mxc_fb_platform_data mx51_fb_data[];
extern struct uio_info gpu2d_platform_data;
extern struct ccwmx5x_lcd_pdata plcd_platform_data[2];
extern struct fsl_ata_platform_data ata_data;
extern int __init ccwmx51_init_i2c2(void);
extern void ccwmx51_init_spidevices(void);
extern int __init ccwmx51_init_fb(void);
extern void __init ccwmx51_io_init(void);
extern int __init ccwmx51_init_mc13892(void);
extern struct platform_device smsc911x_device;
extern void ccwmx51_register_sdio(int interface);
extern void ccwmx51_init_devices(void);

extern struct mxc_iim_data iim_data;
extern struct gpio_keys_platform_data ccwmx51js_gpio_key_info;
extern struct platform_device ccwmx51js_keys_gpio;

#endif /* DEVICES_CCWMX51_H_ */
