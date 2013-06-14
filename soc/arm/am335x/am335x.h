/*
 * am335x.h - definitions of "am355x" machine  for skyeye
 *
 * Copyright (C) 2012 Kewei Yu <keweihk@gmail.com>
 * Skyeye Develop Group, for help please send mail to
 * <skyeye-developer@lists.gro.clinux.org>
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __AM355X_H__
#define __AM355X_H__
#include <skyeye_types.h>
#include <skyeye_class.h>
#include <skyeye_addr_space.h>
#include <skyeye_mach.h>
#include "skyeye_internal.h"
#include <skyeye_interface.h>
#include <skyeye_lcd_intf.h>
//#define DEBUG
#include <skyeye_log.h>
#include <skyeye_uart.h>
#include <skyeye_arch.h>
#include <skyeye_mm.h>
#include <skyeye_core_intf.h>
#include "am335x.h"

typedef struct am335x_mach{
	conf_object_t* obj;
	addr_space_t* space;
}am335x_mach_t;

#endif /* __OMAP4460X_H__ */
