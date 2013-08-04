#include <Python.h>
#include <stdlib.h>
#include <errno.h>
#include "skyeye_types.h"
#include "skyeye_module.h"
#include "skyeye_cli.h"

#ifdef __MINGW32__
int setenv(const char *name, const char *value, int overwrite)
{
    int result = 0;
    if (overwrite || !getenv(name)) {
        size_t length = strlen(name) + strlen(value) + 2;
        char *string = malloc(length);
        snprintf(string, length, "%s=%s", name, value);
        result = putenv(string);
    }
    return result;
}
#endif

int
cli(const char* prompt)
{
	char new_path[1024];
	char* skyeye_bin = SKYEYE_BIN;
	setenv("SKYEYEBIN", skyeye_bin, 1);
	Py_Initialize();
	sprintf(new_path, "sys.path.append(\"%s\")\n", skyeye_bin);
	//printf("new_path=%s\n", new_path);
	PyRun_SimpleString("import sys\n");
	PyRun_SimpleString(new_path);
	PyRun_SimpleString("from skyeye_cli import *\nSkyEyeCli().cmdloop()\n");
	
	Py_Finalize();
	return 0;
}

int
gui(const char* prompt){

	char new_path[1024];
	char* skyeye_bin = SKYEYE_BIN;
	setenv("SKYEYEBIN", skyeye_bin, 1);
	Py_Initialize();
	PyRun_SimpleString("import sys, os\n");
	sprintf(new_path, "sys.path.append(\"%s\")\n", skyeye_bin);
	PyRun_SimpleString(new_path);
	//PyRun_SimpleString("execfile(os.getenv(\"SKYEYEBIN\") + \"skyeye_xrc.py\")\n");
	PyRun_SimpleString("import skyeye_gui\n");
	PyRun_SimpleString("app = skyeye_gui.SkyEyeGUI()\n");
	PyRun_SimpleString("app.MainLoop()\n");
	Py_Finalize();

	return 0;
}
/* module name */
const char* skyeye_module = "python-cli";

/* module initialization and will be executed automatically when loading. */
void module_init(){
	register_cli(cli);
	register_gui(gui);
}

/* module destruction and will be executed automatically when unloading */
void module_fini(){
}
