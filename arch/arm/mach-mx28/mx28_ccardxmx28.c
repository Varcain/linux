/*
 * Copyright (C) 2009-2011 Freescale Semiconductor, Inc. All Rights Reserved.
 * Copyright (C) 2011 Digi International, Inc. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/phy.h>
#include <linux/micrel_phy.h>

#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>

#include <mach/hardware.h>
#include <mach/device.h>
#include <mach/pinctrl.h>

#include <linux/spi/spi.h>
#include <linux/spi/spi_gpio.h>
#include <linux/spi/flash.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/physmap.h>

#include "device.h"
#include "mx28_ccardxmx28.h"
#include "mx28_pins.h"

static struct i2c_board_info __initdata mxs_i2c_device[] = {
#if defined(CONFIG_SND_MXS_SOC_CCARDXMX28) || defined(CONFIG_SND_MXS_SOC_CCARDXMX28_MODULE)
	{ I2C_BOARD_INFO("sgtl5000-i2c", 0xa), .flags = I2C_M_TEN },
#endif
#if defined(CONFIG_W1_MASTER_DS2482) || defined(CONFIG_W1_MASTER_DS2482_MODULE)
	{ I2C_BOARD_INFO("ds2482", 0x18), /*.flags = 0 */}
#endif
};

static void __init i2c_device_init(void)
{
	i2c_register_board_info(1, mxs_i2c_device, ARRAY_SIZE(mxs_i2c_device));
}
#if defined(CONFIG_MTD_M25P80) || defined(CONFIG_MTD_M25P80_MODULE)
static struct flash_platform_data mx28_spi_flash_data = {
	.name = "m25p80",
	.type = "w25x80",
};
#endif

#if defined(CONFIG_SPI_MXS) || defined(CONFIG_SPI_MXS_MODULE) || \
    defined(CONFIG_SPI_GPIO) || defined(CONFIG_SPI_GPIO_MODULE)
static struct spi_board_info spi_board_info[] __initdata = {
#if defined(CONFIG_SPI_MXS) || defined(CONFIG_SPI_MXS_MODULE)
	{
		.modalias = "spidev", /* use the generic spi driver */
		.max_speed_hz = 20000000,
		.bus_num = 3,        /* !!!you have to make sure master->bus_num (in spi_xms.c::mxs_spi_probe())is the same as this number!!! */
		.chip_select = 0,
		.platform_data = NULL, /* no data for this device */
	},
#endif
#if defined(CONFIG_MTD_M25P80) || defined(CONFIG_MTD_M25P80_MODULE)
	{
		/* the modalias must be the same as spi device driver name */
		.modalias = "m25p80", /* Name of spi_driver for this device */
		.max_speed_hz = 20000000,     /* max spi clock (SCK) speed in HZ */
		.bus_num = 1, /* Framework bus number */
		.chip_select = 0, /* Framework chip select. */
		.platform_data = &mx28_spi_flash_data,
	},
#endif
#if defined(CONFIG_SPI_GPIO) || defined(CONFIG_SPI_GPIO_MODULE)
	{
		.modalias = "spidev", 		/* Name of spi_driver for this device */
		.max_speed_hz = 20000000,     	/* max spi clock (SCK) speed in HZ */
		.bus_num = 3, 			/* Framework bus number */
		.chip_select = 0, 		/* Framework chip select. */
		.controller_data	= (void *) MXS_PIN_TO_GPIO(PINID_SSP3_SS0),
		.platform_data = NULL, 		/* no data for this device */
	},
#endif
};
#endif /* SPI || SPI_GPIO */

#if defined(CONFIG_SPI_GPIO) || defined(CONFIG_SPI_GPIO_MODULE)
static struct spi_gpio_platform_data spi_gpio_data = {
          .sck = MXS_PIN_TO_GPIO(PINID_SSP3_SCK),
          .mosi = MXS_PIN_TO_GPIO(PINID_SSP3_MOSI),
	  .miso = MXS_PIN_TO_GPIO(PINID_SSP3_MISO),
          .num_chipselect = 1,
};

static struct platform_device spi_gpio_device = {
	  .name      = "spi_gpio",
	  .id      = 3,   	/*This has to the same as .bus_num = 3, Framework bus number above */
	  .dev      = {
	    .platform_data   = &spi_gpio_data,
	  },
};
#endif

static void spi_device_init(void)
{
#if defined(CONFIG_SPI_MXS) || defined(CONFIG_SPI_MXS_MODULE)
      spi_register_board_info(spi_board_info, ARRAY_SIZE(spi_board_info));
#endif
#if defined(CONFIG_SPI_GPIO) || defined(CONFIG_SPI_GPIO_MODULE)
      spi_register_board_info(spi_board_info, ARRAY_SIZE(spi_board_info));
      platform_device_register(&spi_gpio_device);
#endif
}

static void __init fixup_board(struct machine_desc *desc, struct tag *tags,
			       char **cmdline, struct meminfo *mi)
{
	mx28_set_input_clk(24000000, 24000000, 32000, 50000000);
}

#if defined(CONFIG_LEDS_MXS) || defined(CONFIG_LEDS_MXS_MODULE)
static struct mxs_pwm_led  mx28_ccardxmx28_led_pwm[2] = {
	[0] = {
		.name = "led-pwm0",
		.pwm = 0,
		},
	[1] = {
		.name = "led-pwm1",
		.pwm = 1,
		},
};

struct mxs_pwm_leds_plat_data mx28_ccardxmx28_led_data = {
	.num = ARRAY_SIZE(mx28_ccardxmx28_led_pwm),
	.leds = mx28_ccardxmx28_led_pwm,
};

static struct resource mx28_ccardxmx28_led_res = {
	.flags = IORESOURCE_MEM,
	.start = PWM_PHYS_ADDR,
	.end   = PWM_PHYS_ADDR + 0x3FFF,
};

static void __init mx28_ccardxmx28_init_pwm_leds(void)
{
	struct platform_device *pdev;

	pdev = mxs_get_device("mxs-leds", 0);
	if (pdev == NULL || IS_ERR(pdev))
		return;

	pdev->resource = &mx28_ccardxmx28_led_res;
	pdev->num_resources = 1;
	pdev->dev.platform_data = &mx28_ccardxmx28_led_data;
	mxs_add_device(pdev, 3);
}
#else
static void __init mx28_ccardxmx28_init_pwm_leds(void)
{
	;
}
#endif

static void __init mx28_ccardxmx28_device_init(void)
{
	/* Add mx28_ccardxmx28 special code */
	i2c_device_init();
	spi_device_init();
	mx28_ccardxmx28_init_pwm_leds();
}

static int ksz8031_phy_fixup(struct phy_device *phydev)
{
	phydev->dev_flags |= MICREL_PHY_50MHZ_CLK;

	return 0;
}

static void __init mx28_ccardxmx28_init_machine(void)
{
	/* Setup hwid information, passed through Serial ATAG */
	ccardxmx28_set_mod_variant(system_serial_low & 0xff);
	ccardxmx28_set_mod_version((system_serial_low >> 8) & 0xff);
	ccardxmx28_set_mod_sn(((system_serial_low << 8) & 0xff000000) |
			      ((system_serial_low >> 8) & 0x00ff0000) |
			      ((system_serial_high << 8) & 0x0000ff00) |
			      ((system_serial_high >> 8) & 0xff));

	mx28_pinctrl_init();
	/* Init iram allocate */
#ifdef CONFIG_VECTORS_PHY_ADDR
	/* reserve the first page for irq vector table*/
	iram_init(MX28_OCRAM_PHBASE + PAGE_SIZE, MX28_OCRAM_SIZE - PAGE_SIZE);
#else
	iram_init(MX28_OCRAM_PHBASE, MX28_OCRAM_SIZE);
#endif
	mx28_gpio_init();
	mx28_ccardxmx28_pins_init();
	mx28_device_init();
	mx28_ccardxmx28_device_init();

	ccardxmx28_create_sysfs_entries();

	/* Register the fixup for the PHYs */
	phy_register_fixup_for_uid(PHY_ID_KSZ8031, MICREL_PHY_ID_MASK, ksz8031_phy_fixup);

	/* init the system revision */
	system_rev = 0x28011;
}

#if defined(CONFIG_MACH_CCARDWMX28JS)
MACHINE_START(CCARDWMX28JS, "ConnectCard Wi-i.MX28")
	.phys_io	= 0x80000000,
	.io_pg_offst	= ((0xf0000000) >> 18) & 0xfffc,
	.boot_params	= 0x40000100,
	.fixup		= fixup_board,
	.map_io		= mx28_map_io,
	.init_irq	= mx28_irq_init,
	.init_machine	= mx28_ccardxmx28_init_machine,
	.timer		= &mx28_timer.timer,
MACHINE_END
#endif

#if defined(CONFIG_MACH_CCARDMX28JS)
MACHINE_START(CCARDMX28JS, "ConnectCard i.MX28")
	.phys_io	= 0x80000000,
	.io_pg_offst	= ((0xf0000000) >> 18) & 0xfffc,
	.boot_params	= 0x40000100,
	.fixup		= fixup_board,
	.map_io		= mx28_map_io,
	.init_irq	= mx28_irq_init,
	.init_machine	= mx28_ccardxmx28_init_machine,
	.timer		= &mx28_timer.timer,
MACHINE_END
#endif
