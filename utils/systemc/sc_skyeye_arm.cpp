/* Copyright (C) 
* 2013 - Michael.Kang blackfin.kang@gmail.com
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
* @file sc_skyeye_arm.cpp
* @brief The running of processor core
* @author Michael.Kang blackfin.kang@gmail.com
* @version 7849
* @date 2013-05-07
*/

#include "sc_skyeye_arm.h"

#define QUTA 1000 * 1000
void sc_skyeye_arm::machine_start()
{
	int insn_num = 0;
	while(1){
		//printf("In %s\n", __FUNCTION__);
		//wait(10, SC_NS);
		//wait();
		skyeye_stepi(QUTA);
		insn_num += QUTA;
		//printf("In %s, total insn number is %d\n", __FUNCTION__, insn_num);
		
	}
}
