c6000_comm = common/c6k_arch_interface.c  common/c6k_module.c common/exec_insn.c common/c6k_decode_16.c
libc6k_la_SOURCES = $(c6000_comm) 
pkglib_LTLIBRARIES = libc6k.la
# we will link the common module with main
libc6k_la_LDFLAGS = -module
DEFS = -g -O2 -D_FILE_OFFSET_BITS=64 -DSTANDALONE -DDEFAULT_INLINE=0 -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS @COMMON_CFLAGS@
INCLUDES = -I./common -I./mach @COMMON_INCLUDES@
