#include "loader.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;

/*
 * release memory and other cleanups
 */
void loader_cleanup()
{
	// Clean up any allocated resources
    if (ehdr != NULL) {
        free(ehdr);
        ehdr = NULL;
    }
    if (phdr != NULL) {
        free(phdr);
        phdr = NULL;
    }
    if (fd != -1) {
        close(fd);
        fd = -1;
    }
}

/*
 * Load and run the ELF executable file
 */
void load_and_run_elf(char **argv)
{
	fd = open(argv[1],O_RDONLY);// O_RDONLY :- means Read-ONLY mode.
	if(fd==-1){   // File opening error Checking
		printf("Error Generated while Opening the File.\n");
		exit(1);
	}

	// ** 1. Load entire binary content into the memory from the ELF file.
	off_t f_size=lseek(fd,0,SEEK_END);
    lseek(fd,0,SEEK_SET);

	// fd :- file descriptor.
	// lseek is used to seek the position indication to a specific position in the given file.
	// i have set the offset to '0' in the lseek to avoid changing.
	// Now the location pointer is set now we will read the contents:
    void *bin_data = mmap(NULL,f_size,PROT_READ,MAP_PRIVATE,fd,0);
	if(bin_data==MAP_FAILED){
		printf("Error while mapping.\n");
		close(fd);
		exit(1);
	}
	// 2. Iterate through the PHDR table and find the section of PT_LOAD
	//    type that contains the address of the entrypoint method in fib.c
	Elf64_Ehdr *elf_header =(Elf64_Ehdr*)bin_data;
    Elf64_Phdr *program_header=(Elf64_Phdr*)(bin_data+elf_header->e_phoff);
    void *entry_point=NULL;

    for(int i=0;i<elf_header->e_phnum;i++)    {
        if(program_header[i].p_type==PT_LOAD){
            entry_point=bin_data+program_header[i].p_offset;
			// adding the bin_data address to the entry point address.
            break;
        }
    }
	if(entry_point==NULL){
        printf("Error: PT_LOAD segment not found!\n");
        close(fd);
        munmap(bin_data, f_size);
		//unmapping the mapped memory using "munmap" that was allocated using mmap function.
        exit(1);
    }
	// 3. Allocate memory of the size "p_memsz" using mmap function
	//    and then copy the segment content
	void *segment_addr=mmap(NULL, program_header->p_memsz, PROT_READ | PROT_WRITE | PROT_EXEC,MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
	if(segment_addr==MAP_FAILED){
        printf("Error Memory cannot be allocated!\n");
        close(fd);
		munmap(bin_data, f_size);
        exit(1);
	}
    memcpy(segment_addr, entry_point, program_header->p_filesz);

	// 4. Navigate to the entrypoint address into the segment loaded in the memory in above step
	// 5. Typecast the address to that of function pointer matching "_start" method in fib.c.
	// 6. Call the "_start" method and print the value returned from the "_start"
    int (*_start)() = segment_addr;
	int result = _start();
	printf("User _start return value = %d\n", result);
	close(fd);
	// munmap(segment_addr, program_header->p_memsz);
    // munmap(bin_data, f_size);
}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		printf("Usage: %s <ELF Executable> \n", argv[0]);
		exit(1);
	}
	// printf("Checking");
	// 1. carry out necessary checks on the input ELF file
	// 2. passing it to the loader for carrying out the loading/execution
	load_and_run_elf(argv);
	// 3. invoke the cleanup routine inside the loader
	loader_cleanup();
	return 0;
}
