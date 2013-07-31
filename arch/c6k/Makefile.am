c6000_comm = common/c6k_arch_interface.c  common/c6k_module.c common/exec_insn.c common/c6k_decode_16.c
libc6k_la_SOURCES = $(c6000_comm) 
pkglib_LTLIBRARIES = libc6k.la
# we will link the common module with main
libc6000_la_LDFLAGS = -module
INCLUDES = -I./common -I./mach @COMMON_INCLUDES@
