/* Copyright (C) 
* 
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
* 
*/
/**
* @file intc_xdhtx_module.c
* @brief intc_xdhtx module for
* @author Kewei Yu : keweihk@gmail.com
* @version 78.77
*/

#include <skyeye_module.h>

const char* skyeye_module = "intc_xdht";

extern void init_intc_xdht();

void module_init(){
	init_intc_xdht();
}

void module_fini(){
}
