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
	close(fd);
	if (ehdr != NULL)
	{
		free(ehdr);
		ehdr = NULL;
	}
	if (phdr != NULL)
	{
		free(phdr);
		phdr = NULL;
	}
	if (fd != -1)
	{
		close(fd);
		fd = -1;
	}
}

/*
 * Load and run the ELF executable file
 */
void load_and_run_elf(char **argv)
{
	fd = open(argv[1], O_RDONLY); // O_RDONLY :- means Read-ONLY mode.

	// printf("Checking\n\n\n");
	if (fd == -1)
	{ // File opening error Checking
		printf("Error Generated while Opening the File.\n");
		exit(1);
	}

	// ** 1. Load entire binary content into the memory from the ELF file
	ehdr = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));

	read(fd, ehdr, sizeof(Elf32_Ehdr));
	phdr = (Elf32_Phdr *)malloc(ehdr->e_phentsize * ehdr->e_phnum);
	// off_t f_size=lseek(fd,0,SEEK_END);

	lseek(fd, ehdr->e_phoff, SEEK_SET);
	read(fd, phdr, ehdr->e_phentsize * ehdr->e_phnum);
	// fd :- file descriptor.
	// lseek is used to seek the position indication to a specific position in the given file.
	// i have set the offset to '0' in the lseek to avoid changing.
	// Now the location pointer is set now we will read the contents:

	// if(bin_data==MAP_FAILED){
	// 	printf("Error while mapping.\n");
	// 	close(fd);
	// 	exit(1);
	// }

	// 2. Iterate through the PHDR table and find the section of PT_LOAD
	//    type that contains the address of the entrypoint method in fib.c
	// Elf32_Ehdr *elf_header =(Elf32_Ehdr*)bin_data;
	// Elf32_Phdr *program_header=(Elf32_Phdr*)(bin_data+elf_header->e_phoff);
	// void *entry_point=NULL;
	for (int i = 0; i < ehdr->e_phnum; i++)
	{ // Navigating to the entry-point address using the for-loop.
		if (phdr[i].p_type == PT_LOAD)
		{

			if (ehdr->e_entry < (phdr[i].p_vaddr + phdr[i].p_memsz))
			{
				// printf("%d",(phdr->p_vaddr + phdr->p_memsz));
				void *segment_addr = mmap(NULL, phdr[i].p_memsz, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
				lseek(fd, phdr[i].p_offset, SEEK_SET);
				read(fd, segment_addr, phdr[i].p_memsz);
				size_t mypointer = ehdr->e_entry - phdr[i].p_vaddr;
				void *main_pointer = (void *)((uintptr_t)segment_addr + mypointer);
				int (*_start)() = (int (*)())main_pointer;
				int result = _start();
				printf("User _start return value = %d\n", result);
				// entry_point=program_header[i].p_offset;
				break;
			}
			// void *bin_data = mmap(NULL,f_size,PROT_READ,MAP_PRIVATE,fd,0);
			// adding the bin_data address to the entry point address as found the _LOAD.
		}
	}
	// 3. Allocate memory of the size "p_memsz" using mmap function
	// 4. Navigate to the entrypoint address into the segment loaded in the memory in above step
	// 5. Typecast the address to that of function pointer matching "_start" method in fib.c.
	// 6. Call the "_start" method and print the value returned from the "_start"
}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		printf("\n\nUsage: %s <ELF Executable> \n\n", argv[0]);
		exit(1);
	}
	// 1. carry out necessary checks on the input ELF file
	// 2. passing it to the loader for carrying out the loading/execution
	load_and_run_elf(argv);
	// 3. invoke the cleanup routine inside the loader
	loader_cleanup();
	return 0;
}
