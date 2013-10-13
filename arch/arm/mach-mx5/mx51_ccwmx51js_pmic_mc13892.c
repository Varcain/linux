 /*
 * Copyright 2009-2011 Freescale Semiconductor, Inc. All Rights Reserved.
 * Copyright 2009-2011 Digi International, Inc. All Rights Reserved.
  */

 /*
  * The code contained herein is licensed under the GNU General Public
  * License. You may obtain a copy of the GNU General Public License
  * Version 2 or later at the following locations:
  *
  * http://www.opensource.org/licenses/gpl-license.html
  * http://www.gnu.org/copyleft/gpl.html
  */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/err.h>
#include <linux/pmic_external.h>
#include <linux/regulator/machine.h>
#include <linux/mfd/mc13892/core.h>
#include <linux/delay.h>
#include <asm/mach-types.h>
#include "iomux.h"
#include <mach/irqs.h>
#include "mx51_pins.h"
#include <linux/timer.h>
#include "board-ccwmx51.h"
#include "linux/irq.h"
#if defined(CONFIG_KEYBOARD_GPIO)
#include "devices_ccwmx51.h"
#include <linux/input.h>
#endif

/*
 * Convenience conversion.
 * Here atm, maybe there is somewhere better for this.
 */
#define mV_to_uV(mV) (mV * 1000)
#define uV_to_mV(uV) (uV / 1000)
#define V_to_uV(V) (mV_to_uV(V * 1000))
#define uV_to_V(uV) (uV_to_mV(uV) / 1000)

/* Coin cell charger enable */
#define COINCHEN_LSH	23
#define COINCHEN_WID	1
/* Coin cell charger voltage setting */
#define VCOIN_LSH	20
#define VCOIN_WID	3

/* Coin Charger voltage */
#define VCOIN_2_5V	0x0
#define VCOIN_2_7V	0x1
#define VCOIN_2_8V	0x2
#define VCOIN_2_9V	0x3
#define VCOIN_3_0V	0x4
#define VCOIN_3_1V	0x5
#define VCOIN_3_2V	0x6
#define VCOIN_3_3V	0x7

/* Keeps VSRTC and CLK32KMCU on for all states */
#define DRM_LSH 4
#define DRM_WID 1

/* regulator standby mask */
#define GEN1_STBY_MASK		(1 << 1)
#define IOHI_STBY_MASK		(1 << 4)
#define DIG_STBY_MASK		(1 << 10)
#define GEN2_STBY_MASK		(1 << 13)
#define PLL_STBY_MASK		(1 << 16)
#define USB2_STBY_MASK		(1 << 19)

#define GEN3_STBY_MASK		(1 << 1)
#define CAM_STBY_MASK		(1 << 7)
#define VIDEO_STBY_MASK		(1 << 13)
#define AUDIO_STBY_MASK		(1 << 16)
#define SD_STBY_MASK		(1 << 19)

/* 0x92412 */
#define REG_MODE_0_ALL_MASK	(GEN1_STBY_MASK |\
				DIG_STBY_MASK |\
				PLL_STBY_MASK)
/* 0x92082 */
#define REG_MODE_1_ALL_MASK	(CAM_STBY_MASK | VIDEO_STBY_MASK |\
				AUDIO_STBY_MASK | SD_STBY_MASK)

/* switch mode setting */
#define	SW1MODE_LSB	0
#define	SW2MODE_LSB	10
#define	SW3MODE_LSB	0
#define	SW4MODE_LSB	8

#define	SWMODE_MASK	0xF
#define SWMODE_AUTO	0x8

/* CPU */
static struct regulator_consumer_supply sw1_consumers[] = {
	{
		.supply = "cpu_vcc",
	}
};

struct mc13892;

static struct regulator_init_data sw1_init = {
	.constraints = {
		.name = "SW1",
		.min_uV = mV_to_uV(600),
		.max_uV = mV_to_uV(1375),
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
		.valid_modes_mask = 0,
		.always_on = 1,
		.boot_on = 1,
		.initial_state = PM_SUSPEND_MEM,
		.state_mem = {
			.uV = 850000,
			.mode = REGULATOR_MODE_NORMAL,
			.enabled = 1,
		},
	},
	.num_consumer_supplies = ARRAY_SIZE(sw1_consumers),
	.consumer_supplies = sw1_consumers,
};

static struct regulator_init_data sw2_init = {
	.constraints = {
		.name = "SW2",
		.min_uV = mV_to_uV(900),
		.max_uV = mV_to_uV(1850),
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
		.always_on = 1,
		.boot_on = 1,
		.initial_state = PM_SUSPEND_MEM,
		.state_mem = {
			.uV = 950000,
			.mode = REGULATOR_MODE_NORMAL,
			.enabled = 1,
		},
	}
};

static struct regulator_init_data sw3_init = {
	.constraints = {
		.name = "SW3",
		.min_uV = mV_to_uV(1100),
		.max_uV = mV_to_uV(1850),
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
		.always_on = 1,
		.boot_on = 1,
	}
};

static struct regulator_init_data sw4_init = {
	.constraints = {
		.name = "SW4",
		.min_uV = mV_to_uV(1100),
		.max_uV = mV_to_uV(1850),
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
		.always_on = 1,
		.boot_on = 1,
	}
};

static struct regulator_init_data viohi_init = {
	.constraints = {
		.name = "VIOHI",
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.always_on = 1,
		.boot_on = 1,
	}
};

static struct regulator_init_data vusb_init = {
	.constraints = {
		.name = "VUSB",
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.boot_on = 1,
		.always_on = 1,
	}
};

static struct regulator_init_data swbst_init = {
	.constraints = {
		.name = "SWBST",
	}
};

static struct regulator_init_data vdig_init = {
	.constraints = {
		.name = "VDIG",
		.min_uV = mV_to_uV(1650),
		.max_uV = mV_to_uV(1650),
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
			REGULATOR_CHANGE_STATUS,
		.boot_on = 1,
	}
};

static struct regulator_init_data vpll_init = {
	.constraints = {
		.name = "VPLL",
		.min_uV = mV_to_uV(1050),
		.max_uV = mV_to_uV(1800),
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
			REGULATOR_CHANGE_STATUS,
		.boot_on = 1,
		.always_on = 1,
	}
};

static struct regulator_init_data vusb2_init = {
	.constraints = {
		.name = "VUSB2",
		.min_uV = mV_to_uV(2400),
		.max_uV = mV_to_uV(2775),
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
			REGULATOR_CHANGE_STATUS,
		.boot_on = 1,
		.always_on = 1,
	}
};

static struct regulator_init_data vvideo_init = {
	.constraints = {
		.name = "VVIDEO",
		.min_uV = mV_to_uV(2775),
		.max_uV = mV_to_uV(2775),
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
			REGULATOR_CHANGE_STATUS,
		.apply_uV =1,
	}
};

static struct regulator_init_data vaudio_init = {
	.constraints = {
		.name = "VAUDIO",
		.min_uV = mV_to_uV(2300),
		.max_uV = mV_to_uV(3000),
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
			REGULATOR_CHANGE_STATUS,
	}
};

static struct regulator_init_data vsd_init = {
	.constraints = {
		.name = "VSD",
		.min_uV = mV_to_uV(1800),
		.max_uV = mV_to_uV(3150),
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
			REGULATOR_CHANGE_STATUS,
	}
};

static struct regulator_init_data vcam_init = {
	.constraints = {
		.name = "VCAM",
		.min_uV = mV_to_uV(2500),
		.max_uV = mV_to_uV(3000),
		.valid_ops_mask =
			REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE |
			REGULATOR_CHANGE_STATUS,
		.valid_modes_mask = REGULATOR_MODE_FAST | REGULATOR_MODE_NORMAL,
	}
};

static struct regulator_init_data vgen1_init = {
	.constraints = {
		.name = "VGEN1",
		.min_uV = mV_to_uV(1200),
		.max_uV = mV_to_uV(1200),
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
			REGULATOR_CHANGE_STATUS,
	}
};

static struct regulator_init_data vgen2_init = {
	.constraints = {
		.name = "VGEN2",
		.min_uV = mV_to_uV(1200),
		.max_uV = mV_to_uV(3150),
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
			REGULATOR_CHANGE_STATUS,
		.always_on = 1,
	}
};

static struct regulator_init_data vgen3_init = {
	.constraints = {
		.name = "VGEN3",
		.min_uV = mV_to_uV(1800),
		.max_uV = mV_to_uV(2900),
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
			REGULATOR_CHANGE_STATUS,
		.always_on = 1,
	}
};

#if defined(CONFIG_KEYBOARD_GPIO)

enum power_on_state {
	PWRON_FROM_RESUME 	= 0,
	PWRON_FROM_SUSPEND  = 1
};

/*!
* the event handler for power on event
*/
static void power_on_evt_handler(void)
{
	int pwron1_sense = 0;
	int pwron1_interrupt = 0;
	static int state = -1;

	if( system_state != SYSTEM_RUNNING)
		return;

	if( state < 0 ){
		/* DIO0_POWER_KEY has been configured as an input GPIO KEY.
		 * However, we need to control it by software*/
		gpio_direction_output(DIO0_POWER_KEY,1);
		state = PWRON_FROM_RESUME;
	}

	switch (state) {
		case PWRON_FROM_RESUME:
			pmic_read_reg(REG_INT_SENSE1, &pwron1_sense, 0xffffff);
			if ( !(pwron1_sense & 0x8) /* Reg 5, Bit 3, PWRON1S */){
				state = PWRON_FROM_SUSPEND;
				gpio_set_value(DIO0_POWER_KEY,0);
			}
			break;

		case PWRON_FROM_SUSPEND:
			pmic_read_reg(REG_INT_STATUS1, &pwron1_interrupt, 0xffffff);
			if ( !(pwron1_interrupt & 0x8) /* Reg 3, Bit 3, PWRON1I */){
				state = PWRON_FROM_RESUME;
				gpio_set_value(DIO0_POWER_KEY,1);
			}
			break;

		default:
			pr_err("power_on_evt_handler: Uninitialized state\n");
	}
}

void power_on_gpio_event( int type , int code , int state)
{
	if( type == EV_KEY && code == KEY_POWER )
		gpio_set_value(DIO0_POWER_KEY,state);
}

#else
static void power_on_evt_handler(void) {}
#endif

static int mc13892_regulator_init(struct mc13892 *mc13892)
{
	unsigned int value, register_mask;
	pmic_event_callback_t power_key_event;

	printk("Initializing regulators for CCWMX51.\n");
	if (mxc_cpu_is_rev(CHIP_REV_2_0) < 0)
		sw2_init.constraints.state_mem.uV = 1100000;
	else if (mxc_cpu_is_rev(CHIP_REV_2_0) == 1) {
		sw2_init.constraints.state_mem.uV = 1250000;
		sw1_init.constraints.state_mem.uV = 1000000;
	}

	/* subscribe PWRON1 event to enable POWER key */
	power_key_event.param = NULL;
	power_key_event.func = (void *)power_on_evt_handler;
	pmic_event_subscribe(EVENT_PWRONI, power_key_event);

	/* enable standby controll for all regulators */
	pmic_read_reg(REG_MODE_0, &value, 0xffffff);
	value |= REG_MODE_0_ALL_MASK;
	pmic_write_reg(REG_MODE_0, value, 0xffffff);

	pmic_read_reg(REG_MODE_1, &value, 0xffffff);
	value |= REG_MODE_1_ALL_MASK;
	pmic_write_reg(REG_MODE_1, value, 0xffffff);

	/* enable switch audo mode */
	pmic_read_reg(REG_IDENTIFICATION, &value, 0xffffff);
	/* only for mc13892 2.0A */
	if ((value & 0x0000FFFF) == 0x45d0) {
		pmic_read_reg(REG_SW_4, &value, 0xffffff);
		register_mask = (SWMODE_MASK << SW1MODE_LSB) |
		       (SWMODE_MASK << SW2MODE_LSB);
		value &= ~register_mask;
		value |= (SWMODE_AUTO << SW1MODE_LSB) |
			(SWMODE_AUTO << SW2MODE_LSB);
		pmic_write_reg(REG_SW_4, value, 0xffffff);

		pmic_read_reg(REG_SW_5, &value, 0xffffff);
		register_mask = (SWMODE_MASK << SW3MODE_LSB) |
			(SWMODE_MASK << SW4MODE_LSB);
		value &= ~register_mask;
		value |= (SWMODE_AUTO << SW3MODE_LSB) |
			(SWMODE_AUTO << SW4MODE_LSB);
		pmic_write_reg(REG_SW_5, value, 0xffffff);
	}

	/* Disable coin cell charger */
	value = BITFVAL(COINCHEN, 0) | BITFVAL(VCOIN, VCOIN_3_0V);
	register_mask = BITFMASK(COINCHEN) | BITFMASK(VCOIN);
	pmic_write_reg(REG_POWER_CTL0, value, register_mask);

#if defined(CONFIG_RTC_DRV_MXC_V2) || defined(CONFIG_RTC_DRV_MXC_V2_MODULE)
	value = BITFVAL(DRM, 1);
	register_mask = BITFMASK(DRM);
	pmic_write_reg(REG_POWER_CTL0, value, register_mask);
#endif

	mc13892_register_regulator(mc13892, MC13892_SW1, &sw1_init);
	mc13892_register_regulator(mc13892, MC13892_SW2, &sw2_init);
	mc13892_register_regulator(mc13892, MC13892_SW3, &sw3_init);
	mc13892_register_regulator(mc13892, MC13892_SW4, &sw4_init);
	mc13892_register_regulator(mc13892, MC13892_SWBST, &swbst_init);
	mc13892_register_regulator(mc13892, MC13892_VIOHI, &viohi_init);
	mc13892_register_regulator(mc13892, MC13892_VPLL, &vpll_init);
	mc13892_register_regulator(mc13892, MC13892_VDIG, &vdig_init);
	mc13892_register_regulator(mc13892, MC13892_VSD, &vsd_init);
	mc13892_register_regulator(mc13892, MC13892_VUSB2, &vusb2_init);
	mc13892_register_regulator(mc13892, MC13892_VVIDEO, &vvideo_init);
	mc13892_register_regulator(mc13892, MC13892_VAUDIO, &vaudio_init);
	mc13892_register_regulator(mc13892, MC13892_VCAM, &vcam_init);
	mc13892_register_regulator(mc13892, MC13892_VGEN1, &vgen1_init);
	mc13892_register_regulator(mc13892, MC13892_VGEN2, &vgen2_init);
	mc13892_register_regulator(mc13892, MC13892_VGEN3, &vgen3_init);
	mc13892_register_regulator(mc13892, MC13892_VUSB, &vusb_init);

	/* FIXME  Move the code below to the corresponding initialization functions */
	pmic_read_reg(REG_POWER_MISC, &value, 0xffffff);
	value &= ~((3 << 8) | (3 << 10) | (3 << 12));
	pmic_write_reg(REG_POWER_MISC, value, (3 << 8) | (3 << 10) | (3 << 12));
	udelay(250);
#if defined(CONFIG_SMSC911X) || defined(CONFIG_SMSC911X_MODULE)
	/* Set to high external mac reset line */
	value |= (1 << 8);
#endif
#if defined(CONFIG_FEC) || defined(CONFIG_FEC_MODULE)
	/* Set to high fec phy reset line */
	value |= (1 << 10);
#endif
	if (machine_is_ccwmx51js() || machine_is_ccwmx51())
		/* Set to high wireless module reset line */
		value |= (1 << 12);

	pmic_write_reg(REG_POWER_MISC, value, (3 << 8) | (3 << 10) | (3 << 12));

	return 0;
}

static struct mc13892_platform_data mc13892_plat = {
	.init = mc13892_regulator_init,
};

static struct spi_board_info __initdata mc13892_spi_device = {
	.modalias = "mc13892",
	.irq = IOMUX_TO_IRQ(MX51_PIN_GPIO1_5),
	.max_speed_hz = 6000000,	/* max spi SCK clock speed in HZ */
	.bus_num = 1,
	.chip_select = 0,
	.platform_data = &mc13892_plat,
};


int __init ccwmx51_init_mc13892(void)
{
	return spi_register_board_info(&mc13892_spi_device, 1);
}

