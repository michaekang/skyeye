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
* @file loader_coff.c
* @brief load the coff executable file
* @author Michael.Kang blackfin.kang@gmail.com
* @version 7849
* @date 2013-07-31
*/

#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "portable/mman.h"
#include "bank_defs.h"
#include "skyeye_pref.h"
#include "skyeye_config.h"
#include "skyeye_ram.h"
#include "coff.h"
// magic numbers for big-endian headers
const uint16_t BEH_COFF_V1_MAGIC = 0xc100;
const uint16_t BEH_COFF_V2_MAGIC = 0xc200;
const uint16_t BEH_AOUT_MAGIC = 0x0801;

// TI's IDs for big-endian headers
const uint16_t BEH_TARGET_ID_TMS320_C1X_C2X_C5X = 0x9200;
const uint16_t BEH_TARGET_ID_TMS320_C3X_C4X = 0x9300;
const uint16_t BEH_TARGET_ID_C80 = 0x9500;
const uint16_t BEH_TARGET_ID_TMS320_C54X = 0x9800;
const uint16_t BEH_TARGET_ID_TMS320_C6X = 0x9900;
const uint16_t BEH_TARGET_ID_TMS320_C28X = 0x9d00;

#if 0

const char get_arch_name(short magic)
{
        switch(magic)
        {
                case LEH_TARGET_ID_TMS320_C1X_C2X_C5X:
                case BEH_TARGET_ID_TMS320_C1X_C2X_C5X:
                        return "TI TMS320C1x/C2x/C5x";
                case LEH_TARGET_ID_TMS320_C3X_C4X:
                case BEH_TARGET_ID_TMS320_C3X_C4X:
                        return "TI TMS320C3x/C4x";
                case LEH_TARGET_ID_C80:
                case BEH_TARGET_ID_C80:
                        return "TI C80";
                case LEH_TARGET_ID_TMS320_C54X:
                case BEH_TARGET_ID_TMS320_C54X:
                        return "TI TMS320C54x";
                case LEH_TARGET_ID_TMS320_C6X:
                case BEH_TARGET_ID_TMS320_C6X:
                        return "TI TMS320C6x";
                case LEH_TARGET_ID_TMS320_C28X:
                case BEH_TARGET_ID_TMS320_C28X:
                        return "TI TMS320C28x";
                case LEH_COFF_V1_MAGIC:
                case BEH_COFF_V1_MAGIC:
                case LEH_COFF_V2_MAGIC:
                case BEH_COFF_V2_MAGIC:
                        switch(target_id)
                        {
                                case TARGET_ID_TMS320_C1X_C2X_C5X:
                                        return "TI TMS320C1x/C2x/C5x";
                                case TARGET_ID_TMS320_C3X_C4X:
                                        return "TI TMS320C3x/C4x";
                                case TARGET_ID_C80:
                                        return "TI C80";
                                case TARGET_ID_TMS320_C54X:
                                        return "TI TMS320C54x";
                                case TARGET_ID_TMS320_C6X:
                                        return "TI TMS320C6x";
                                case TARGET_ID_TMS320_C28X:
                                        return "TI TMS320C28x";
                        }
                        break;
        }

        return "?";
}

unsigned int get_filehdr_size(short ti_coff_version)
{
        switch(ti_coff_version)
        {
                case 0:
                        return sizeof(filehdr_v0);
                case 1:
                case 2:
                        return sizeof(filehdr_v12);
        }
        return 0;
}
#endif
bool_t is_coff(const char* filename){
	filehdr_v12 header;
	bool_t ret = False;
	FILE* file = fopen(filename, "r");
	if(file == NULL){
		fprintf(stderr, "In %s, can not open file %s\n", __FUNCTION__, filename);
		exit(-1);
	}

	if (fread (&header, sizeof(filehdr_v12), 1, file) != 1){
#if __WIN32__
		fprintf(stderr, "Workaround for windows fopen function in %s\n", __FUNCTION__);
#else
		goto out;
#endif
	}
	printf("In %s, header.f_magic = 0x%x\n", __FUNCTION__, header.f_magic);
	if (header.f_magic != 0xc1
		&& header.f_magic != 0xc2
		)
		ret = False;
	else
		ret = True;
out:
	fclose(file);
	return ret;
}

exception_t load_coff(const char* filename, addr_type_t addr_type){
	//1334
	struct coff_header *coffFile;
	struct stat coff_fstat;
	int tmp_fd = open(filename, O_RDONLY);
	if (tmp_fd == -1) {
		fprintf (stderr, "open %s error: %s\n", filename, strerror(errno));
		goto out;
	}
	fstat(tmp_fd, &coff_fstat);
	   /* malloc */
	coffFile = mmap(NULL, coff_fstat.st_size, PROT_READ, MAP_PRIVATE, tmp_fd, 0);
	if (coffFile == NULL || coffFile == MAP_FAILED) {
		fprintf (stderr, "mmap error: %s\n", strerror(errno));
		goto out;
	}
	filehdr_v12* filehdr = ((char*)coffFile + 0x0);
	aouthdr* aout_header = ((char*)coffFile + sizeof(filehdr_v12));
	#if 0
	printf("In %s, num_sections=%d, magic=0x%x, target_id=0x%x\n", __FUNCTION__, filehdr->f_nscns, filehdr->f_magic, filehdr->f_target_id);
	printf("In %s, num_sections=%d\n", __FUNCTION__, filehdr->f_nscns);
	printf("In %s, sizeof(filehdr_v12)=0x%x, entry=0x%x, text_start=0x%x\n", __FUNCTION__, sizeof(filehdr_v12), aout_header->o_entry, aout_header->o_text_start);
	#endif
	scnhdr_v2* section_header;
	//section_header = (scnhdr_v2 *)((char*)coffFile + sizeof(filehdr_v12) + sizeof(aouthdr));
	int section_num = filehdr->f_nscns;
	
	int i;
	for ( i = 0; i < section_num; i++){	
		section_header = (scnhdr_v2 *)((char*)coffFile + sizeof(filehdr_v12) + sizeof(aouthdr) + i * sizeof(scnhdr_v2));
		if(section_header->s_size != 0 && section_header->s_scnptr != 0 && section_header->s_paddr != 0){
			//printf("In %s, section_header name=%c%c%c%c%c%c%c, paddr=0x%x, vaddr=0x%x, size=0x%x, offset=0x%x\n", __FUNCTION__, section_header->s_name[0], section_header->s_name[1], section_header->s_name[2], section_header->s_name[3], section_header->s_name[4], section_header->s_name[5], section_header->s_name[6], section_header->s_paddr, section_header->s_vaddr, section_header->s_size, section_header->s_scnptr);
			//printf("In %s, section_header name=%s, paddr=0x%x, vaddr=0x%x, size=0x%x, offset=0x%x\n", __FUNCTION__, section_header->s_name, section_header->s_paddr, section_header->s_vaddr, section_header->s_size, section_header->s_scnptr);
			uint8_t* start = (uint8_t *)((char*)coffFile + section_header->s_scnptr);
			int j = 0;
		#if 1
			for (; j < section_header->s_size; j++) {
				if(bus_write(8, section_header->s_paddr + j, start[j]) != 0){
					printf("SKYEYE: write physical address 0x%x error!!!\n", section_header->s_paddr + j);
					return Excess_range_exp;
				}
			}
		#endif
		}
	}
#if 0
	uint8_t* text_start = (uint8_t *)((char*)coffFile + 0x536);
	printf("The first instrution is 0x%x\n", *(unsigned long *)text_start);
	/* text loading */
	uint32_t load_addr = 0x810000;
	uint32_t length = 0x66E0;
	i = 0;
#if 1
	for (; i < length; i++) {
		if(bus_write(8, load_addr + i, text_start[i]) != 0){
			printf("SKYEYE: write physical address 0x%x error!!!\n", load_addr + i);
			return Excess_range_exp;
		}
	}
#endif
	/* cinit */
	//load_addr = 0x817b68;
	load_addr = 0x817b48;
	length = 0x154;
	//uint8_t* cinit_start = (uint8_t *)((char*)coffFile + 0x6d76);
	//uint8_t* cinit_start = (uint8_t *)((char*)coffFile + 0x6d56);
	uint8_t* cinit_start = (uint8_t *)((char*)coffFile + 0x6d48);
	i = 0;
	for (; i < length; i++) {
		if(bus_write(8, load_addr + i, cinit_start[i]) != 0){
			printf("SKYEYE: write physical address 0x%x error!!!\n", load_addr + i);
			return Excess_range_exp;
		}
	}
	printf("data at cinit is 0x%x\n", *(unsigned long *)cinit_start);
	/* const */
	load_addr = 0x817ca0;
	length = 0x132;
	uint8_t* const_start = (uint8_t *)((char*)coffFile + 0x6c16);
	i = 0;
	for (; i < length; i++) {
		if(bus_write(8, load_addr + i, const_start[i]) != 0){
			printf("SKYEYE: write physical address 0x%x error!!!\n", load_addr + i);
			return Excess_range_exp;
		}
	}
	printf("data at const is 0x%x\n", *(unsigned long *)const_start);
#endif
	sky_pref_t* pref = get_skyeye_pref();
	unsigned long load_base = pref->exec_load_base;
	unsigned long load_mask = pref->exec_load_mask;
	pref->exec_load_base = 0x0;
	pref->exec_load_mask = 0xFFFFFFFF;
	skyeye_config_t* config = get_current_config();
	//config->start_address = 0x816320;
	config->start_address = aout_header->o_entry;

out:
	close(tmp_fd);
	//sleep(10);
	return No_exp;
}
