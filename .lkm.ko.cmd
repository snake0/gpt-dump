cmd_/home/snake0/linux-kernel-module/lkm.ko := ld -r -m elf_x86_64 -z max-page-size=0x200000 -T ./scripts/module-common.lds --build-id  -o /home/snake0/linux-kernel-module/lkm.ko /home/snake0/linux-kernel-module/lkm.o /home/snake0/linux-kernel-module/lkm.mod.o ;  true
