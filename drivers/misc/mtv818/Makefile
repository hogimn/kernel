#
# Kernel makefile for TDMB(MTV818)
#

ifdef CONFIG_TDMB_MTV818
EXTRA_CFLAGS += -I. -I$(PRJ_ROOT_DIR)/src
endif

obj-$(CONFIG_TDMB_MTV818)	+= mtv818.o \
							mtv818_i2c.o \
							mtv818_spi.o \
							mtv818_isr.o \
							./src/raontv.o \
							./src/raontv_port.o \
							./src/raontv_rf.o \
							./src/raontv_tdmb.o \
							./src/raontv_isdbt.o \
							./src/raontv_fm.o \
							./src/raontv_cif_dec.o
