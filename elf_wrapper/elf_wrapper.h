#ifndef _WRAPPER_H_
#define _WRAPPER_H_

#include "elf.h"


#include<sys/mman.h>
#include<sys/stat.h>
#include<sys/types.h>

#include<stdint.h>

// #include "Read_WrapperConfig.h"
#define DEFAULT_CONFIG_FILE "wrapper.config"



/* Errors */
#define FUNC_REF_NOT_FOUND -2
#define FUNC_REF_ALREADY_CHANGED -3

// xmlNodePtr root;
const char *ca_lib = NULL;

struct CA_elf
{
	char *exec_path;

	int file_sz;

	void *vm_start;

	struct load_command **cmd_list;

	struct elf_header *m_header;

	struct dylib **dylib_list;
	uint32_t n_dylib;
	int ca_dylib_no;
	int sym_cmd_no;

    uint64_t VM_SIZE_64;
    uint32_t VM_SIZE;
	uint32_t real_main_addr;

	uint32_t changed_main_index;
	struct dysymtab_command *dysym_cmd;
	struct dyld_info_command *dyld_info_cmd;

} elf_ref;


#endif
