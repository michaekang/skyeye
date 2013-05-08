#include "lt_top.h"
#include "lt_target.h"
#include "lt_transform.h"
#if 1 
Lt_top::Lt_top(sc_core::sc_module_name module_name)
:sc_module    
(module_name
)
,core_initiator("core_initiator")
,core_target("core_target")
,mem_initiator("mem_initiator")
,mem_target("mem_target")
,uart_initiator("uart_initiator")
,uart_target("uart_target")
,arm_initiator
("arm_initiator"
)
{
	//printf("In %s\n", __FUNCTION__);
	//arm_initiator.trans_ptr->initiator_socket.bind(mem_target.memop_socket);
	core_initiator.initiator_socket(core_target.target_socket);
	mem_initiator.initiator_socket(mem_target.target_socket);
	uart_initiator.initiator_socket(uart_target.target_socket);
};
#endif
//Lt_top::Lt_top(){};
