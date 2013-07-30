#dev_uart = uart/skyeye_uart.c          uart/skyeye_uart_net.c   uart/skyeye_uart_stdio.c uart/uart_module.c \
#uart/skyeye_uart_cvt_dcc.c  uart/skyeye_uart_pipe.c
dev_uart = uart/skyeye_uart.c          uart/skyeye_uart_net.c   uart/skyeye_uart_stdio.c uart/uart_module.c \
uart/skyeye_uart_pipe.c uart/skyeye_uart_term.c

dev_uart_16550 = uart_16550/uart_16550.c uart_16550/uart_16550_module.c
dev_lcd_s3c6410 = lcd_s3c6410/lcd_s3c6410.c lcd_s3c6410/lcd_s3c6410_module.c
dev_sdhci_s3c6410 = sdhci_s3c6410/sdhci_s3c6410.c sdhci_s3c6410/sdhci_s3c6410_module.c
dev_touchscreen_s3c6410 = touchscreen_s3c6410/touchscreen_s3c6410.c touchscreen_s3c6410/touchscreen_s3c6410_module.c
dev_sysctrl_s3c6410 = sysctrl_s3c6410/sysctrl_s3c6410.c sysctrl_s3c6410/sysctrl_s3c6410_module.c
dev_gpio_s3c6410 = gpio_s3c6410/gpio_s3c6410.c gpio_s3c6410/gpio_s3c6410_module.c
dev_spi_s3c6410 = spi_s3c6410/spi_s3c6410.c spi_s3c6410/spi_s3c6410_module.c
dev_usbhost_s3c6410 = usbhost_s3c6410/usbhost_s3c6410.c usbhost_s3c6410/usbhost_s3c6410_module.c
dev_ac97_s3c6410 = ac97_s3c6410/ac97_s3c6410.c ac97_s3c6410/ac97_s3c6410_module.c
dev_mfc_s3c6410 = mfc_s3c6410/mfc_s3c6410.c mfc_s3c6410/mfc_s3c6410_module.c
dev_rtc_s3c6410 = rtc_s3c6410/rtc_s3c6410.c rtc_s3c6410/rtc_s3c6410_module.c
dev_camif_s3c6410 = camif_s3c6410/camif_s3c6410.c camif_s3c6410/camif_s3c6410_module.c
dev_image = image/image.c image/image_module.c
dev_ram = ram/ram.c ram/ram_module.c

#dev_flash = flash/armflash.c  flash/dev_flash_sst39lvf160.c flash/flash_module.c\
#flash/dev_flash_am29.c  flash/dev_flash_intel.c  flash/skyeye_flash.c
dev_flash = flash/dev_flash_sst39lvf160.c flash/flash_module.c\
flash/dev_flash_am29.c  flash/dev_flash_intel.c  flash/skyeye_flash.c

dev_nandflash = nandflash/dev_nandflash_s3c2410.c  nandflash/nandflash_smallblock.c  nandflash/skyeye_nandflash.c nandflash/nandflash_module.c nandflash/dev_nandflash_s3c6410.c nandflash/K9G8G08.c nandflash/nandflash_K9G8G08.c  

#dev_sound = sound/dev_sound_s3c44b0x.c  sound/skyeye_sound.c  sound/skyeye_sound_pcm.c sound/sound_module.c
dev_sound = sound/skyeye_sound.c  sound/skyeye_sound_pcm.c sound/sound_module.c

dev_lcd = lcd/dev_lcd_au1100.c  lcd/dev_lcd_s3c2410.c   lcd/skyeye_lcd.c \
lcd/dev_lcd_ep7312.c  lcd/dev_lcd_s3c44b0x.c lcd/dev_lcd_pxa.c lcd/lcd_module.c

gtk_lcd = lcd_gtk/lcd_gtk_module.c lcd_gtk/lcd_gtk.c
sdl_lcd = lcd_sdl/lcd_sdl_module.c lcd_sdl/lcd_sdl.c

#dev_net = net/dev_net_cs8900a.c  net/dev_net_s3c4510b.c net/skyeye_net_tuntap.c \
#net/dev_net_rtl8019.c  net/skyeye_net.c    net/skyeye_net_vnet.c net/net_module.c
#dev_net = net/dev_net_cs8900a.c  net/skyeye_net_tuntap.c net/dev_net_s3c4510b.c

dev_net = net/dev_net_cs8900a.c  net/skyeye_net_tuntap.c \
net/dev_net_rtl8019.c net/skyeye_net_vhub.c net/skyeye_net.c net/skyeye_net_vnet.c net/net_module.c

dev_ts = touchscreen/dev_touchscreen_skyeye.c  touchscreen/skyeye_touchscreen.c touchscreen/ts_module.c
dev_pci = pci/pci_bus/pci.c  pci/pci_bus/pci_module.c
#libdev_a_SOURCES = skyeye_device.c $(dev_uart) $(dev_flash) $(dev_nandflash) $(dev_sound) $(dev_net) $(dev_ts)
libuart_la_SOURCES = $(dev_uart)
libuart_la_LDFLAGS = -module

libuart_16550_la_SOURCES = $(dev_uart_16550)
libuart_16550_la_LDFLAGS = -module

liblcd_s3c6410_la_SOURCES = $(dev_lcd_s3c6410)
liblcd_s3c6410_la_LDFLAGS = -module

if GTK_LCD
libgtk_lcd_la_SOURCES = $(gtk_lcd) 
libgtk_lcd_la_LDFLAGS = -module $(LCD_LIBS)
endif

libsdl_lcd_la_SOURCES = $(sdl_lcd) 

if BUILD_ANDROID
if WIN32
libsdl_lcd_la_LDFLAGS = -module -Xlinker -L$(top_builddir)/android/objs/emulator_libui/.libs/ -Xlinker -lemulator_libui
else
libsdl_lcd_la_LDFLAGS = -module -L$(top_builddir)/android/objs/emulator_libui -lemulator_libui
endif
endif

libsdhci_s3c6410_la_SOURCES = $(dev_sdhci_s3c6410)
libsdhci_s3c6410_la_LDFLAGS = -module

libtouchscreen_s3c6410_la_SOURCES = $(dev_touchscreen_s3c6410)
libtouchscreen_s3c6410_la_LDFLAGS = -module

libsysctrl_s3c6410_la_SOURCES = $(dev_sysctrl_s3c6410)
libsysctrl_s3c6410_la_LDFLAGS = -module

libgpio_s3c6410_la_SOURCES = $(dev_gpio_s3c6410)
libgpio_s3c6410_la_LDFLAGS = -module
libspi_s3c6410_la_SOURCES = $(dev_spi_s3c6410)
libspi_s3c6410_la_LDFLAGS = -module
libusbhost_s3c6410_la_SOURCES = $(dev_usbhost_s3c6410)
libusbhost_s3c6410_la_LDFLAGS = -module
libac97_s3c6410_la_SOURCES = $(dev_ac97_s3c6410)
libac97_s3c6410_la_LDFLAGS = -module
libmfc_s3c6410_la_SOURCES = $(dev_mfc_s3c6410)
libmfc_s3c6410_la_LDFLAGS = -module
librtc_s3c6410_la_SOURCES = $(dev_rtc_s3c6410)
librtc_s3c6410_la_LDFLAGS = -module
libcamif_s3c6410_la_SOURCES = $(dev_camif_s3c6410)
libcamif_s3c6410_la_LDFLAGS = -module

libimage_la_SOURCES = $(dev_image)
libimage_la_LDFLAGS = -module

libram_la_SOURCES = $(dev_ram)
libram_la_LDFLAGS = -module

libflash_la_SOURCES = $(dev_flash)
libflash_la_LDFLAGS = -module

libnandflash_la_SOURCES = $(dev_nandflash)
libnandflash_la_LDFLAGS = -module

libsound_la_SOURCES = $(dev_sound)
libsound_la_LDFLAGS = -module

liblcd_la_SOURCES = $(dev_lcd)
liblcd_la_LDFLAGS = -module $(LCD_LIBS)

libnet_la_SOURCES = $(dev_net)
libnet_la_LDFLAGS = -module

libts_la_SOURCES = $(dev_ts)
libts_la_LDFLAGS = -module

libpci_la_SOURCES = $(dev_pci)
libpci_la_LDFLAGS = -module

dev_leon2_uart = uart_leon2/uart_leon2.c uart_leon2/uart_leon2_module.c
libleon2_uart_la_SOURCES = $(dev_leon2_uart)
libleon2_uart_la_LDFLAGS = -module

dev_uart_term = uart_term/uart_term.c uart_term/uart_term_module.c
libuart_term_la_SOURCES = $(dev_uart_term)
libuart_term_la_LDFLAGS = -module


dev_am35x_hecc = am35x_hecc/am35x_hecc.c am35x_hecc/am35x_hecc_module.c
libam35x_hecc_la_SOURCES = $(dev_am35x_hecc)
libam35x_hecc_la_LDFLAGS = -module

dev_can_zlg = can_zlg/can_zlg.c can_zlg/can_zlg_module.c
libcan_zlg_la_SOURCES = $(dev_can_zlg)
libcan_zlg_la_LDFLAGS = -module

if WIN32
WIN32_LD_DEVICE = -no-undefined -shared -module -Xlinker -L../common/.libs/  -Xlinker -lcommon -L/mingw/lib  -Xlinker -L/mingw/ -Xlinker --export-all-symbol
libuart_la_LDFLAGS += $(WIN32_LD_DEVICE)
libuart_16550_la_LDFLAGS += $(WIN32_LD_DEVICE)
liblcd_s3c6410_la_LDFLAGS += $(WIN32_LD_DEVICE)
libram_la_LDFLAGS += $(WIN32_LD_DEVICE)
libimage_la_LDFLAGS += $(WIN32_LD_DEVICE)
if GTK_LCD
libgtk_lcd_la_LDFLAGS += $(WIN32_LD_DEVICE)
endif
libsdl_lcd_la_SOURCES += $(WIN32_LD_DEVICE)
libsdhci_s3c6410_la_LDFLAGS += $(WIN32_LD_DEVICE)
libtouchscreen_s3c6410_la_LDFLAGS += $(WIN32_LD_DEVICE)
libsysctrl_s3c6410_la_LDFLAGS += $(WIN32_LD_DEVICE)
libgpio_s3c6410_la_LDFLAGS += $(WIN32_LD_DEVICE)
libspi_s3c6410_la_LDFLAGS += $(WIN32_LD_DEVICE)
libusbhost_s3c6410_la_LDFLAGS += $(WIN32_LD_DEVICE)
libac97_s3c6410_la_LDFLAGS += $(WIN32_LD_DEVICE)
libmfc_s3c6410_la_LDFLAGS += $(WIN32_LD_DEVICE)
librtc_s3c6410_la_LDFLAGS += $(WIN32_LD_DEVICE)
libcamif_s3c6410_la_LDFLAGS += $(WIN32_LD_DEVICE)
libflash_la_LDFLAGS += $(WIN32_LD_DEVICE)
libnandflash_la_LDFLAGS += $(WIN32_LD_DEVICE)
libsound_la_LDFLAGS += $(WIN32_LD_DEVICE)
liblcd_la_LDFLAGS += $(WIN32_LD_DEVICE)
libnet_la_LDFLAGS += $(WIN32_LD_DEVICE)
libts_la_LDFLAGS  += $(WIN32_LD_DEVICE)
libpci_la_LDFLAGS += $(WIN32_LD_DEVICE)
libleon2_uart_la_LDFLAGS  += $(WIN32_LD_DEVICE)
libuart_term_la_LDFLAGS  += $(WIN32_LD_DEVICE)
libam35x_hecc_la_LDFLAGS  += $(WIN32_LD_DEVICE)
libcan_zlg_la_LDFLAGS += $(WIN32_LD_DEVICE)
endif

pkglib_LTLIBRARIES = libuart.la libuart_16550.la libflash.la libnandflash.la libsound.la liblcd.la libnet.la libts.la libpci.la
pkglib_LTLIBRARIES += liblcd_s3c6410.la libsdhci_s3c6410.la libtouchscreen_s3c6410.la libsysctrl_s3c6410.la libgpio_s3c6410.la libspi_s3c6410.la libusbhost_s3c6410.la libac97_s3c6410.la libmfc_s3c6410.la libcamif_s3c6410.la librtc_s3c6410.la libleon2_uart.la libuart_term.la libimage.la libram.la

if GTK_LCD
pkglib_LTLIBRARIES += libgtk_lcd.la
endif
pkglib_LTLIBRARIES += libsdl_lcd.la
pkglib_LTLIBRARIES += libam35x_hecc.la
pkglib_LTLIBRARIES += libcan_zlg.la


DEFS = -DMODET -g -O2 -D_FILE_OFFSET_BITS=64 -DSTANDALONE -DDEFAULT_INLINE=0  @COMMON_CFLAGS@

skyeye_bin_dir = \"$(prefix)/bin/\"
INCLUDES = -I./ @COMMON_INCLUDES@ -I./uart_16550 -DSKYEYE_BIN=$(skyeye_bin_dir)
INCLUDES += -I./nandflash  -I./uart_xdht   -I./gpio_am335x  -I./intc_xdht  -I./uart_am335x


INCLUDES += @LCD_CFLAGS@
if WIN32
INCLUDES +=-I/mingw/gtk-2.0/include/gtk-2.0/
INCLUDES +=-I/mingw/gtk-2.0/include/glib-2.0/
INCLUDES +=-I/mingw/gtk-2.0/include/cairo/
INCLUDES +=-I/mingw/gtk-2.0/include/pango-1.0/
INCLUDES +=-I/mingw/gtk-2.0/include/atk-1.0/
INCLUDES +=-I/mingw/gtk-2.0/lib/gtk-2.0/include/
INCLUDES +=-I/mingw/gtk-2.0/lib/glib-2.0/include/

else
INCLUDES += -I/usr/include/gtk-2.0/ -I/usr/include/glib-2.0/
endif
if GTK_LCD
liblcd_la_SOURCES += lcd/skyeye_lcd_gtk.c
endif # GTK_LCD
if WIN32_LCD
liblcd_la_SOURCES += lcd/skyeye_lcd_win32.c
endif # WIN32_LCD
INCLUDES += -I$(top_srcdir)/android/
INCLUDES += -I$(top_srcdir)
INCLUDES += -I$(top_srcdir)/android/skyeye/
dev_s3c6410_keypad = s3c6410_keypad/s3c6410_keypad.c s3c6410_keypad/s3c6410_keypad_module.c
libs3c6410_keypad_la_SOURCES = $(dev_s3c6410_keypad)
libs3c6410_keypad_la_LDFLAGS = -module
pkglib_LTLIBRARIES += libs3c6410_keypad.la
dev_timer_am335x = timer_am335x/timer_am335x.c timer_am335x/timer_am335x_module.c
libtimer_am335x_la_SOURCES = $(dev_timer_am335x)
libtimer_am335x_la_LDFLAGS = -module
pkglib_LTLIBRARIES += libtimer_am335x.la
dev_prcm_am335x = prcm_am335x/prcm_am335x.c prcm_am335x/prcm_am335x_module.c
libprcm_am335x_la_SOURCES = $(dev_prcm_am335x)
libprcm_am335x_la_LDFLAGS = -module
pkglib_LTLIBRARIES += libprcm_am335x.la
dev_intc_am335x = intc_am335x/intc_am335x.c intc_am335x/intc_am335x_module.c
libintc_am335x_la_SOURCES = $(dev_intc_am335x)
libintc_am335x_la_LDFLAGS = -module
pkglib_LTLIBRARIES += libintc_am335x.la

dev_xdht_uart = uart_xdht/uart_xdht.c uart_xdht/uart_xdht_module.c
libuart_xdht_la_SOURCES = $(dev_xdht_uart)
libuart_xdht_la_LDFLAGS = -module
pkglib_LTLIBRARIES += libuart_xdht.la

dev_am335x_gpio = gpio_am335x/gpio_am335x.c gpio_am335x/gpio_am335x_module.c
libgpio_am335x_la_SOURCES = $(dev_am335x_gpio)
libgpio_am335x_la_LDFLAGS = -module
pkglib_LTLIBRARIES += libgpio_am335x.la

dev_xdht_intc = intc_xdht/intc_xdht.c intc_xdht/intc_xdht_module.c
libintc_xdht_la_SOURCES = $(dev_xdht_intc)
libintc_xdht_la_LDFLAGS = -module
pkglib_LTLIBRARIES += libintc_xdht.la

dev_am335x_uart = uart_am335x/uart_am335x.c uart_am335x/uart_am335x_module.c
libuart_am335x_la_SOURCES = $(dev_am335x_uart)
libuart_am335x_la_LDFLAGS = -module
pkglib_LTLIBRARIES += libuart_am335x.la

dev_uart_s3c6410 = uart_s3c6410/uart_s3c6410.c uart_s3c6410/uart_s3c6410_module.c
libuart_s3c6410_la_SOURCES = $(dev_uart_s3c6410)
libuart_s3c6410_la_LDFLAGS = -module
pkglib_LTLIBRARIES += libuart_s3c6410.la
