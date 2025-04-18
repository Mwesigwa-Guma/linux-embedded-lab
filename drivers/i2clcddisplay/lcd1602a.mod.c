#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

#ifdef CONFIG_UNWINDER_ORC
#include <asm/orc_header.h>
ORC_HEADER;
#endif

BUILD_SALT;
BUILD_LTO_INFO;

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



static const char ____versions[]
__used __section("__versions") =
	"\x1c\x00\x00\x00\x72\x75\x4f\xeb"
	"i2c_register_driver\0"
	"\x14\x00\x00\x00\x09\x27\x65\xa9"
	"_dev_info\0\0\0"
	"\x18\x00\x00\x00\x0b\x1d\xe6\xd3"
	"misc_deregister\0"
	"\x18\x00\x00\x00\x0f\x04\x82\x51"
	"i2c_del_driver\0\0"
	"\x18\x00\x00\x00\x6f\xea\x8a\x06"
	"devm_kmalloc\0\0\0\0"
	"\x10\x00\x00\x00\xfd\xf9\x3f\x3c"
	"sprintf\0"
	"\x18\x00\x00\x00\x63\xcf\x33\xd6"
	"misc_register\0\0\0"
	"\x24\x00\x00\x00\x60\x29\xbe\x27"
	"i2c_transfer_buffer_flags\0\0\0"
	"\x10\x00\x00\x00\xf9\x82\xa4\xf9"
	"msleep\0\0"
	"\x10\x00\x00\x00\x7e\x3a\x2c\x12"
	"_printk\0"
	"\x1c\x00\x00\x00\xcb\xf6\xfd\xf0"
	"__stack_chk_fail\0\0\0\0"
	"\x1c\x00\x00\x00\x48\x9f\xdb\x88"
	"__check_object_size\0"
	"\x20\x00\x00\x00\x28\xe1\xa4\x12"
	"__arch_copy_from_user\0\0\0"
	"\x24\x00\x00\x00\x52\x3f\x0a\x4b"
	"gic_nonsecure_priorities\0\0\0\0"
	"\x1c\x00\x00\x00\xef\x6d\x5c\xa6"
	"alt_cb_patch_nops\0\0\0"
	"\x1c\x00\x00\x00\x1b\x2e\x3d\xec"
	"trace_hardirqs_off\0\0"
	"\x1c\x00\x00\x00\x9a\xe6\x97\xd6"
	"trace_hardirqs_on\0\0\0"
	"\x28\x00\x00\x00\xb3\x1c\xa2\x87"
	"__ubsan_handle_out_of_bounds\0\0\0\0"
	"\x18\x00\x00\x00\x8c\x89\xd4\xcb"
	"fortify_panic\0\0\0"
	"\x18\x00\x00\x00\xdc\x47\xa0\xa9"
	"module_layout\0\0\0"
	"\x00\x00\x00\x00\x00\x00\x00\x00";

MODULE_INFO(depends, "");

MODULE_ALIAS("i2c:lcd1602");
MODULE_ALIAS("of:N*T*Cqapass,lcd1602");
MODULE_ALIAS("of:N*T*Cqapass,lcd1602C*");

MODULE_INFO(srcversion, "635D54A009290712D010054");
