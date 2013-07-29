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
* @file sim_ctrl.c
* @brief the control function set of the simulator
* @author Michael.Kang blackfin.kang@gmail.com
* @version 
* @date 2011-04-30
*/

#include <stdlib.h>
#include <sim_control.h>
#include "skyeye_types.h"
#include "skyeye_config.h"
#include "skyeye_pref.h"
#include "skyeye_module.h"
#include "skyeye_arch.h"
#include "skyeye_callback.h"
#include "skyeye_cell.h"
#include "skyeye_internal.h"
#include "skyeye_mm.h"
#include "skyeye_command.h"
#include "skyeye_loader.h"
#include "skyeye_symbol.h"
#include "skyeye_log.h"
#include "skyeye_cli.h"
#include "skyeye_class.h"
#include "portable/portable.h"
/* FIXME, we should get it from prefix varaible after ./configure */
#ifndef SKYEYE_MODULE_DIR
const char* default_lib_dir = "/opt/skyeye/lib/skyeye/";
#else
const char* default_lib_dir = SKYEYE_MODULE_DIR;
#endif

/**
* @brief the default cell of the simulator
*/
static skyeye_cell_t* default_cell = NULL;

/**
* @brief the flag of running or stop
*/
static bool_t SIM_running = False;
static bool_t SIM_started = False;
void SIM_init_command_line(void){
}

/**
* @brief initialization of environment
*
* @param argv
* @param handle_signals
*/
void SIM_init_environment(char** argv, bool_t handle_signals){
}

void SIM_main_loop(void){
}

/* we try to do init in batch mode */
static exception_t try_init(){
	return No_exp;
}

//void SIM_cli();
cli_func_t global_cli = NULL;
cli_func_t global_gui = NULL;

/**
* @brief all the initilization of the simulator
*/
void SIM_init(){
	sky_pref_t* pref;
	char* welcome_str = get_front_message();
	/* 
	 * get the corrent_config_file and do some initialization 
	 */
	skyeye_config_t* config = get_current_config();
	skyeye_option_init(config);
	/*
	 * initialization of callback data structure, it needs to 
	 * be initialized at very beginning.
	 */
	init_callback();

	/*
	 * initilize the data structure for command
	 * register some default built-in command
	 */
	init_command_list();

	init_stepi();

	/*
	 * initialization of module manangement 
	 */
	init_module_list();
	
	/*
	 * initialization the timer scheduler 
	 */
	init_thread_scheduler();
	//init_timer_scheduler();

	
	/*
	 * initialization of architecture and cores
	 */
	init_arch();
	
	/*
	 * initialization of bus and memory module
	 */
	init_bus();


	/*
	 * initialization of machine module
	 */
	init_mach();

	
	/*
	 * initialization of breakpoint, that depends on callback module.
	 */
	init_bp();

	/* the management of named object */
	//init_conf_obj();

	/*
	 * initialization of breakpoint, that depends on callback module.
	 */
	init_chp();

	/* 
	 * get the current preference for simulator 
	 */
	pref = get_skyeye_pref();

	/* 
	 * loading all the modules in search directory 
	 */
	if(!pref->module_search_dir)
		pref->module_search_dir = skyeye_strdup(default_lib_dir);
	SKY_load_all_modules(pref->module_search_dir, NULL);

	/* save the original termios */
	struct termios tmp;
	tcgetattr(0, &tmp);
	memcpy(&pref->saved_term, &tmp, sizeof(struct termios));

	/*
	 * if we run simulator in GUI or external IDE, we do not need to
	 * launch our CLI.
	 */
	if(pref->gui_mode == True)
	{
		if(global_gui != NULL){
			global_gui("SkyEye");
			exit(0);
		}else
			printf("No gui found\n");
	}
	if(pref->interactive_mode == True){
		if(global_cli != NULL)
			global_cli("SkyEye");
		else
			printf("No cli found\n");
	}
	else{
		if (pref->autoboot == True) {
			SIM_start();
			SIM_run();
		}
	}
}

/**
* @brief launch the simlator
*/
void SIM_start(void){
	if(SIM_started){
		printf("SkyEye has been started\n");
		return;
	}
	sky_pref_t *pref;
	/* get the current preference for simulator */
	pref = get_skyeye_pref();
	skyeye_config_t* config = get_current_config();
	if(pref->conf_filename)
		skyeye_read_config(pref->conf_filename);

	if(config->arch == NULL){
		skyeye_log(Error_log, __FUNCTION__, "Should provide valid arch option in your config file.\n");
		return;
	}
	generic_arch_t *arch_instance = get_arch_instance(config->arch->arch_name);

	if(config->mach != NULL){
		/* reset all the memory */
		mem_reset();
		arch_instance->init();

		config->mach->mach_init(arch_instance, config->mach);
		/* reset all the values of mach */
		io_reset(arch_instance);
		/* reset current arch_instance */
		arch_instance->reset();
	}

	if(pref->exec_file){
		exception_t ret;
		/* 
		 * If no relocation is indicated, we will load elf file by 
		 * virtual address
		 */
		//if((((~pref->exec_load_mask) & pref->exec_load_base) == 0x0) &&
		if(((pref->exec_load_mask) == 0x0) &&
			(arch_instance->mmu_write != NULL))
			ret = SKY_load_elf(pref->exec_file, Virt_addr);
		else
			ret = SKY_load_elf(pref->exec_file, Phys_addr);
		if(ret == No_exp){
			printf("load elf %s succeed\n", pref->exec_file);
		}else{
			printf("load elf %s fault\n", pref->exec_file);
			return;
		}
	}
#ifndef __WIN32__
	init_symbol_table(pref->exec_file, arch_instance->arch_name);
#endif

	/* set pc from config */
	generic_address_t pc = (config->start_address & pref->exec_load_mask)|pref->exec_load_base; 
	skyeye_log(Info_log, __FUNCTION__, "Set PC to the address 0x%x\n", pc);
	arch_instance->set_pc(pc);

	/* Call bootmach callback */
	exec_callback(Bootmach_callback, arch_instance);	
	SIM_started = True;

		
	//create_thread(skyeye_loop, arch_instance, &id);
	
	/* 
	 * At this time, if we set conf file, then we parse it
	 * Or do it later.
	 */

	/*
	if(pref->conf_filename)
		skyeye_read_config(pref->conf_filename);
	*/
#if 0	
	else{
		/* try to run in batch mode */
		if(skyeye_read_config(pref->conf_filename) == No_exp){
		/* if can not */
			init_threads();
			SIM_run();
		}
	}
#endif	
}

/**
* @brief all the cli
*/
void SIM_cli(){
	skyeye_cli();
}

/**
* @brief set the running state for the simulator
*/
void SIM_run(){
	//skyeye_start();
	if(!SIM_started){
		printf("You didn't input start!\n");
		return;
	}
	SIM_running = True;
	start_all_cell();
}
#if 0
void SIM_break_simulation(const char *msg){
}
void SIM_pause(){
	skyeye_pause();
}
#endif

/**
* @brief continue the last stop state
*
* @param arch_instance
*/
void SIM_continue(generic_arch_t* arch_instance){
	//skyeye_continue();
	SIM_running = True;
	start_all_cell();
}

/**
* @brief stop the simulator
*
* @param arch_instance
*/
void SIM_stop(generic_arch_t* arch_instance){
	//skyeye_pause();
	SIM_running = False;
	stop_all_cell();
}

/**
* @brief if the simulator is in running state
*
* @param arch_instance
*/

bool_t SIM_is_running(){
	return SIM_running;
}

/**
* @brief destructor of the simulator
*/
extern void skyeye_erase_map(void);
void SIM_fini(){
	sky_pref_t *pref = get_skyeye_pref();
	//pthread_cancel();
	generic_arch_t* arch_instance = get_arch_instance("");
	SIM_stop(arch_instance);
	/* Call SIM_exit callback */
        exec_callback(SIM_exit_callback, arch_instance);
	printf("Destroy threads.\n");
	destroy_threads();
	printf("Unload all modules.\n");
	/* unload all the module */
	SKY_unload_all_modules();
	/* free the memory */
#ifndef __MINGW32__
	skyeye_erase_map();
#endif
	printf("exit.\n");

#ifndef __MINGW32__
	/* restore the environment */
	tcsetattr(0, TCSANOW, &pref->saved_term);
#else
	tcsetattr(0, TCSAFLUSH, &pref->saved_term);
#endif

	return;
	//exit(0);
}
exception_t space_obj_free(addr_space_t* addr_space);
void reset_arch(void);
void del_from_cell(void);
void reset_skyeye_config(void);
void SIM_restart(void){
	if(!SIM_started){
		printf("SkyEye is very pure, you don't need to restart it!\n");
		return;
	}
	skyeye_config_t* config = get_current_config();
	if(config->mach == NULL){
		return;
	}
	sky_pref_t *pref = get_skyeye_pref();
	space_obj_free(config->mach->phys_mem);
	//pthread_cancel();
	generic_arch_t* arch_instance = get_arch_instance("");
	SIM_stop(arch_instance);
	/* Call SIM_exit callback */
	exec_callback(SIM_exit_callback, arch_instance);
	printf("Destroy threads.\n");
	//destroy_threads();
	/* unload all the module */
	SKY_unload_all_modules();

	reset_skyeye_config();
	/* reset mem map */
	reset_global_memmap();
	/* reset arch */
	reset_arch();
	/* free exec from cell */
	del_from_cell();
	/* free the memory */
	skyeye_erase_map();

	if(!pref->module_search_dir)
		pref->module_search_dir = skyeye_strdup(default_lib_dir);
	SKY_load_all_modules(pref->module_search_dir, NULL);

#ifndef __MINGW32__
	/* restore the environment */
	tcsetattr(0, TCSANOW, &pref->saved_term);
#else
	tcsetattr(0, TCSAFLUSH, &pref->saved_term);
#endif
	SIM_started = False;
}
#if 1
void register_cli(cli_func_t cli){
	global_cli = cli;
}
void register_gui(cli_func_t gui){
	global_gui= gui;
}
#endif
