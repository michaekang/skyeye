#ifndef __LT_TRANSFORM_H__
#define __LT_TRANSFORM_H__

#include "bank_defs.h"
#include "armdefs.h"
#include "armmmu.h"
#include "systemc.h"
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"

#define	CACHE() (&state->mmu.u.arm7100_mmu.cache_t)
#define TLB() (&state->mmu.u.arm7100_mmu.tlb_t)

class Lt_transform:public sc_core::sc_module{
public:
	Lt_transform(sc_core::sc_module_name module_name);
	//Lt_transform();
	~Lt_transform();

	void initiator_thread(tlm::tlm_generic_payload *transaction_ptr);

	fault_t sc_a71_mmu_read (ARMul_State * state, ARMword virt_addr, ARMword * data,
	      ARMword datatype);

	fault_t sc_a71_mmu_write(ARMul_State * state, ARMword virt_addr, ARMword *data, 
		ARMword datatype);
	
	tlm::tlm_generic_payload *gp_ptr;     //generic payload
	tlm_utils::simple_initiator_socket<Lt_transform> initiator_socket;
	
};

#endif
