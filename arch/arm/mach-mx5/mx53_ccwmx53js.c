/*
 * Copyright (C) 2010-2011 Freescale Semiconductor, Inc. All Rights Reserved.
 * Copyright (C) 2011 Digi International, Inc. All Rights Reserved.
 */

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <linux/types.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/nodemask.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/fsl_devices.h>
#include <linux/spi/spi.h>
#include <linux/i2c.h>
#include <linux/ata.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>
#include <linux/regulator/consumer.h>
#include <linux/android_pmem.h>
#include <linux/usb/android_composite.h>
#include <linux/usb/f_accessory.h>
#include <linux/pmic_external.h>
#include <linux/pmic_status.h>
#include <linux/ipu.h>
#include <linux/mxcfb.h>
#include <linux/pwm_backlight.h>
#include <linux/ahci_platform.h>
#include <linux/gpio_keys.h>
#include <linux/mfd/da9052/da9052.h>
#include <mach/common.h>
#include <mach/hardware.h>
#include <asm/irq.h>
#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/time.h>
#include <asm/mach/keypad.h>
#include <asm/mach/flash.h>
#include <mach/memory.h>
#include <mach/gpio.h>
#include <mach/mmc.h>
#include <mach/mxc_dvfs.h>
#include <mach/iomux-mx53.h>
#include <mach/i2c.h>
#include <mach/mxc_iim.h>
#include <mach/iomux-mx53.h>
#include <mach/check_fuse.h>

#include "devices_ccwmx53.h"
#include "devices_ccxmx5x.h"
#include "crm_regs.h"
#include "devices.h"
#include "usb.h"
#include "board-ccwmx53.h"
#include "linux/fsl_devices.h"

#if defined(CONFIG_FB_MXC_SYNC_PANEL) || defined(CONFIG_FB_MXC_SYNC_PANEL_MODULE)
#include "bl_shared_ram.h"
extern void ccwmx53_set_lcd_struct_from_bl_shared(nv_lcd_config_t *lcd_config);
#endif

#define TZIC_WAKEUP0_OFFSET	(0x0E00)
#define TZIC_WAKEUP1_OFFSET	(0x0E04)
#define TZIC_WAKEUP2_OFFSET	(0x0E08)
#define TZIC_WAKEUP3_OFFSET	(0x0E0C)
#define GPIO7_0_11_IRQ_BIT	(0x1<<11)

/*!
 * @file mach-mx5/mx53_ccwmx53js.c
 *
 * @brief This file contains MX53 ccwmx53js board specific initialization routines.
 *
 * @ingroup MSL_MX53
 */

/* MX53 LOCO GPIO PIN configurations */
#define MX53_nONKEY			(0*32 + 8)	/* GPIO_1_8 */

u8 ccwmx51_swap_bi = 1;

extern int __init mx53_ccwmx53js_init_da9052(void);
extern void gpio_dio_active(void);
extern void gpio_usb_active(void);
extern void pm_i2c_init(u32 base_addr);
extern int ccxmx53_pm_da9053_mask_irqs(void);
extern int ccxmx53_pm_da9053_unmask_irqs(void);

static iomux_v3_cfg_t mx53_ccwmx53js_pads[] = {
	/* I2C3, connected to the DA9053 and MMA7455  */
	MX53_PAD_GPIO_5__I2C3_SCL,
	MX53_PAD_GPIO_6__I2C3_SDA,
};

static iomux_v3_cfg_t ccwmx53js_keys_leds_pads[] = {
	USER_LED1_PAD,
	USER_LED2_PAD,
	USER_KEY1_PAD,
	USER_KEY2_PAD,
};

extern void mx5_ipu_reset(void);
static struct mxc_ipu_config mxc_ipu_data = {
	.rev = 3,
	.reset = mx5_ipu_reset,
};

extern void mx5_vpu_reset(void);
static struct mxc_vpu_platform_data mxc_vpu_data = {
#ifdef CONFIG_MXC_VPU_IRAM
	.iram_enable = true,
	.iram_size = CONFIG_MXC_VPU_IRAM_SIZE,
#else
	.iram_enable = false,
#endif
	.reset = mx5_vpu_reset,
};

static struct mxc_dvfs_platform_data dvfs_core_data = {
	.reg_id = "DA9052_BUCK_CORE",
	.clk1_id = "cpu_clk",
	.clk2_id = "gpc_dvfs_clk",
	.gpc_cntr_offset = MXC_GPC_CNTR_OFFSET,
	.gpc_vcr_offset = MXC_GPC_VCR_OFFSET,
	.ccm_cdcr_offset = MXC_CCM_CDCR_OFFSET,
	.ccm_cacrr_offset = MXC_CCM_CACRR_OFFSET,
	.ccm_cdhipr_offset = MXC_CCM_CDHIPR_OFFSET,
	.prediv_mask = 0x1F800,
	.prediv_offset = 11,
	.prediv_val = 3,
	.div3ck_mask = 0xE0000000,
	.div3ck_offset = 29,
	.div3ck_val = 2,
	.emac_val = 0x08,
	.upthr_val = 25,
	.dnthr_val = 9,
	.pncthr_val = 33,
	.upcnt_val = 10,
	.dncnt_val = 10,
	.delay_time = 30,
};

static struct mxc_bus_freq_platform_data bus_freq_data = {
	.gp_reg_id = "DA9052_BUCK_CORE",
	.lp_reg_id = "DA9052_BUCK_PRO",
};

#if defined CONFIG_ANDROID_PMEM
static struct android_pmem_platform_data android_pmem_data = {
	.name = "pmem_adsp",
	.size = SZ_64M,
	.cached = 1,
};

static struct android_pmem_platform_data android_pmem_gpu_data = {
	.name = "pmem_gpu",
	.size = SZ_64M,
	.cached = 1,
};
#endif

#ifdef CONFIG_USB_ANDROID
static char *usb_functions_ums[] = {
	"usb_mass_storage",
};

static char *usb_functions_ums_adb[] = {
	"usb_mass_storage",
	"adb",
};

static char *usb_functions_rndis[] = {
	"rndis",
};

static char *usb_functions_rndis_adb[] = {
	"rndis",
	"adb",
};

static char *usb_functions_accessory[] = {
	"accessory",
};

static char *usb_functions_accessory_adb[] = {
	"accessory",
	"adb",
};

static char *usb_functions_all[] = {
	"usb_mass_storage",
	"adb",
	"rndis",
	"accessory",
};

static struct android_usb_product usb_products[] = {
	{
		.product_id	= 0x0c01,
		.num_functions	= ARRAY_SIZE(usb_functions_ums),
		.functions	= usb_functions_ums,
	},
	{
		.product_id	= 0x0c02,
		.num_functions	= ARRAY_SIZE(usb_functions_ums_adb),
		.functions	= usb_functions_ums_adb,
	},
	{
		.product_id	= 0x0c10,
		.num_functions	= ARRAY_SIZE(usb_functions_rndis),
		.functions	= usb_functions_rndis,
	},
	{
		.vendor_id	= USB_ACCESSORY_VENDOR_ID,
		.product_id	= USB_ACCESSORY_PRODUCT_ID,
		.num_functions	= ARRAY_SIZE(usb_functions_accessory),
		.functions	= usb_functions_accessory,
	},
	{
		.vendor_id	= USB_ACCESSORY_VENDOR_ID,
		.product_id	= USB_ACCESSORY_ADB_PRODUCT_ID,
		.num_functions	= ARRAY_SIZE(usb_functions_accessory_adb),
		.functions	= usb_functions_accessory_adb,
	},
};

static struct usb_mass_storage_platform_data mass_storage_data = {
	.nluns		= 3,
	.vendor		= "Freescale",
	.product	= "MX53 SMD Android",
	.release	= 0x0100,
};

static struct usb_ether_platform_data rndis_data = {
	.vendorID	= 0x15a2,
	.vendorDescr	= "Freescale",
};

static struct android_usb_platform_data android_usb_data = {
	.vendor_id      = 0x15a2,
	.product_id     = 0x0c01,
	.version        = 0x0100,
	.product_name   = "MX53 SMD Android",
	.manufacturer_name = "Freescale",
	.num_products = ARRAY_SIZE(usb_products),
	.products = usb_products,
	.num_functions = ARRAY_SIZE(usb_functions_all),
	.functions = usb_functions_all,
};
#endif /* CONFIG_USB_ANDROID */

/*!
 * Board specific fixup function. It is called by \b setup_arch() in
 * setup.c file very early on during kernel starts. It allows the user to
 * statically fill in the proper values for the passed-in parameters. None of
 * the parameters is used currently.
 *
 * @param  desc         pointer to \b struct \b machine_desc
 * @param  tags         pointer to \b struct \b tag
 * @param  cmdline      pointer to the command line
 * @param  mi           pointer to \b struct \b meminfo
 */
static void __init fixup_mxc_board(struct machine_desc *desc, struct tag *tags,
				   char **cmdline, struct meminfo *mi)
{
#if defined CONFIG_ANDROID_PMEM
	char *str;
	struct tag *t = NULL;
	struct tag *mem_tag = 0;
	int total_mem = SZ_1G;
	int left_mem = 0, avali_mem = 0;
	int gpu_mem = SZ_64M;
	int pmem_gpu_size = android_pmem_gpu_data.size;
	int pmem_adsp_size = android_pmem_data.size;
	int count = 0;

	mxc_set_cpu_type(MXC_CPU_MX53);

	/* Get total mem from tags */
	for_each_tag(t, tags) {
		if (t->hdr.tag == ATAG_MEM) {
			total_mem = t->u.mem.size;
			mem_tag = t;
			/* Identify 1GB variants to adjust pmem */
			if( total_mem == SZ_512M ){
				/* If count is not zero, this is the second 512MB bank.
				 * Only 1GB variants have a second bank of 512MB.*/
				if(count)
					pmem_gpu_size = android_pmem_gpu_data.size = SZ_128M;
				count++;
			}
		}
	}

	/* get mem= and gpu_memory= from cmdline */
	for_each_tag(t, tags) {
		if (t->hdr.tag == ATAG_CMDLINE) {
			str = t->u.cmdline.cmdline;
			str = strstr(str, " mem=");
			if (str != NULL) {
				str += 5;
				avali_mem = memparse(str, &str);
			}

			str = t->u.cmdline.cmdline;
			str = strstr(str, "gpu_mmu");
			if (str != NULL)
				gpu_data.enable_mmu = 1;

			str = t->u.cmdline.cmdline;
			str = strstr(str, "gpu_memory=");
			if (str != NULL) {
				str += 11;
				gpu_mem = memparse(str, &str);
			}

			str = t->u.cmdline.cmdline;
			str = strstr(str, "pmem=");
			if (str != NULL) {
				str += 5;
				pmem_gpu_size = memparse(str, &str);
				android_pmem_gpu_data.size = pmem_gpu_size;
				if (*str == ',') {
					str++;
					pmem_adsp_size = memparse(str, &str);
					android_pmem_data.size = pmem_adsp_size;
				}
			}
			left_mem = total_mem - gpu_mem - pmem_gpu_size - pmem_adsp_size;
			break;
		}
	}

	if (gpu_data.enable_mmu)
		gpu_mem = 0;

	if (avali_mem > 0 && avali_mem < left_mem)
		left_mem = avali_mem;

	if (mem_tag) {
		android_pmem_data.start = mem_tag->u.mem.start
				+ left_mem + gpu_mem + pmem_gpu_size;
		android_pmem_gpu_data.start = mem_tag->u.mem.start
				+ left_mem + gpu_mem;
		mem_tag->u.mem.size = left_mem;

		/*reserve memory for gpu*/
		if (!gpu_data.enable_mmu) {
			gpu_device.resource[5].start =
				mem_tag->u.mem.start + left_mem;
			gpu_device.resource[5].end =
				gpu_device.resource[5].start + gpu_mem - 1;
		}
	}
#else /* CONFIG_ANDROID_PMEM */
	struct tag *t;
	struct tag *mem_tag = 0;
	int total_mem = SZ_512M;
	int left_mem = 0;
	int gpu_mem = SZ_64M;
	int fb_mem = FB_MEM_SIZE;
	char *str;

	mxc_set_cpu_type(MXC_CPU_MX53);

	for_each_tag(mem_tag, tags) {
		if (mem_tag->hdr.tag == ATAG_MEM) {
			total_mem = mem_tag->u.mem.size;
			break;
		}
	}

	for_each_tag(t, tags) {
		if (t->hdr.tag == ATAG_CMDLINE) {
			str = t->u.cmdline.cmdline;
			str = strstr(str, "mem=");
			if (str != NULL) {
				str += 4;
				left_mem = memparse(str, &str);
			}
			str = t->u.cmdline.cmdline;
			str = strstr(str, "gpu_mmu");
			if (str != NULL)
				gpu_data.enable_mmu = 1;

			str = t->u.cmdline.cmdline;
			str = strstr(str, "gpu_memory=");
			if (str != NULL) {
				str += 11;
				gpu_mem = memparse(str, &str);
			}

			break;
		}
	}

	if (gpu_data.enable_mmu)
		gpu_mem = 0;

	if (left_mem == 0 || left_mem > total_mem)
		left_mem = total_mem - gpu_mem - fb_mem;

	if (mem_tag) {
		fb_mem = total_mem - left_mem - gpu_mem;
		if (fb_mem < 0) {
			gpu_mem = total_mem - left_mem;
			fb_mem = 0;
		}
		mem_tag->u.mem.size = left_mem;
		/*reserve memory for gpu*/
		if (!gpu_data.enable_mmu) {
			gpu_device.resource[5].start =
				mem_tag->u.mem.start + left_mem;
			gpu_device.resource[5].end =
				gpu_device.resource[5].start + gpu_mem - 1;
		}
#if defined(CONFIG_FB_MXC_SYNC_PANEL) || \
	defined(CONFIG_FB_MXC_SYNC_PANEL_MODULE)
		if (fb_mem) {
#if defined(CONFIG_CCXMX5X_DISP0) && defined(CONFIG_CCXMX5X_DISP1)
			fb_mem = fb_mem / 2;	/* Divide the mem for between the displays */
#endif
			mxcfb_resources[0].start =
				gpu_data.enable_mmu ?
				mem_tag->u.mem.start + left_mem :
				gpu_device.resource[5].end + 1;
			mxcfb_resources[0].end =
				mxcfb_resources[0].start + fb_mem - 1;
#if defined(CONFIG_CCXMX5X_DISP0) && defined(CONFIG_CCXMX5X_DISP1)
			mxcfb_resources[1].start =
				mxcfb_resources[0].end + 1;
			mxcfb_resources[1].end =
				mxcfb_resources[1].start + fb_mem - 1;
#endif
		} else {
			mxcfb_resources[0].start = 0;
			mxcfb_resources[0].end = 0;
			mxcfb_resources[1].start = 0;
			mxcfb_resources[1].end = 0;
		}
#endif
	}
#endif /* CONFIG_ANDROID_PMEM */
#if defined(CONFIG_FB_MXC_SYNC_PANEL) || \
       defined(CONFIG_FB_MXC_SYNC_PANEL_MODULE)
	ccwmx53_set_lcd_struct_from_bl_shared(
		(nv_lcd_config_t *)((unsigned int)desc->boot_params + BL_SHARED_RAM_OFFS_LCD));
#endif
}

static void __init mx53_ccwmx53js_io_init(void)
{
	mxc_iomux_v3_setup_multiple_pads(mx53_ccwmx53js_pads,
					ARRAY_SIZE(mx53_ccwmx53js_pads));
	mxc_iomux_v3_setup_multiple_pads(ccwmx53js_keys_leds_pads,
					 ARRAY_SIZE(ccwmx53js_keys_leds_pads));
	gpio_wireless_active();
	gpio_dio_active();
	gpio_usb_active();
}

static void ccxmx53_suspend_enter(void)
{
	ccxmx53_pm_da9053_mask_irqs();
}

static void ccxmx53_suspend_exit(void)
{
	ccxmx53_pm_da9053_unmask_irqs();
}

static struct mxc_pm_platform_data ccwmx53_pm_data = {
	.suspend_enter = ccxmx53_suspend_enter,
	.suspend_exit = ccxmx53_suspend_exit,
};

#if defined(CONFIG_I2C_IMX_RECOVER_FROM_STUCK)
#define I2C_RECOVERY_MAX_CLOKS		50
/* Generate a pulse on the i2c clock pin. */
void i2c_recover_from_stuck(void)
{
	int i = 0, i2c_ready = 1;

	gpio_request(CCWMX53_I2C3_SCL_GPIO, "i2c-2_clk");
	gpio_request(CCWMX53_I2C3_SDA_GPIO, "i2c-2_sda");

	/* Reconfigure pins as gpios */
	gpio_direction_input(CCWMX53_I2C3_SCL_GPIO);
	mxc_iomux_v3_setup_pad(MX53_PAD_GPIO_5__GPIO1_5);
	gpio_direction_input(CCWMX53_I2C3_SDA_GPIO);
	mxc_iomux_v3_setup_pad(MX53_PAD_GPIO_6__GPIO1_6);

	/* First of all, verify that both i2c lines are ready (pulled up) */
	if (gpio_get_value(CCWMX53_I2C3_SCL_GPIO) != 1)
		i2c_ready = 0;

	if (gpio_get_value(CCWMX53_I2C3_SDA_GPIO) != 1)
		i2c_ready = 0;

	/* If not, try to recover by clocking the bus (bitbanging) */
	if (!i2c_ready) {
		for (i = 0; i < I2C_RECOVERY_MAX_CLOKS; i++) {
			if (gpio_get_value(CCWMX53_I2C3_SDA_GPIO) == 1)
				break;

			gpio_direction_output(CCWMX53_I2C3_SCL_GPIO, 0);
			udelay(50);
			gpio_direction_input(CCWMX53_I2C3_SCL_GPIO);
			udelay(50);
		}
	}

	if (!i2c_ready && i != I2C_RECOVERY_MAX_CLOKS) {
		printk(KERN_INFO "%s: successfully recovered!\n", __func__);
	}

	/* Free gpios adn go back to i2c operational mode */
	gpio_free(CCWMX53_I2C3_SCL_GPIO);
	gpio_free(CCWMX53_I2C3_SDA_GPIO);
	mxc_iomux_v3_setup_pad(MX53_PAD_GPIO_5__I2C3_SCL);
	mxc_iomux_v3_setup_pad(MX53_PAD_GPIO_6__I2C3_SDA);
}
#endif /* CONFIG_I2C_IMX_RECOVER_FROM_STUCK */

/*!
 * Board specific initialization.
 */
static void __init mxc_board_init(void)
{
	/* Setup hwid information, passed through Serial ATAG */
	ccxmx5x_set_mod_variant(system_serial_low & 0xff);
	ccxmx5x_set_mod_revision((system_serial_low >> 8) & 0xff);
	ccxmx5x_set_mod_sn(((system_serial_low << 8) & 0xff000000) |
			   ((system_serial_low >> 8) & 0x00ff0000) |
			   ((system_serial_high << 8) & 0x0000ff00) |
			   ((system_serial_high >> 8) & 0xff));

	mxc_ipu_data.di_clk[0] = clk_get(NULL, "ipu_di0_clk");
	mxc_ipu_data.di_clk[1] = clk_get(NULL, "ipu_di1_clk");
	mxc_ipu_data.csi_clk[0] = clk_get(NULL, "ssi_ext1_clk");
	mxc_ipu_data.csi_clk[1] = clk_get(NULL, "ssi_ext1_clk");

	mxc_cpu_common_init();
	mx53_ccwmx53js_io_init();

	mxc_register_device(&mxc_dma_device, NULL);
	mxc_register_device(&mxc_wdt_device, NULL);
	mxc_register_device(&mxci2c_devices[2], &mxci2c3_data);

	mx53_ccwmx53js_init_da9052();

	mxc_register_device(&mxc_ipu_device, &mxc_ipu_data);
	if (!mxc_fuse_get_vpu_status())
		mxc_register_device(&mxcvpu_device, &mxc_vpu_data);
	if (!mxc_fuse_get_gpu_status())
		mxc_register_device(&gpu_device, &gpu_data);
	mxc_register_device(&mxcscc_device, NULL);
	mxc_register_device(&pm_device, &ccwmx53_pm_data);
#if defined(CONFIG_PMIC_DA9052)
	mxc_register_device(&mxc_dvfs_core_device, &dvfs_core_data);
#endif
	mxc_register_device(&busfreq_device, &bus_freq_data);
	mxc_register_device(&mxc_iim_device, &iim_data);
	mxc_register_device(&mxc_v4l2_device, NULL);
	mxc_register_device(&mxc_v4l2out_device, NULL);
#if defined(CONFIG_SPI_MXC_SELECT1) || defined(CONFIG_SPI_MXC_SELECT1_MODULE)
	mxc_register_device(&mxcspi1_device, &mxcspi1_data);
#endif
	ccwmx53_init_spidevices();
//	mxc_register_device(&mxc_ssi1_device, NULL);
	mxc_register_device(&mxc_ssi2_device, NULL);

#ifdef CONFIG_ANDROID_PMEM
	mxc_register_device(&mxc_android_pmem_device, &android_pmem_data);
	mxc_register_device(&mxc_android_pmem_gpu_device,
				&android_pmem_gpu_data);
#endif
#ifdef CONFIG_USB_ANDROID
	mxc_register_device(&usb_mass_storage_device, &mass_storage_data);
	mxc_register_device(&usb_rndis_device, &rndis_data);
	mxc_register_device(&android_usb_device, &android_usb_data);
#endif

	ccwmx53_register_sdio(1);
#ifdef CONFIG_ESDHCI_MXC_SELECT2
	ccwmx53_register_sdio(2);
#endif
#ifdef CONFIG_ESDHCI_MXC_SELECT3
	ccwmx53_register_sdio(3);
#endif
#ifdef CONFIG_ESDHCI_MXC_SELECT4
	ccwmx53_register_sdio(4);
#endif
	ccwmx53_register_nand();
	ccxmx53_register_sata();
	ccwmx53_register_fec();
	ccwmx53_register_ext_eth();
	ccwmx53_register_sgtl5000();
	ccwmx5x_init_fb();
	ccwmx53_init_i2c_devices();
	ccwmx53_register_can(0);
	ccwmx53_register_can(1);

	mx5_usb_dr_init();
#if defined(CONFIG_USB_EHCI_ARC_H1) || defined(CONFIG_USB_EHCI_ARC_H1_MODULE)
	mx5_usbh1_init();
#endif

#ifdef CONFIG_CCWMX5X_SECOND_TOUCH
	ccwmx53_init_2nd_touch();
#endif
	ccwmx53_register_fusion_touch();

#if defined(CONFIG_KEYBOARD_GPIO)
	mxc_register_device(&ccwmx53js_keys_gpio, &ccwmx53js_gpio_key_info);
#endif

	ccxmx5x_create_sysfs_entries();

#if defined (CONFIG_PMIC_DA9052)
	pm_power_off = da9053_power_off;
#endif
	pm_i2c_init(I2C3_BASE_ADDR - MX53_OFFSET);
}

static void __init mx53_ccwmx53js_timer_init(void)
{
	struct clk *uart_clk;

	mx53_clocks_init(32768, 24000000, 0, 0);

	uart_clk = clk_get_sys("mxcintuart.0", NULL);

	if (CONSOLE_UART_BASE_ADDR)
		early_console_setup(MX53_BASE_ADDR(CONSOLE_UART_BASE_ADDR), uart_clk);
}

static struct sys_timer mxc_timer = {
	.init	= mx53_ccwmx53js_timer_init,
};

/*
 * The following uses standard kernel macros define in arch.h in order to
 * initialize __mach_desc_CCWMX53 data structure.
 */

#if defined(CONFIG_MACH_CCWMX53JS)
MACHINE_START(CCWMX53JS, "ConnectCore Wi-i.MX53"BOARD_NAME)
	/* Maintainer: Digi International, Inc. */
	.boot_params	= PHYS_OFFSET + 0x100,
	.fixup = fixup_mxc_board,
	.map_io = mx5_map_io,
	.init_irq = mx5_init_irq,
	.init_machine = mxc_board_init,
	.timer = &mxc_timer,
MACHINE_END
#endif /* CONFIG_MACH_CCWMX53JS */

#if defined(CONFIG_MACH_CCMX53JS)
MACHINE_START(CCMX53JS, "ConnectCore i.MX53"BOARD_NAME)
	/* Maintainer: Digi International, Inc. */
	.boot_params	= PHYS_OFFSET + 0x100,
	.fixup = fixup_mxc_board,
	.map_io = mx5_map_io,
	.init_irq = mx5_init_irq,
	.init_machine = mxc_board_init,
	.timer = &mxc_timer,
MACHINE_END
#endif /* CONFIG_MACH_CCMX53JS */
