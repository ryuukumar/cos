#include <kernel/memmgt.h>

/*!
 * Get a vaddr_t object from a virtual pointer
 * @param ptr the virtual pointer
 * @return corresponding vaddr_t object
 */
vaddr_t get_vaddr_t_from_ptr (void* ptr) {
	uint64_t ptr_64t = (uint64_t)ptr;
	vaddr_t	 ret_vaddr;

	ret_vaddr.offset = ptr_64t & 0xFFF;
	ret_vaddr.pt_index = (ptr_64t >> 12) & 0x1FF;
	ret_vaddr.pd_index = (ptr_64t >> 21) & 0x1FF;
	ret_vaddr.pdpt_index = (ptr_64t >> 30) & 0x1FF;
	ret_vaddr.pml4_index = (ptr_64t >> 39) & 0x1FF;

	return ret_vaddr;
}

/*!
 * Get a virtual pointer from a vaddr_t object. Automatically performs sign extension required in
 * x86_64
 * @param virtual_addr the vaddr_t object
 * @return corresponding virtual pointer
 */
inline void* vaddr_t_to_ptr (vaddr_t* virtual_addr) {
	uint64_t ptr_64t = ((uint64_t)virtual_addr->pml4_index << 39) |
					   ((uint64_t)virtual_addr->pdpt_index << 30) |
					   ((uint64_t)virtual_addr->pd_index << 21) |
					   ((uint64_t)virtual_addr->pt_index << 12) | (uint64_t)virtual_addr->offset;

	if (ptr_64t & (1ULL << 47)) ptr_64t |= 0xFFFF000000000000ULL;
	return (void*)ptr_64t;
}

/*!
 * Convert a physical frame to vaddr pointer with HHDM mapping
 * @param phys_frame the physical frame
 * @return pointer to virtual memory using HHDM mapping
 */
inline void* get_vaddr_from_frame (uint64_t phys_frame) {
	return (void*)((phys_frame << 12) + get_hhdm_offset ());
}

/*!
 * Access a physical pointer as vaddr pointer with HHDM mapping
 * @param phys_address the physical pointer
 * @return pointer to virtual memory using HHDM mapping
 */
void* get_vaddr_from_phys_addr (uint64_t phys_address) {
	return (void*)(phys_address + get_hhdm_offset ());
}

/*!
 * Check if vaddr is supposed to be a user page. Practically just a check if pml4 index is at least
 * 256.
 * @param addr vaddr to check
 * @return whether it is a user page
 */
inline bool is_vaddr_t_user (vaddr_t* addr) { return addr->pml4_index < 256; }

/*!
 * Check if vaddr a is strictly less than vaddr b
 * @param a first vaddr
 * @param b second vaddr
 * @return true if a lies before b, false otherwise
 */
bool is_vaddr_t_lt (vaddr_t* a, vaddr_t* b) {
	if (a->pml4_index == b->pml4_index) {
		if (a->pdpt_index == b->pdpt_index) {
			if (a->pd_index == b->pd_index) {
				if (a->pt_index == b->pt_index) return a->offset < b->offset;
				return a->pt_index < b->pt_index;
			}
			return a->pd_index < b->pd_index;
		}
		return a->pdpt_index < b->pdpt_index;
	}
	return a->pml4_index < b->pml4_index;
}

/*!
 * Check if vaddr a is strictly equal to vaddr b
 * @param a first vaddr
 * @param b second vaddr
 * @return true if a == b, false otherwise
 */
bool is_vaddr_t_eq (vaddr_t a, vaddr_t b) {
	return a.pml4_index == b.pml4_index && a.pdpt_index == b.pdpt_index &&
		   a.pd_index == b.pd_index && a.pt_index == b.pt_index && a.offset == b.offset;
}
