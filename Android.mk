.PHONY: del_linux_image

KERNEL_ENV := ARCH=arm CROSS_COMPILE=../$(TARGET_TOOLS_PREFIX)

ifeq ($(TARGET_DEL_LINUX_IMAGE),true)
del_linux_image: $(MKIMAGE)
ifeq ($(wildcard kernel_imx/.config),)
	for a in $(TARGET_DEL_LINUX_DEFCONF); do \
		suffix="$$(echo $${a} | sed -e 's,^.*_android_\(.*\)_defconfig,\1,g')"; \
		echo "$${a}:$${suffix}"; \
		$(MAKE) -C kernel_imx $(KERNEL_ENV) $${a}; \
		$(MAKE) -C kernel_imx $(KERNEL_ENV) uImage; \
		$(ACP) kernel_imx/arch/arm/boot/uImage $(PRODUCT_OUT)/uImage-android-$${suffix}; \
	done
else
	$(MAKE) -C kernel_imx $(KERNEL_ENV) oldconfig
	$(MAKE) -C kernel_imx $(KERNEL_ENV) uImage
	$(ACP) kernel_imx/arch/arm/boot/uImage $(PRODUCT_OUT)/uImage-android-$(TARGET_BOOTLOADER_BOARD_NAME)
endif

else
del_linux_image:
endif

del_linux_distclean:
	$(MAKE) -C kernel_imx $(KERNEL_ENV) distclean
