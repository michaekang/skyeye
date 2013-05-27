#ifndef __SIMPLEBUSLT_H__
#define __SIMPLEBUSLT_H__

#include <string>
#include <sstream>
#include <systemc>
#include "tlm.h"
//
#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "bus/lt_bus_initiator.h"
#include "bus/lt_bus_target.h"

struct PortIdList{
        int start_address;
        int len;
        struct PortIdList *next;
};

typedef struct PortIdList PortIdList;
static PortIdList *ptrHead;
static PortIdList *ptrCurrNode;
//const char* target_name = "simple_bus_target";
//const char* initiator_name = "simple_bus_initiator";

template <int NR_OF_INITIATORS, int NR_OF_TARGETS>
class SimpleBusLT : public sc_core::sc_module
{
public:
    typedef tlm::tlm_generic_payload                 transaction_type;
    typedef tlm_utils::simple_target_socket_tagged<SimpleBusLT>    target_socket_type;
    typedef tlm_utils::simple_initiator_socket_tagged<SimpleBusLT> initiator_socket_type;
	

public:
    target_socket_type target_socket[NR_OF_INITIATORS];
	//Lt_bus_target* bus_target_ptr[NR_OF_TARGETS];
	//Lt_bus_initiator* bus_initiator_ptr[NR_OF_INITIATORS];
    initiator_socket_type initiator_socket[NR_OF_TARGETS];

public:
    SC_HAS_PROCESS(SimpleBusLT);
    SimpleBusLT(sc_core::sc_module_name name) :
      sc_core::sc_module(name)
    {
	sc_module_name* module_name_ptr;
        for (unsigned int i = 0; i < NR_OF_INITIATORS; ++i) {
		target_socket[i].register_b_transport(this, &SimpleBusLT::initiatorBTransport, i);
              //target_socket[i].register_transport_dbg(this, &SimpleBusLT::transportDebug, i);
              //target_socket[i].register_get_direct_mem_ptr(this, &SimpleBusLT::getDMIPointer, i);
		//bus_target_ptr[i] = new Lt_bus_target(new sc_module_name(s.str().c_str()));
		/*
		assert(i < 10);
		char* name = (char*)malloc(strlen("simple_bus_target") + 3);
		sprintf(name, "%s_%d", "simple_bus_target", i);
		module_name_ptr = new sc_module_name(name);
		bus_target_ptr[i] = new Lt_bus_target(*module_name_ptr);
		*/
        }
        for (unsigned int i = 0; i < NR_OF_TARGETS; ++i) {
	#if 0
            //initiator_socket[i].register_invalidate_direct_mem_ptr(this, &SimpleBusLT::invalidateDMIPointers, i);
		assert(i < 10);
		char* name = (char*)malloc(strlen("simple_bus_initiator") + 3);
		sprintf(name, "%s_%d", "simple_bus_initiator", i);
		module_name_ptr = new sc_module_name(name);
		//sc_module_name module_name(s.str().c_str());
		//bus_initiator_ptr[i] = new Lt_bus_initiator(new sc_module_name(s.str().c_str()));
		bus_initiator_ptr[i] = new Lt_bus_initiator(*module_name_ptr);
	#endif
        }     
       ptrHead = NULL;
       ptrCurrNode = NULL;       
    }

    void addDevice(int startAdd, int len)
    {
	if(NULL == ptrHead){
	    ptrHead = new PortIdList;
	    ptrCurrNode = new PortIdList;
	    ptrCurrNode->start_address = startAdd;
	    ptrCurrNode->len = len;
	    ptrHead->next = ptrCurrNode;
	    ptrCurrNode->next = NULL;
	}else{
	    ptrCurrNode->next = new PortIdList;   
	    ptrCurrNode->next->start_address = startAdd;
	    ptrCurrNode->next->len  = len;
	    ptrCurrNode = ptrCurrNode->next;
	    ptrCurrNode->next = NULL;
	}
    }

    int getPortId(const sc_dt::uint64& address)
    {
	PortIdList *ptrNode = ptrHead->next;
        int portId = 0;
        while(NULL != ptrNode){
	    if(ptrNode->start_address <= address && address < ptrNode->start_address+ptrNode->len)
	        return portId;
	    else{
	        portId++;														                                     ptrNode = ptrNode->next;             
	    }
	}
	return -1;
    }

    unsigned int decode(const sc_dt::uint64& address)
    {
		        
	    return getPortId(address);
    }
#if 1
    void initiatorBTransport(int SocketId,
		    transaction_type& trans,
		    sc_core::sc_time& t)
    {
	    initiator_socket_type* decodeSocket;
	    unsigned int portId = decode(trans.get_address());
	    if(portId >= NR_OF_TARGETS)
	    	printf("address  %d  portId %d\n", trans.get_address(),portId);
	    assert(portId < NR_OF_TARGETS);
	    decodeSocket = &initiator_socket[portId];
	    trans.set_address(trans.get_address());
	    (*decodeSocket)->b_transport(trans, t);
    }
#endif
};

#endif
