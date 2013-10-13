/*
 * Copyright 2012 Digi International, Inc. All Rights Reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <linux/types.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/stat.h>
#include <linux/slab.h>
#include "mx28_ccardxmx28.h"

static u8 ccardxmx28_mod_variant = 0;
static u8 ccardxmx28_mod_ver = 0;
static u32 ccardxmx28_mod_sn = 0;

void ccardxmx28_set_mod_variant(u8 variant)
{
	ccardxmx28_mod_variant = variant;
}
void ccardxmx28_set_mod_version(u8 version)
{
	ccardxmx28_mod_ver = version;
}
u8 ccardxmx28_get_mod_version(void)
{
	return ccardxmx28_mod_ver;
}
void ccardxmx28_set_mod_sn(u32 sn)
{
	ccardxmx28_mod_sn = sn;
}

#ifdef CONFIG_SYSFS
static ssize_t ccardxmx28_mod_variant_attr_show(struct kobject *kobj,
					     struct kobj_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", ccardxmx28_mod_variant);
}

static ssize_t ccardxmx28_mod_ver_attr_show(struct kobject *kobj,
					 struct kobj_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", ccardxmx28_mod_ver);
}

static ssize_t ccardxmx28_mod_sn_attr_show(struct kobject *kobj,
					struct kobj_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", ccardxmx28_mod_sn);
}

static struct kobj_attribute ccardxmx28_mod_variant_attr =
	__ATTR(mod_variant, S_IRUGO, ccardxmx28_mod_variant_attr_show, NULL);
static struct kobj_attribute ccardxmx28_mod_ver_attr =
	__ATTR(mod_ver, S_IRUGO, ccardxmx28_mod_ver_attr_show, NULL);
static struct kobj_attribute ccardxmx28_mod_sn_attr =
	__ATTR(mod_sn, S_IRUGO, ccardxmx28_mod_sn_attr_show, NULL);

int ccardxmx28_create_sysfs_entries(void)
{
	struct kobject *ccardxmx28_kobj;
	int ret;

	ccardxmx28_kobj = kobject_create_and_add(CCARDXMX28_SYSFS_FNAME, kernel_kobj);
	if (!ccardxmx28_kobj) {
		printk(KERN_WARNING "kobject_create_and_add %s failed\n",
		       CCARDXMX28_SYSFS_FNAME);
		return -EINVAL;
	}

	ret = sysfs_create_file(ccardxmx28_kobj, &ccardxmx28_mod_variant_attr.attr);
	if (ret) {
		printk(KERN_ERR
		       "Unable to register sysdev entry for %s hardware variant\n",
		       CCARDXMX28_SYSFS_FNAME);
		return ret;
	}
	ret = sysfs_create_file(ccardxmx28_kobj, &ccardxmx28_mod_ver_attr.attr);
	if (ret) {
		printk(KERN_ERR
		       "Unable to register sysdev entry for %s hardware version\n",
		       CCARDXMX28_SYSFS_FNAME);
		return ret;
	}
	ret = sysfs_create_file(ccardxmx28_kobj, &ccardxmx28_mod_sn_attr.attr);
	if (ret) {
		printk(KERN_ERR
		       "Unable to register sysdev entry for %s hardware SN\n",
		       CCARDXMX28_SYSFS_FNAME);
		return ret;
	}

	return 0;
}
#else
int ccardxmx28_create_sysfs_entries(void) { return 0; }
#endif /* CONFIG_SYSFS */

