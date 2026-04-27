#include <kclib/string.h>
#include <kernel/elf.h>
#include <kernel/error.h>
#include <kernel/exec.h>
#include <kernel/idt.h>
#include <kernel/memmgt.h>
#include <kernel/process.h>
#include <liballoc/liballoc.h>
#include <stdint.h>

static char** arg_deep_copy (char* const arg_og[]) {
	if (!arg_og) return nullptr;

	int count = 0;
	while (arg_og[count])
		count++;

	char** arg_new = kmalloc ((count + 1) * sizeof (char*));
	if (!arg_new) return nullptr;

	for (int i = 0; i < count; i++)
		arg_new[i] = kstrdup (arg_og[i]);

	arg_new[count] = nullptr;

	return arg_new;
}

static int do_execve (const char* path, char* const argv[], char* const envp[]) {
	int err = 0;

	const char* path_cp = kstrdup (path);
	char**		argv_cp = arg_deep_copy (argv);
	char**		envp_cp = arg_deep_copy (envp);

	if (!path_cp || !argv_cp || !envp_cp) {
		err = -ENOMEM;
		goto cleanup_envc_argc_path;
	}

	uintptr_t entry_point;
	err = load_elf (path_cp, get_current_process (), &entry_point);
	if (err != 0) goto cleanup_envc_argc_path;

	vaddr_t	  us_base_vaddr = {254, 255, 0, 0, 0};
	uintptr_t user_stack_base = (uintptr_t)vaddr_t_to_ptr (&us_base_vaddr);
	size_t	  stack_pages = 4;
	alloc_by_cr3 (get_current_process ()->p_cr3, user_stack_base - (stack_pages * PAGE_SIZE),
				  stack_pages, true);

	int argc = 0, envc = 0;
	while (argv_cp && argv_cp[argc])
		argc++;
	while (envp_cp && envp_cp[envc])
		envc++;

	uintptr_t* argv_ptrs = kmalloc (argc * sizeof (uintptr_t));
	uintptr_t* envp_ptrs = kmalloc (envc * sizeof (uintptr_t));
	uintptr_t  sp = user_stack_base;

	if (!argv_ptrs || !envp_ptrs) {
		err = -ENOMEM;
		goto cleanup_all;
	}

	for (int i = envc - 1; i >= 0; i--) {
		size_t len = kstrlen (envp_cp[i]) + 1;
		sp -= len;
		kmemcpy (((void*)sp), envp_cp[i], len);
		envp_ptrs[i] = sp;
	}

	for (int i = argc - 1; i >= 0; i--) {
		size_t len = kstrlen (argv_cp[i]) + 1;
		sp -= len;
		kmemcpy (((void*)sp), argv_cp[i], len);
		argv_ptrs[i] = sp;
	}

	sp &= ~15;
	int slots_to_push = 1 + argc + 1 + envc + 1;
	if ((slots_to_push * 8) % 16 != 0) sp -= 8;

	sp -= 8;
	*((uintptr_t*)sp) = 0;

	for (int i = envc - 1; i >= 0; i--) {
		sp -= 8;
		*((uintptr_t*)sp) = envp_ptrs[i];
	}

	sp -= 8;
	*((uintptr_t*)sp) = 0;

	for (int i = argc - 1; i >= 0; i--) {
		sp -= 8;
		*((uintptr_t*)sp) = argv_ptrs[i];
	}

	sp -= 8;
	*((uintptr_t*)sp) = argc;
	get_current_process ()->p_registers_ptr->rsp = sp;
	get_current_process ()->p_registers_ptr->rip = entry_point;

cleanup_all:
	if (envp_ptrs) kfree (envp_ptrs);
	if (argv_ptrs) kfree (argv_ptrs);

cleanup_envc_argc_path:
	if (envp_cp) {
		for (int i = 0; i < envc; i++)
			kfree ((void*)envp_cp[i]);
		kfree ((void*)envp_cp);
	}

	if (argv_cp) {
		for (int i = 0; i < argc; i++)
			kfree ((void*)argv_cp[i]);
		kfree ((void*)argv_cp);
	}

	kfree ((void*)path_cp);
	return 0;
}

uint64_t sys_execve (uint64_t path, uint64_t argv, uint64_t envp) {
	return do_execve ((const char*)path, (char* const*)argv, (char* const*)envp);
}

__attribute__ ((noreturn)) static void jump_to_usermode (uintptr_t entry_point,
														 uintptr_t user_stack, bool* user_flag) {
	__asm__ volatile ("cli");
	*user_flag = true;
	__asm__ volatile (
		// Push the structure for iretq
		"pushq $0x3B \n\t"	// Push SS
		"pushq %0 \n\t"		// Push RSP
		"pushq $0x202 \n\t" // Push RFLAGS
		"pushq $0x43 \n\t"	// Push CS
		"pushq %1 \n\t"		// Push RIP

		"iretq \n\t" // Fire iretq to pop registers and drop to Ring 3
		:
		: "r"(user_stack), "r"(entry_point)
		: "memory", "rax");

	for (;;)
		;
}

__attribute__ ((noreturn)) void kernel_execve_as_user (const char* path, char* const argv[],
													   char* const envp[]) {
	__asm__ volatile ("cli");

	registers_t* stable_regs = kmalloc (sizeof (registers_t));
	kmemset (stable_regs, 0, sizeof (registers_t));
	stable_regs->cs = 0x43;
	stable_regs->ss = 0x3B;
	stable_regs->rflags = 0x202;

	process* current = get_current_process ();
	current->p_registers_ptr = stable_regs;

	int err = do_execve (path, argv, envp);
	if (err != 0) goto forever_loop;

	jump_to_usermode (stable_regs->rip, stable_regs->rsp, &current->p_user);

forever_loop:
	for (;;)
		;
}