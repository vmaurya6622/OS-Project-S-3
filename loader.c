#include "loader.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;			   // fd :- file descriptor.

void loader_cleanup()		// Function to Clean any allocated resources and memory	
{
	if (ehdr != NULL)       // Clearing the space allocated to ELF header
	{
		free(ehdr);
		ehdr = NULL;
	}
	if (phdr != NULL)		// Clearing the space allocated to Program header
	{
		free(phdr);
		phdr = NULL;
	}
	if (fd != -1)			// Closing the file descriptor
	{
		close(fd);
		fd = -1;
	}
}

void load_and_run_elf(char **argv);

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		printf("\n\nUsage: %s <ELF Executable> \n\n", argv[0]);
		exit(1);
	}
	// 1. carry out necessary checks on the input ELF file
	/*       	--> Already done in load_and_run_elf()<--           */

	// 2. passing it to the loader for carrying out the loading/execution
	load_and_run_elf(argv);

	// 3. invoke the cleanup routine inside the loader
	loader_cleanup();
	return 0;
}


void load_and_run_elf(char **argv)       // Function to load and run ELF file
{
	fd = open(argv[1], O_RDONLY); // O_RDONLY :- means Read-ONLY mode.

	if (fd == -1)				// Checking error on file opening
	{ 
		printf("Error Generated while Opening the File.\n");  
		exit(1);
	}

	// ** 1. Load entire binary content into the memory from the ELF file
	ehdr = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));				 // allocating space to ELF header
	read(fd, ehdr, sizeof(Elf32_Ehdr));    							 // reading the ELF file using fd
	phdr = (Elf32_Phdr *)malloc(ehdr->e_phentsize * ehdr->e_phnum);  // allocating space to Program header
	lseek(fd, ehdr->e_phoff, SEEK_SET);								// lseek is used to seek the position indication to a specific position in the given file.
	read(fd, phdr, ehdr->e_phentsize * ehdr->e_phnum);             // storing the binary content to the phdr.

	// ** 2. Iterating through the PHDR table and finding the section of PT_LOAD

	for (int i = 0; i < ehdr->e_phnum; i++)   						// Navigating to the entry-point address using the for-loop.
	{ 
		if (phdr[i].p_type == PT_LOAD)   							// Comparing PT_LOAD with p_type of the program header
		{
			if (ehdr->e_entry < (phdr[i].p_vaddr + phdr[i].p_memsz)) // Finding correct PT_LOAD using the conditon that entry point address must be less than the sum of vaddr and memsz of the program header
			{
				// ** 3. Allocating memory of the size "p_memsz" using mmap function
				void *segment_addr = mmap(NULL, phdr[i].p_memsz, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);

				// ** 4. Navigating to the entrypoint address into the segment loaded in the memory in above step
				lseek(fd, phdr[i].p_offset, SEEK_SET);
				read(fd, segment_addr, phdr[i].p_memsz);

				// ** 5. Typecasting the address to that of function pointer matching "_start" method in fib.c.
				size_t Entry_Addr_Pointer = ehdr->e_entry - phdr[i].p_vaddr;
				void *main_pointer = (void *)((uintptr_t)segment_addr + Entry_Addr_Pointer);
				int (*_start)() = (int (*)())main_pointer;

				// ** 6. Calling the "_start" method and printing the value returned from the "_start"
				int result = _start();
				printf("\nUser _start return value = %d\n", result);
				break;
			}
		}
	}
}

