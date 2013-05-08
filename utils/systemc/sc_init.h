#ifndef __SC_INIT_H__
#define __SC_INIT_H__
#include <systemc.h>
#include <tlm.h>
#ifdef __cplusplus
 extern "C" {
#endif

void init_systemc_class();

#ifdef __cplusplus
}
#endif
void bus_dispatch(tlm:: tlm_generic_payload & payload, sc_core :: sc_time & delay_time);

#endif
