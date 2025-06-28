#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xe798b5ff, "module_layout" },
	{ 0xc4677298, "device_destroy" },
	{ 0xe346f67a, "__mutex_init" },
	{ 0x108c4296, "class_destroy" },
	{ 0xd80244ee, "device_create" },
	{ 0x376d64e5, "cdev_del" },
	{ 0xb812f02d, "__class_create" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0xd900a2be, "cdev_add" },
	{ 0xbfce1636, "cdev_init" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x51a910c0, "arm_copy_to_user" },
	{ 0xc358aaf8, "snprintf" },
	{ 0xb07c7216, "get_button_status" },
	{ 0x60b8ecca, "get_led_status" },
	{ 0x8f678b07, "__stack_chk_guard" },
	{ 0x86332725, "__stack_chk_fail" },
	{ 0x3dcf1ffa, "__wake_up" },
	{ 0x14a36ccb, "gpio_led_toggle" },
	{ 0x84b183ae, "strncmp" },
	{ 0xae353d77, "arm_copy_from_user" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
	{ 0xc5850110, "printk" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "215ACC56A74A173D6E9BE8F");
