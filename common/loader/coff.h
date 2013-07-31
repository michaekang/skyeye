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
* @file coff.h
* @brief The header of coff format
* @author Michael.Kang blackfin.kang@gmail.com
* @version 7849
* @date 2013-07-31
*/

#ifndef __COFF_H__
#define __COFF_H__

#include <skyeye_types.h>

// TI's COFF v0 file header
typedef struct __attribute__ ((packed))
{
        uint16_t f_magic;     // magic number
        uint16_t f_nscns;     // number of sections
        uint32_t f_timdat;    // time and date stamp
        uint32_t f_symptr;    // file ptr to symtab
        uint32_t f_nsyms;     // number of symtab entries
        uint16_t f_opthdr;    // size of optional header
        uint16_t f_flags;     // flags
} filehdr_v0;

// TI's COFF V1/V2 file header
typedef struct __attribute__ ((packed))
{
        uint16_t f_magic;     // magic number
        uint16_t f_nscns;     // number of sections
        uint32_t f_timdat;    // time and date stamp
        uint32_t f_symptr;    // file ptr to symtab
        uint32_t f_nsyms;     // number of symtab entries
        uint16_t f_opthdr;    // size of optional header
        uint16_t f_flags;     // flags
        uint16_t f_target_id; // COFF-TI specific: TI target magic number that can execute the file
} filehdr_v12;

// Section header for TI's COFF2 files
typedef struct __attribute__ ((packed))
{
        char s_name[8];        // section name
        uint32_t s_paddr;      // physical address
        uint32_t s_vaddr;      // virtual address
        uint32_t s_size;       // section size
        uint32_t s_scnptr;     // file ptr to raw data
        uint32_t s_relptr;     // file ptr to relocation
        uint32_t s_lnnoptr;    // file ptr to line numbers
        uint32_t s_nreloc;     // number of reloc entries
        uint32_t s_nlnno;      // number of line number entries
        uint32_t s_flags;      // flags
        uint16_t s_reserved;   // reserved
        uint16_t s_page;       // memory page number */
} scnhdr_v2;

typedef struct __attribute__ ((packed))
{
        uint16_t o_magic;        // magic number
        uint16_t o_vstamp;       // version stamp
        uint32_t o_tsize;        // text size in bytes, padded to FW bdry
        uint32_t o_dsize;        // initialized data
        uint32_t o_bsize;        // uninitialized data
        uint32_t o_entry;        // entry point
        uint32_t o_text_start;   // base of text used for this file
        uint32_t o_data_start;   // base of data used for this file
} aouthdr;

bool_t is_coff(const char* filename);
exception_t load_coff(const char* filename, addr_type_t addr_type);
#endif
