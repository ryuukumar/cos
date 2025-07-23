#include <memmgt.h>
#include <stdio.h>
#include <memory.h>


uint64_t pml4_base;

/*!
Function to read the CR3 register. Used to get location of PML4 table.
*/
uint64_t read_cr3() {
	uint64_t cr3;
	__asm__ volatile ("mov %%cr3, %0" : "=r" (cr3));
	return cr3;
}


void init_memmgt() {
	uint64_t cr3 = read_cr3();
	pml4_base = cr3 & 0xFFFFFFFFFF000;

	pml4t_entry_t* pml4t_base = (pml4t_entry_t*)pml4_base;

	uint64_t maxaddr = 0,
			 minaddr = 0xffffffffffffffff;

	printf("Looking through pml4t:\n");

	for (int i = 0; i < 512; ++i) {
		pml4t_entry_t entry = pml4t_base[i];
		if (entry.present) {
			printf("PDPT: 0x%lx | size: %ld\n",
				entry.pdpt_base_address << 12, entry.page_size);
			pdpt_entry_t* pdpt_base = (pdpt_entry_t*)(entry.pdpt_base_address << 12);

			for (int j = 0; j < 512; j++) {
				pdpt_entry_t pdpt_entry = pdpt_base[j];
				if (pdpt_entry.present) {
					printf(" - PD: 0x%lx | size: %ld\n",
						pdpt_entry.pd_base_address << 12, pdpt_entry.page_size);
					pd_entry_t* pd_base = (pd_entry_t*)(pdpt_entry.pd_base_address << 12);

					set_color(0xaaaaaa);
					int cnt = 0, cntt = 0, dirty = 0, accessed = 0;
					for (int k = 0; k < 512; k++) {
						pd_entry_t pd_entry = pd_base[k];
						if (pd_entry.present) {
							cnt++;
							pt_entry_t* pt_base = (pt_entry_t*)(pd_entry.pt_base_address << 12);
							for (int l=0; l < 512; l++) {
								if (pt_base[l].present) {
									cntt++;
									dirty += pt_base[l].dirty > 0;
									accessed += pt_base[l].accessed > 0;
									if (pt_base[l].frame_base_address << 12 < minaddr) minaddr = (pt_base[l].frame_base_address << 12);
									if (pt_base[l].frame_base_address << 12 > maxaddr) maxaddr = (pt_base[l].frame_base_address << 12);
								}
							}
						}
					}
					printf ("    - %d PD entries.\n      - %d PT entries.\n      - %d dirty, %d accessed PT entries.\n",
							cnt, cntt, dirty, accessed);
					set_color(0xffffff);
					printf("\n");
				}
			}
		}
	}

	printf ("Lowest address : 0x%lx\nHighest address: 0x%lx\n", minaddr, maxaddr);
	printf ("init_memmgt address: 0x%lx\npml4_base address  : 0x%lx\n", &init_memmgt, &pml4t_base);
}