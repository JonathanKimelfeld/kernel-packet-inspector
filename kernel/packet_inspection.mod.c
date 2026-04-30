#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

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



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xe8213e80, "_printk" },
	{ 0x0ead42bf, "single_open" },
	{ 0xee7dd786, "seq_printf" },
	{ 0x96370a3b, "__register_chrdev" },
	{ 0x94d4eff0, "proc_create" },
	{ 0x52b15b3b, "__unregister_chrdev" },
	{ 0x0f7963b7, "proc_remove" },
	{ 0x5af09d8b, "_raw_spin_lock" },
	{ 0x4f1e5fd0, "__list_del_entry_valid_or_report" },
	{ 0xcb8b6ec6, "kfree" },
	{ 0x5af09d8b, "_raw_spin_unlock" },
	{ 0xbd03ed67, "random_kmalloc_seed" },
	{ 0xdf601750, "kmalloc_caches" },
	{ 0xdfeb88b0, "__kmalloc_cache_noprof" },
	{ 0xdd6830c7, "sprintf" },
	{ 0x437e81c7, "simple_read_from_buffer" },
	{ 0xaa47b76e, "__arch_copy_from_user" },
	{ 0x0e9cab28, "memset" },
	{ 0x9c4ed43a, "alt_cb_patch_nops" },
	{ 0xaa47b76e, "__arch_copy_to_user" },
	{ 0xd272d446, "__stack_chk_fail" },
	{ 0xdc352a3b, "__list_add_valid_or_report" },
	{ 0x62aaf70f, "seq_read" },
	{ 0x7bb3b2f8, "seq_lseek" },
	{ 0xf200bb2d, "single_release" },
	{ 0xaedda0fe, "param_ops_int" },
	{ 0x9b3f403c, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0xe8213e80,
	0x0ead42bf,
	0xee7dd786,
	0x96370a3b,
	0x94d4eff0,
	0x52b15b3b,
	0x0f7963b7,
	0x5af09d8b,
	0x4f1e5fd0,
	0xcb8b6ec6,
	0x5af09d8b,
	0xbd03ed67,
	0xdf601750,
	0xdfeb88b0,
	0xdd6830c7,
	0x437e81c7,
	0xaa47b76e,
	0x0e9cab28,
	0x9c4ed43a,
	0xaa47b76e,
	0xd272d446,
	0xdc352a3b,
	0x62aaf70f,
	0x7bb3b2f8,
	0xf200bb2d,
	0xaedda0fe,
	0x9b3f403c,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"_printk\0"
	"single_open\0"
	"seq_printf\0"
	"__register_chrdev\0"
	"proc_create\0"
	"__unregister_chrdev\0"
	"proc_remove\0"
	"_raw_spin_lock\0"
	"__list_del_entry_valid_or_report\0"
	"kfree\0"
	"_raw_spin_unlock\0"
	"random_kmalloc_seed\0"
	"kmalloc_caches\0"
	"__kmalloc_cache_noprof\0"
	"sprintf\0"
	"simple_read_from_buffer\0"
	"__arch_copy_from_user\0"
	"memset\0"
	"alt_cb_patch_nops\0"
	"__arch_copy_to_user\0"
	"__stack_chk_fail\0"
	"__list_add_valid_or_report\0"
	"seq_read\0"
	"seq_lseek\0"
	"single_release\0"
	"param_ops_int\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "38FAB4D0D989880AE71703D");
