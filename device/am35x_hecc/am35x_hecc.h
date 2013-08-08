/* Copyright (C) 
* 2011 - Michael.Kang blackfin.kang@gmail.com
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
* @file am35x_hecc.h
* @brief The definition of HEXX for am35x
* @author Michael.Kang blackfin.kang@gmail.com
* @version 78.77
* @date 2011-12-12
*/

#ifndef __AM35X_HEXX_H__
#define __AM35X_HECC_H__

typedef struct hecc_reg{
	uint32_t dcan_ctl;
	uint32_t dcan_es;
	uint32_t dcan_errc;

	uint32_t dcan_nwdat;
	uint32_t if1_cmd;
	uint32_t if2_cmd;
	uint32_t if1_mask;
	uint32_t if2_mask;
	uint32_t if1_arb;
	uint32_t if2_arb;
	uint32_t if1_mctl;
	uint32_t if2_mctl;
	uint32_t if1_data_a;
	uint32_t if2_data_a;
	uint32_t if1_data_b;
	uint32_t if2_data_b;
	uint32_t can_tioc;
	uint32_t can_rioc;
}hecc_reg_t; 

typedef struct message_obj{
	int MsgVal;
	int UMask;
	int ID;
	int Msk;
	int Xtd;
	int Dir;
	int MDir;
	int EOB;
	int NewDat;
	int MsgLst;
	int RxIE;
	int TxIE;
	int IntPnd;
	int RmtEn;
	int TxRqst;
	int DLC;
	uint8_t Data[8];
}message_obj_t;

typedef struct am35x_hecc_device{
	conf_object_t* obj;
	hecc_reg_t* regs;
	can_ops_intf* can_ops;
	message_obj_t message[64];
}am35x_hecc_device;

#endif
