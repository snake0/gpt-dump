#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x8f71abee, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0xb3f7646e, __VMLINUX_SYMBOL_STR(kthread_should_stop) },
	{ 0xa44d9493, __VMLINUX_SYMBOL_STR(sock_create_lite) },
	{ 0x4e6b3d9b, __VMLINUX_SYMBOL_STR(sock_release) },
	{ 0xe55b18, __VMLINUX_SYMBOL_STR(__mutex_init) },
	{ 0x1b6314fd, __VMLINUX_SYMBOL_STR(in_aton) },
	{ 0x779a18af, __VMLINUX_SYMBOL_STR(kstrtoll) },
	{ 0xd5d7e85e, __VMLINUX_SYMBOL_STR(sock_create) },
	{ 0x12a38747, __VMLINUX_SYMBOL_STR(usleep_range) },
	{ 0xf16cb605, __VMLINUX_SYMBOL_STR(mutex_unlock) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0x69acdf38, __VMLINUX_SYMBOL_STR(memcpy) },
	{ 0xd6cd58e5, __VMLINUX_SYMBOL_STR(current_task) },
	{ 0x8ca314b0, __VMLINUX_SYMBOL_STR(mutex_lock) },
	{ 0x72f320dc, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0xe36c7d76, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0x3922dbe3, __VMLINUX_SYMBOL_STR(kernel_sendmsg) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0xdb7305a1, __VMLINUX_SYMBOL_STR(__stack_chk_fail) },
	{ 0x11997383, __VMLINUX_SYMBOL_STR(kernel_recvmsg) },
	{ 0xbdfb6dbb, __VMLINUX_SYMBOL_STR(__fentry__) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "3C6117FCCD5134F38418421");
