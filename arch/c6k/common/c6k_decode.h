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
* @file c6k_decode.h
* @brief some definition for decoder
* @author Michael.Kang blackfin.kang@gmail.com
* @version 7849
* @date 2013-07-31
*/

#ifndef __DECODER_H__
#define __DECODER_H__
typedef int(*insn_action_t)(c6k_core_t* core, uint32_t insn);
struct instruction_set_encoding_item {
        const char *name;
        int attribute_value;
        int version;
        int content[21];
	insn_action_t action;
};
typedef struct instruction_set_encoding_item ISEITEM;

enum DECODE_STATUS {
	DECODE_SUCCESS,
	DECODE_FAILURE
};
#define BITS(a,b) ((insn >> (a)) & ((1 << (1+(b)-(a)))-1))
#define BIT(n) ((instr >> (n)) & 1)
#define SIGN_EXTEND(cst, n) ((cst & (1 << (n - 1))) ? (cst | (0xFFFFFFFF << n)) : cst)
#define DSZ(header) ((header >> 16) & 0x7)
#endif
