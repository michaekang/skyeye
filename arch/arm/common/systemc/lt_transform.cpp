#include "lt_transform.h"

Lt_transform::Lt_transform(sc_core::sc_module_name module_name)
:sc_module   (module_name)
,initiator_socket("initiator_socket")
{
	gp_ptr = new tlm::tlm_generic_payload();
}
Lt_transform::~Lt_transform()
{
}


fault_t Lt_transform::sc_a71_mmu_read(ARMul_State * state, ARMword virt_addr, ARMword * data, ARMword datatype)
{
#if 1 
	tlb_entry_t *tlb;
	ARMword phys_addr;
	ARMword temp, offset;
	fault_t fault;
	ARMword data_size;

	gp_ptr->set_command(tlm::TLM_READ_COMMAND);
	gp_ptr->set_data_ptr((unsigned char*)data);
	if (!(state->mmu.control & CONTROL_MMU)) {

//              *data = mem_read_word(state, virt_addr);

		if (datatype == ARM_BYTE_TYPE)
			data_size = 8;

		else if (datatype == ARM_HALFWORD_TYPE)
			data_size = 16;

		else if (datatype == ARM_WORD_TYPE)
			data_size = 32;

		else {
			printf ("SKYEYE:1 a71_mmu_read error: unknown data type %d\n", datatype);
			skyeye_exit (-1);
		}
		
		gp_ptr->set_address(virt_addr);
		gp_ptr->set_data_length(data_size);
		gp_ptr->set_streaming_width(data_size);
		gp_ptr->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
		initiator_thread( gp_ptr );

		return NO_FAULT;
	}

#if 0
/* XXX */
	if (hack && (virt_addr >= 0xc0000000) && (virt_addr < 0xc0200000)) {
		printf ("0x%08x\n", virt_addr);
	}

#endif /*  */
	if ((virt_addr & 3) && (datatype == ARM_WORD_TYPE)
	    && (state->mmu.control & CONTROL_ALIGN_FAULT) || (virt_addr & 1)
	    && (datatype == ARM_HALFWORD_TYPE)
	    && (state->mmu.control & CONTROL_ALIGN_FAULT)) {
		fprintf (stderr, "SKYEYE, a71_mmu_read ALIGNMENT_FAULT\n");
		return ALIGNMENT_FAULT;
	}
	if (state->mmu.control & CONTROL_CACHE) {
		cache_line_t *cache;
		cache = mmu_cache_search (state, CACHE (), virt_addr);
		if (cache) {
			if (datatype == ARM_WORD_TYPE)
				*data = cache->data[(virt_addr >> 2) & 3];

			else if (datatype == ARM_HALFWORD_TYPE) {
				temp = cache->data[(virt_addr >> 2) & 3];
				offset = (((ARMword) state->bigendSig * 2) ^ (virt_addr & 2)) << 3;	/* bit offset into the word */
				*data = (temp >> offset) & 0xffff;
			}
			else if (datatype == ARM_BYTE_TYPE) {
				temp = cache->data[(virt_addr >> 2) & 3];
				offset = (((ARMword) state->bigendSig * 3) ^ (virt_addr & 3)) << 3;	/* bit offset into the word */
				*data = (temp >> offset & 0xffL);
			}
			return NO_FAULT;
		}
	}
	fault = translate (state, virt_addr, TLB (), &tlb);
	if (fault) {
		return fault;
	}
	fault = check_access (state, virt_addr, tlb, 1);
	if (fault) {
		return fault;
	}
	phys_addr = (tlb->phys_addr & tlb_masks[tlb->mapping]) |
		(virt_addr & ~tlb_masks[tlb->mapping]);

	/* allocate to the cache if cacheable */
	if ((tlb->perms & 0x08) && (state->mmu.control & CONTROL_CACHE)) {
		cache_line_t *cache;
		ARMword fetch;
		int i;
		cache = mmu_cache_alloc (state, CACHE (), virt_addr, 0);
		fetch = phys_addr & 0xFFFFFFF0;
		for (i = 0; i < 4; i++) {
			//cache->data[i] = mem_read_word (state, fetch);
			bus_read(32, fetch, &cache->data[i]);     
			fetch += 4;
		}
		cache->tag =
			va_cache_align (virt_addr, CACHE ()) | TAG_VALID_FLAG;

		//*data = cache->data[(virt_addr >> 2) & 3];
		if (datatype == ARM_WORD_TYPE)
			*data = cache->data[(virt_addr >> 2) & 3];

		else if (datatype == ARM_HALFWORD_TYPE) {
			temp = cache->data[(virt_addr >> 2) & 3];
			offset = (((ARMword) state->bigendSig * 2) ^ (virt_addr & 2)) << 3;	/* bit offset into the word */
			*data = (temp >> offset) & 0xffff;
		}
		else if (datatype == ARM_BYTE_TYPE) {
			temp = cache->data[(virt_addr >> 2) & 3];
			offset = (((ARMword) state->bigendSig * 3) ^ (virt_addr & 3)) << 3;	/* bit offset into the word */
			*data = (temp >> offset & 0xffL);
		}
		return NO_FAULT;
	}
	else {
		if (datatype == ARM_BYTE_TYPE)
			//*data = mem_read_byte (state, phys_addr);
			//bus_read(8, phys_addr, data);
			data_size = 8;
		else if (datatype == ARM_HALFWORD_TYPE)
			//*data = mem_read_halfword (state, phys_addr);
			//bus_read(16, phys_addr, data);
			data_size = 8;
		else if (datatype == ARM_WORD_TYPE)
			//*data = mem_read_word (state, phys_addr);
			//bus_read(32, phys_addr, data);
			data_size = 8;
		else {
			printf ("SKYEYE:2 a71_mmu_read error: unknown data type %d\n", datatype);
			skyeye_exit (-1);
		}

		/*fill the generic payload*/
		gp_ptr->set_address(virt_addr);
		gp_ptr->set_data_length(data_size);
		gp_ptr->set_streaming_width(data_size);
		gp_ptr->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
		initiator_thread( gp_ptr );
		return NO_FAULT;
	}
}

fault_t Lt_transform::sc_a71_mmu_write(ARMul_State * state, ARMword virt_addr, ARMword* data, ARMword datatype)
{
       tlb_entry_t *tlb;
	ARMword phys_addr;
	fault_t fault;
	ARMword temp, offset;
	ARMword data_size;

	gp_ptr->set_command(tlm::TLM_WRITE_COMMAND);
	gp_ptr->set_data_ptr((unsigned char*)data);
	
	if (!(state->mmu.control & CONTROL_MMU)) {
		if (datatype == ARM_BYTE_TYPE)
			data_size = 8;

		else if (datatype == ARM_HALFWORD_TYPE)
			data_size = 16;

		else if (datatype == ARM_WORD_TYPE)
			data_size = 32;

		else {
			printf ("SKYEYE:1 a71_mmu_write error: unknown data type %d\n", datatype);
			skyeye_exit (-1);
		}

		gp_ptr->set_address(virt_addr);
		gp_ptr->set_data_length(data_size);
		gp_ptr->set_streaming_width(data_size);
		gp_ptr->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

		initiator_thread( gp_ptr );
		return NO_FAULT;
	}

//      if ((virt_addr & 3) && (state->mmu.control & CONTROL_ALIGN_FAULT)) {
	if ((virt_addr & 3) && (datatype == ARM_WORD_TYPE)
	    && (state->mmu.control & CONTROL_ALIGN_FAULT) || (virt_addr & 1)
	    && (datatype == ARM_HALFWORD_TYPE)
	    && (state->mmu.control & CONTROL_ALIGN_FAULT)) {
		fprintf (stderr, "SKYEYE, a71_mmu_write ALIGNMENT_FAULT\n");
		return ALIGNMENT_FAULT;
	}
	if (state->mmu.control & CONTROL_CACHE) {
		cache_line_t *cache;
		cache = mmu_cache_search (state, CACHE (), virt_addr);
		if (cache) {
			if (datatype == ARM_WORD_TYPE)
				cache->data[(virt_addr >> 2) & 3] = *data;

			else if (datatype == ARM_HALFWORD_TYPE) {
				temp = cache->data[(virt_addr >> 2) & 3];
				offset = (((ARMword) state->bigendSig * 2) ^ (virt_addr & 2)) << 3;	/* bit offset into the word */
				cache->data[(virt_addr >> 2) & 3] =
					(temp & ~(0xffffL << offset)) |
					(((*data) & 0xffffL) << offset);
			}
			else if (datatype == ARM_BYTE_TYPE) {
				temp = cache->data[(virt_addr >> 2) & 3];
				offset = (((ARMword) state->bigendSig * 3) ^ (virt_addr & 3)) << 3;	/* bit offset into the word */
				cache->data[(virt_addr >> 2) & 3] =
					(temp & ~(0xffL << offset)) |
					(((*data) & 0xffL) << offset);
			}
		}
	}
	fault = translate (state, virt_addr, TLB (), &tlb);
	if (fault) {
		return fault;
	}
	fault = check_access (state, virt_addr, tlb, 0);
	if (fault) {
		return fault;
	}
	phys_addr = (tlb->phys_addr & tlb_masks[tlb->mapping]) |
		(virt_addr & ~tlb_masks[tlb->mapping]);
	if (datatype == ARM_BYTE_TYPE)
		//bus_write (8, phys_addr, data);
		data_size = 8;

	else if (datatype == ARM_HALFWORD_TYPE)
		//bus_write (16, phys_addr, data);
		data_size = 16;

	else if (datatype == ARM_WORD_TYPE)
		//bus_write (32, phys_addr, data);
		data_size = 32;

	else {
		printf ("SKYEYE:2  a71_mmu_write error: unknown data type %d \n", datatype);
		skyeye_exit (-1);
	}

	gp_ptr->set_address(phys_addr);
	gp_ptr->set_data_length(data_size);
	gp_ptr->set_streaming_width(data_size);
	gp_ptr->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

	initiator_thread( gp_ptr );
#endif
	return NO_FAULT;
}

void Lt_transform::initiator_thread( tlm::tlm_generic_payload * transaction_ptr)
{
	sc_time delay = SC_ZERO_TIME;
	tlm::tlm_response_status gp_status;

	initiator_socket->b_transport(*transaction_ptr, delay);           //transport the payload to target
	gp_status = transaction_ptr->get_response_status();

	if(tlm::TLM_OK_RESPONSE == gp_status)
		wait(delay);
	else
		return;
}
