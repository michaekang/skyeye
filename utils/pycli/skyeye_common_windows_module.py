#### SkyEye common module ####
import os
from ctypes import *

libcommon_path = "C:/skyeye/1.0/opt/skyeye/bin/libcommon-0.dll"

libcommon = CDLL(libcommon_path, RTLD_GLOBAL)

#### SkyEye other modules ####
libdisasm_path = "C:/skyeye/1.0/opt/skyeye/lib/skyeye/libdisasm-0.dll"
libpmon_path = "C:/skyeye/1.0/opt/skyeye/lib/skyeye/libpmon-0.dll"
libuart_16550_path = "C:/skyeye/1.0/opt/skyeye/lib/skyeye/libuart_16550-0.dll"
libcodecov_path = "C:/skyeye/1.0/opt/skyeye/lib/skyeye/libcodecov-0.dll"
libbus_log_path = "C:/skyeye/1.0/opt/skyeye/lib/skyeye/libbus_log-0.dll"
libgdbserver_path = "C:/skyeye/1.0/opt/skyeye/lib/skyeye/libgdbserver-0.dll"

# Generate CDLL handlers
libdisasm = CDLL(libdisasm_path, RTLD_LOCAL)
libpmon = CDLL(libpmon_path, RTLD_LOCAL)
libuart_16550 = CDLL(libuart_16550_path, RTLD_LOCAL)
libcodecov = CDLL(libcodecov_path, RTLD_LOCAL)
libbus_log = CDLL(libbus_log_path, RTLD_LOCAL)
libgdbserver = CDLL(libgdbserver_path, RTLD_LOCAL)
