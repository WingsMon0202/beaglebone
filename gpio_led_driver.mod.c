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
	{ 0xd0595c84, "devm_gpiod_get" },
	{ 0x6beec397, "__platform_driver_register" },
	{ 0xc5850110, "printk" },
	{ 0x58017092, "of_property_read_string" },
	{ 0xd44f7d43, "_dev_err" },
	{ 0x86332725, "__stack_chk_fail" },
	{ 0xb85ec714, "gpiod_get_value" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
	{ 0x8f678b07, "__stack_chk_guard" },
	{ 0x49c33077, "platform_driver_unregister" },
	{ 0x29e06a52, "gpiod_set_value" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("of:N*T*Cwings,gpio-led");
MODULE_ALIAS("of:N*T*Cwings,gpio-ledC*");

MODULE_INFO(srcversion, "E40113CCBCF39A1113660E1");
