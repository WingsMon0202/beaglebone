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
	{ 0x2d3385d3, "system_wq" },
	{ 0xffeedf6a, "delayed_work_timer_fn" },
	{ 0xd0595c84, "devm_gpiod_get" },
	{ 0xc6f46339, "init_timer_key" },
	{ 0x29d9f26e, "cancel_delayed_work_sync" },
	{ 0x6beec397, "__platform_driver_register" },
	{ 0x14a36ccb, "gpio_led_toggle" },
	{ 0xc5850110, "printk" },
	{ 0x58017092, "of_property_read_string" },
	{ 0xd44f7d43, "_dev_err" },
	{ 0xd7dca449, "_dev_info" },
	{ 0xcf86cdac, "queue_delayed_work_on" },
	{ 0xb85ec714, "gpiod_get_value" },
	{ 0x96f3c457, "gpiod_to_irq" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
	{ 0x49c33077, "platform_driver_unregister" },
	{ 0xcdd3486a, "devm_request_threaded_irq" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("of:N*T*Cwings,gpio-button");
MODULE_ALIAS("of:N*T*Cwings,gpio-buttonC*");

MODULE_INFO(srcversion, "B7466F21D63072C42455B83");
