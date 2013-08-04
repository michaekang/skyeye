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
* @file c6k_regformat.h
* @brief The definition of REGISTER
* @author Michael.Kang blackfin.kang@gmail.com
* @version 7849
* @date 2013-08-02
*/
#ifndef __C6K_REGFORMAT_H__
#define __C6K_REGFORMAT_H__
typedef enum{
	A0 = 0,
	A1,
	A2,
	A3,
	A4,
	A5,
	A6,
	A7,
	A8,
	A9,
	A10,
	A11,
	A12,
	A13,
	A14,
	A15,
	A16,
	A17,
	A18,
	A19,
	A20,
	A21,
	A22,
	A23,
	A24,
	A25,
	A26,
	A27,
	A28,
	A29,
	A30,
	A31,
	B0,
	B1,
	B2,
	B3,
	B4,
	B5,
	B6,
	B7,
	B8,
	B9,
	B10,
	B11,
	B12,
	B13,
	B14,
	B15,
	B16,
	B17,
	B18,
	B19,
	B20,
	B21,
	B22,
	B23,
	B24,
	B25,
	B26,
	B27,
	B28,
	B29,
	B30,
	B31,
	PC_REG,
	CSR_REG,
	TSR_REG,
	ILC_REG,
	RILC_REG
}reg_id_t;
#endif
