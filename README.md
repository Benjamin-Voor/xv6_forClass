The following files are automatically generated and I'm not going to be extra careful merging them. You can .gitignore them for all I care, because they'll come back when you run "make qemu-nox":

xv6_patched/fs/
xv6_patched/kernel/bootblock
xv6_patched/kernel/kernel
xv6_patched/user/bin/
xv6_patched/xv6.img
xv6_patched/fs.img

The following file is NOT automatically generated:
!xv6_patched/tools/mkfs.c
However, as a .c file, it does automatically generate these other files:
xv6_patched/tools/mkfs
xv6_patched/tools/mkfs.d
xv6_patched/tools/mkfs.o
