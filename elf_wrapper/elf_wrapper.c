#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "elf_wrapper.h"

//#define WRAP_MAIN
#define WRAP_FUNC
#define WRAP_OPCODE
#define MAX_OBJC_ARRAY 3


#ifdef DEBUG
#undef DEBUG
#endif

#define DT_NEEDED 1

typedef struct
{
    Elf32_Word  sh_name;
    Elf32_Word  sh_type;
    Elf32_Word  sh_flags;
    Elf32_Word  sh_addr;
    Elf32_Word  sh_offset;
    Elf32_Word  sh_size;
    Elf32_Word  sh_link;
    Elf32_Word  sh_info;
    Elf32_Word  sh_addralign;
    Elf32_Word  sh_entsize;
} Elf32SectHdr;

/* Dynamic section entry.  */
typedef struct
{
    Elf32_Sword d_tag;      /* Dynamic entry type */
    union
    {
        Elf32_Word  d_val;     /* Integer value */
        Elf32_Word  d_ptr;     /* Address value */
    } d_un;
} Elf32_Dyn;

/*
  ---------------------------------------------------------------
  | Function: CCDumpData
  | Purpose:  Debugging aid to show formatted data buffers
  ---------------------------------------------------------------
 */
void CADumpData(int length, void *data)
{
        char *p = (char *)data, hexchars[32], binchars[16];
        int  i, echar, togo, this_len;

        for (togo = length; togo > 0; togo -= this_len)
        {
            this_len = (togo > 16 ? 16 : togo);
            memset(hexchars, 0, sizeof(hexchars));
            memcpy(hexchars, p, this_len);
            memset(binchars, 0, sizeof(binchars));
            memcpy(binchars, p, this_len);
            for (i = 0; i < 16; i++)
            {
                if ( (hexchars[i] >= '0' && hexchars[i] <= '9' )
                    ||
                    (hexchars[i] >= 'a' && hexchars[i] <= 'z' )
                    ||
                    (hexchars[i] >= 'A' && hexchars[i] <= 'Z' )
                    || (hexchars[i] == ' ') )
                    continue;
                else
                {

                    echar = (int)hexchars[i];

                    if ( (echar >= '0' && echar <= '9')
                        ||
                        (echar >= 'a' && echar <= 'z')
                        ||
                        (echar >= 'A' && echar <= 'Z')
                        || (echar == ' ') )
                    {
                        hexchars[i] = echar;
                        continue;
                    }
                    else
                        hexchars[i] = '.';
                }
            }
            printf("%08X %08X %08X %08X <%16.16s>\n",
                   *(int *)(&binchars[0]),
                   *(int *)(&binchars[4]),
                   *(int *)(&binchars[8]),
                   *(int *)(&binchars[12]),
                   hexchars);
            p += this_len;
        }

        return;
}


void ORG_dump_dyn_sh(Elf32_Dyn *sh_dyn, int num_entries){
    int k;
    printf("********* Dump Dynamic Section *********\n\n");
    printf("[Nr]    Tag       Offset\n");
    for (k = 0; k < num_entries; ++k){
        printf(" %02d ", k);
        printf(" 0x%08x ", sh_dyn[k].d_tag);
        printf(" 0x%08x ", sh_dyn[k].d_un.d_val);
        //void *dummy = (void *)sh_dyn[k];
        if(sh_dyn[k].d_tag == DT_NEEDED)
            printf("    <= NEEDED (lib)");
        printf("\n");
    }
}

void* locate_start_of_library(Elf32_Dyn *sh_dyn, int num_entries){
    int k;
    // printf("********* Dump Dynamic Section *********\n\n");
    // printf("[Nr]    Tag       Offset\n");
    for (k = 0; k < num_entries; ++k)
        {
        //printf(" %02d ", k);
        //printf(" 0x%08x ", sh_dyn[k].d_tag);
        //printf(" 0x%08x ", sh_dyn[k].d_un.d_val);
        
        if(sh_dyn[k].d_tag == DT_NEEDED)
            return(&sh_dyn[k].d_tag);
        }
    return NULL;
}

void* locate_end_of_library(Elf32_Dyn *sh_dyn, int num_entries){
    int k;
    // printf("********* Dump Dynamic Section *********\n\n");
    // printf("[Nr]    Tag       Offset\n");
    for (k = 0; k < num_entries; ++k)
    {
        //printf(" %02d ", k);
        //printf(" 0x%08x ", sh_dyn[k].d_tag);
        //printf(" 0x%08x ", sh_dyn[k].d_un.d_val);
        
        if(sh_dyn[k].d_tag == 0x00000000)
            return(&sh_dyn[k].d_tag);
    }
    return NULL;
}



void push_down_stack(void *start_of_stack, void *end_of_stack, Elf32_Word offset)
    {
	Elf32_Word * offsetaddr = NULL;
    do
    {
        void *movefrom = end_of_stack - (2 * sizeof(Elf32_Word) );
        memcpy( end_of_stack, movefrom, ( 2 * sizeof(Elf32_Word)));
        end_of_stack = end_of_stack - ( 2 * sizeof(Elf32_Word) );
    }
        while( end_of_stack > start_of_stack );
        // memset(start_of_stack, 0, ( 2 * sizeof(Elf32_Word) ) );
        offsetaddr = start_of_stack;
        offsetaddr++;
        *offsetaddr = offset+1;
}




void update_library_list(void *start_of_stack, Elf32_Word so_offset)
{
    *(int *)start_of_stack = 0x00000001;
    start_of_stack = start_of_stack + sizeof( Elf32_Word);
    *(Elf32_Word *)start_of_stack = so_offset;
}


void * dump_symtab(void *base, Elf32_Word size, Elf32_Word entsize){
    int i = 0;
    Elf32_Sym * ptr = (Elf32_Sym *)base;
    printf("Dumping SYMBOL TABLE: \n\n");
    for(i=0; i < size/entsize; ++i, ++ptr){
        printf("0x%x\n", ptr->st_name);
    }
    return base;

}
void dump_hash(void *base, Elf32_Word size, Elf32_Word entsize, Elf32_Sym * symbase, char * str_tab){
    int i;
    unsigned int nbuckets, nchains, hn;
    Elf32_Word ndx;
    Elf32_Word *ptr, *hash, *chain;
    ptr =  (Elf32_Word *)base;

    // return;

    nbuckets = *ptr++;
    nchains = *ptr++;

    hash = ptr;
    chain = ptr + nbuckets;

    printf("ELF HASH SECTION: \n\nnbuckets = %u, nchains = %u\n\n", nbuckets, nchains);

#if 0
    for(i=0; i < nchains; ++i){
        printf("0x%x\n",chain[i]);
    }

    hn = elf_hash("ANativeActivity_onCreate") & nbuckets;
    for (ndx = hash[ hn ]; ndx; ndx = chain[ ndx ]) {
	symbol = symbase + ndx;
	if (strcmp("ANativeActivity_onCreate", str_tab + symbol->st_name) == 0)
	    return (load_addr + symbol->st_value);
    }
#endif


}

void dump_dyn_sh(Elf32_Dyn *sh_dyn, int num_entries, char *dt_str_ptr){
    int k;
    printf("********* Dump Dynamic Section *********\n\n");
    printf("[Nr]    Tag       Offset\n");
    for (k = 0; k < num_entries; ++k){
        printf(" %02d ", k);
        printf(" 0x%08x ", sh_dyn[k].d_tag);
        printf(" 0x%08x ", sh_dyn[k].d_un.d_val);
        if(sh_dyn[k].d_tag == DT_NEEDED){
            printf("    %15s    <= NEEDED (lib)",dt_str_ptr+sh_dyn[k].d_un.d_val);
        }
        printf("\n");
    }
}

int main(int argc, char* argv[])
{
    int fd_input                                =   0;
    int file_sz                                 =   0;
    int num_bytes                               =   0;
    int z                                       =   0;
    int sh_dyn_index                            =   0;
    char               *architect_type          =  NULL;
    Elf32_Ehdr         *elf_header              =  NULL;
    Elf32_Phdr         *elf_program_header      =  NULL;
    Elf32_Shdr         *elf_section_header      =  NULL;
    struct elf_arch    *elf_fat_arch          	=  NULL;
    Elf32_Addr          dt_str_adr              =  NULL;
    Elf32_Word          dt_str_size             =   0;
    int index = 0;
    int phindex = 0;
    int scindex = 0;
    Elf32_Sym * symbase = NULL;
    char *dt_str_ptr = NULL;
    


    void *elf_vm_map;
    struct stat filestat;
    for (index = 0; index < argc; index++)
        printf("arg %d = %s\n", index, argv[index]);
    /* Check if file exists */
    
    int rc = stat(argv[1],&filestat);
    printf("stat for %s returned %d\n", argv[1], rc );
    if(stat(argv[1],&filestat) < 0){
        printf("Not able to stat file %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

#ifdef USE_CONFIG_FILE
    char *xml = DEFAULT_CONFIG_FILE;
    /* Check if config file exists */
    if(argc >=4 && strcmp(argv[2], "-c") == 0) {
      printf("Explicit wrapper config file specified\n");
      if(stat(argv[3],&configFilestat) < 0){
        printf("Not able to stat config file\n");
        exit(EXIT_FAILURE);
      }
      xml = argv[3];
    }

    printf("Wrapper config file is %s \n", xml);

    root = parseConfigurationFile(xml);
    ca_lib = get_library_name();
    printf(" ca_lib is %s\n", ca_lib);
#endif


    file_sz = (int)filestat.st_size;
    printf("File %s size:%d\n", argv[1], file_sz);

    elf_ref.file_sz = file_sz;

    /* Map file onto memory */
    num_bytes = file_sz/sizeof(int);
    num_bytes = ( (num_bytes * sizeof(int)) == file_sz ) ? num_bytes: num_bytes + 1;
#ifdef DEBUG
    printf("Num bytes:%d\n",num_bytes);
#endif
    fd_input = open(argv[1], O_RDWR, (mode_t)0600);
    if( fd_input < 0){
        printf("Unable to read file\n");
        exit(EXIT_FAILURE);
    }

    elf_vm_map = mmap(0, (num_bytes * sizeof(int)),
                            PROT_READ | PROT_WRITE,
                            MAP_SHARED, fd_input, 0);
    
    if(elf_vm_map == MAP_FAILED){
        printf("Unable to mmap file\n");
        close(fd_input);
        exit(EXIT_FAILURE);
    }

    char   *p = NULL;
    elf_header = ( Elf32_Ehdr *)elf_vm_map;
    Elf32SectHdr *sh_table;
    Elf32SectHdr *sh_str;
    Elf32_Dyn *sh_dyn = NULL;
    
    
    printf("e_ident         0X", elf_header->e_ident);
    p    = (char *)&(elf_header->e_ident);
    for  ( index = 0; index < EI_NIDENT; index++ )
        printf("%02x", *p++);
    printf("\n");
    printf("e_type          %d\n", elf_header->e_type);
    printf("e_machine       %d\n", elf_header->e_machine);
    printf("e_version       %d\n", elf_header->e_version);
    printf("e_entry         0X");
    p    = (char *)&(elf_header->e_entry);
    for  ( index = 0; index < sizeof(Elf32_Addr); index++ )
        printf("%02x", *p++);
    printf("\n");
    
    printf("e_phoff         %d\n", elf_header->e_phoff);
    printf("e_shoff         %d\n", elf_header->e_shoff);
    printf("e_flags         0X");
    p    = (char *)&(elf_header->e_flags);
    for  ( index = 0; index < sizeof(Elf32_Word); index++ )
        printf("%02x", *p++);
    printf("\n");
    
    printf("e_ehsize        %d\n", elf_header->e_ehsize);
    printf("e_phentsize     %d\n", elf_header->e_phentsize);
    printf("e_phnum         %d\n", elf_header->e_phnum);
    printf("e_shentsize     %d\n", elf_header->e_shentsize);
    printf("e_shnum         %d\n", elf_header->e_shnum);
    printf("e_shstrndx      %d\n", elf_header->e_shstrndx);
    
    
    elf_program_header = ( Elf32_Phdr *)elf_vm_map;
    elf_program_header = (long)elf_program_header + elf_header->e_phoff;
    elf_program_header--;
    for  ( phindex = 0; phindex<elf_header->e_phnum; phindex++)
            {
                elf_program_header++;
                printf("p_type          %d\n", elf_program_header->p_type);
                printf("p_offset        %d\n", elf_program_header->p_offset);
                printf("p_vaddr         %d\n", elf_program_header->p_vaddr);
                printf("p_paddr         %d\n", elf_program_header->p_paddr);
                printf("p_filesz        %d\n", elf_program_header->p_filesz);
                printf("p_memsz         %d\n", elf_program_header->p_memsz);
                printf("p_flags         %d\n", elf_program_header->p_flags);
                printf("p_align         %d\n", elf_program_header->p_align);
            }

    elf_section_header = ( Elf32_Shdr *)elf_vm_map;
    elf_section_header = (long)elf_section_header + elf_header->e_shoff;
    elf_section_header--;
    for  ( scindex = 0; scindex<elf_header->e_shnum; scindex++)
    {
        elf_section_header++;
        printf("sh_name          %d\n", elf_section_header->sh_name);
        printf("sh_type          %d\n", elf_section_header->sh_type);
        printf("sh_flags         %d\n", elf_section_header->sh_flags);
        printf("sh_addr          %d\n", elf_section_header->sh_addr);
        printf("sh_offset        %d\n", elf_section_header->sh_offset);
        printf("sh_size          %d\n", elf_section_header->sh_size);
        printf("sh_link          %d\n", elf_section_header->sh_link);
        printf("sh_info          %d\n", elf_section_header->sh_info);
        printf("sh_addralign     %d\n", elf_section_header->sh_addralign);
        printf("sh_entsize       %d\n", elf_section_header->sh_entsize);
    }

    printf("Number of Sections = %d\n", elf_header->e_shnum);
    sh_table = (Elf32SectHdr *)((uint64_t)elf_vm_map + elf_header->e_shoff);
    
    int  i = 0;
    for( i=0; i<elf_header->e_shnum; i++)
    {
        if(sh_table[i].sh_type == SHT_DYNAMIC)
            {
            sh_dyn          =  (Elf32_Dyn *) ((uint64_t)elf_vm_map + sh_table[i].sh_offset);
            sh_dyn_index    = i;
            printf("DYNAMIC Section found at index = %d   (0x%08x)\n",i, sh_table[i].sh_offset);
            printf("\n[Nr]    Addr       Off        Size        Al   Flg     Type      entry_size\n");
            
            printf(" %02d ", i);
            printf(" 0x%08x ", sh_table[i].sh_addr);
            printf(" 0x%08x ", sh_table[i].sh_offset);
            printf(" 0x%08x ", sh_table[i].sh_size);
            printf("%4d ", sh_table[i].sh_addralign);
            printf(" 0x%04x ", sh_table[i].sh_flags);
            printf(" 0x%08x ", sh_table[i].sh_type);
            printf(" 0x%04x ", sh_table[i].sh_entsize);
            printf("\n\n");
            break;
            }
        else if(sh_table[i].sh_type == SHT_STRTAB && (i!=elf_header->e_shstrndx)){
                    dt_str_adr = sh_table[i].sh_offset;
                    dt_str_size = sh_table[i].sh_size;
		    dt_str_ptr = (char *)((uint64_t)elf_vm_map + dt_str_adr);
        }
	else if(sh_table[i].sh_type == SHT_DYNSYM) {

            printf("SYMBOL TABLE found at index = %d   (0x%08x)\n",i, sh_table[i].sh_offset);
            printf("\n[Nr]    Addr       Off        Size        sh_link     entry_size\n");
            
            printf(" %02d ", i);
            printf(" 0x%08x ", sh_table[i].sh_addr);
            printf(" 0x%08x ", sh_table[i].sh_offset);
            printf(" 0x%08x ", sh_table[i].sh_size);
            printf(" 0x%08x ", sh_table[i].sh_link);
            printf(" 0x%04x ", sh_table[i].sh_entsize);
            printf("\n\n");

	    symbase = dump_symtab((uint64_t)elf_vm_map + sh_table[i].sh_offset, sh_table[i].sh_size, sh_table[i].sh_entsize);
	}
        else if(sh_table[i].sh_type == SHT_HASH){
            printf("HASH Section found at index = %d   (0x%08x)\n",i, sh_table[i].sh_offset);
            printf("\n[Nr]    Addr       Off        Size        sh_link     entry_size\n");
            
            printf(" %02d ", i);
            printf(" 0x%08x ", sh_table[i].sh_addr);
            printf(" 0x%08x ", sh_table[i].sh_offset);
            printf(" 0x%08x ", sh_table[i].sh_size);
            printf(" 0x%08x ", sh_table[i].sh_link);
            printf(" 0x%04x ", sh_table[i].sh_entsize);
            printf("\n\n");

            dump_hash((uint64_t)elf_vm_map + sh_table[i].sh_offset, sh_table[i].sh_size, sh_table[i].sh_entsize,
		      symbase, dt_str_ptr);
        }
    }


    void*  start_library_list  =   NULL;
    void*  end_library_list    =   NULL;
    Elf32_Word libcoffset = 0;

    
    if(sh_dyn)
    {
        int  index = sh_table[i].sh_size/sh_table[i].sh_entsize;
        start_library_list = locate_start_of_library( sh_dyn, index );
        end_library_list = locate_end_of_library( sh_dyn, index );
  
        printf("Start of library list<%#llx>\n", (unsigned long long)start_library_list);
        printf("End of library list<%#llx>\n", (unsigned long long)end_library_list);

    
#if 1
        if(dt_str_adr!=0)
            {
            char *dt_str_start = dt_str_ptr + 1;
            do
                {
                    if  ( strcmp( dt_str_start, "libc.so") == 0 ) {
			libcoffset = dt_str_start - dt_str_ptr;
                        break;
                    }
                    else
                        dt_str_start = dt_str_start + strlen(dt_str_start) + 1;
                } while ( dt_str_start - dt_str_ptr < dt_str_size);
        }
        else
            printf("NO STRING TABLE found\n");
#endif

        push_down_stack( start_library_list, end_library_list, libcoffset);
    }

    
    
    
    if (munmap(elf_vm_map, num_bytes * sizeof(int)) == -1) {
                perror("Error un-mmapping the file");
                close(fd_input);
                exit(EXIT_FAILURE);
        }

        close(fd_input);
        return 0;
}





