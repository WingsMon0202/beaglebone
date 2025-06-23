#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
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
__used
__attribute__((section("__versions"))) = {
	{ 0x516e49f9, "module_layout" },
	{ 0x2e5810c6, "__aeabi_unwind_cpp_pr1" },
	{ 0x395c5ae9, "gpio_to_desc" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
	{ 0xe89992a5, "__platform_driver_register" },
	{ 0x1bf2b029, "of_property_read_string" },
	{ 0xc4021e53, "_dev_err" },
	{ 0x44bc07ec, "devm_gpio_request_one" },
	{ 0x7767f9fb, "_dev_info" },
	{ 0x4b2b94d9, "of_get_named_gpio_flags" },
	{ 0xddc1fca4, "gpiod_set_raw_value" },
	{ 0xf55bc851, "platform_driver_unregister" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("of:N*T*Cwings,gpio-led");
MODULE_ALIAS("of:N*T*Cwings,gpio-ledC*");
