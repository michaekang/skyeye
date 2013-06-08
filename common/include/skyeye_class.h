#ifndef __SKYEYE_CLASS_H__
#define __SKYEYE_CLASS_H__
#include <skyeye_obj.h>
#include <skyeye_attr.h>
typedef enum{
	/* Need to be saved for checkpointing */
	Class_Persistent,
	/* Do not need to be saved for checkpointing */
	Class_temp
}class_kind_t;

typedef struct skyeye_class{
	char* class_name;
	char* class_desc;
	conf_object_t* (*new_instance)(char* obj_name);
	exception_t (*free_instance)(char* obj_name);
	exception_t (*reset_instance)(conf_object_t* obj, const char* parameter);
	attr_value_t* (*get_attr)(const char* attr_name, conf_object_t* obj);
	exception_t (*set_attr)(conf_object_t* obj, const char* attr_name, attr_value_t* value);
	char** interface_list;
}skyeye_class_t;

#ifdef __cplusplus
 extern "C" {
#endif

void SKY_register_class(const char* name, skyeye_class_t* skyeye_class);

conf_object_t* pre_conf_obj(const char* objname, const char* class_name);
exception_t reset_conf_obj(conf_object_t* obj);
#ifdef __cplusplus
}
#endif

//SIM_register_typed_attribute

#endif
