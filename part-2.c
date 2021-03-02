/*
 * file:        part-2.c
 * description: Part 2, CS5600 Project 1, Spring 2021
 */

/* NO OTHER INCLUDE FILES */
#include "elf64.h"
#include "sysdefs.h"

#define MAXLINE 201         // longest line is 200 chars. 201 to make room for null terminator
#define MAXARGC 10          // max number of program arguments
#define LOADADDR 0x80000000 // where to load program in memory

extern void *vector[];

// global variables to store argc and argv for do_getarg
int glob_argc;
char** glob_argv;

/* ---------- */

/* link these functions from part-1.o */
int read(int fd, void *ptr, int len){
    return syscall(__NR_read, fd, ptr, len);
}

int write(int fd, void *ptr, int len){
    return syscall(__NR_write, fd, ptr, len);
}

void exit(int err){
    syscall(__NR_exit, err);
}

/* write these functions */
int open(char *path, int flags){
		return syscall(__NR_open, path, flags);
}

int close(int fd){
		return syscall(__NR_close, fd);
}

int lseek(int fd, int offset, int flag){
		return syscall(__NR_lseek, fd, offset, flag);
}

void *mmap(void *addr, int len, int prot, int flags, int fd, int offset){
		syscall(__NR_mmap, addr, len, prot, flags, fd, offset);
}

int munmap(void *addr, int len){
		return syscall(__NR_munmap, addr, len);
}

/* ---------- */

/* the three 'system call' functions - readline, print, getarg
 * hints:
 *  - read() or write() one byte at a time. It's OK to be slow.
 *  - stdin is file desc. 0, stdout is file descriptor 1
 *  - use global variables for getarg
 */

void do_readline(char *buf, int len){
		int i = 0;
		for(i=0; i < MAXLINE; i++){
				read(0, (void*) buf+i, 1);

				if (buf[i] == '\n'){
						buf[i] = '\0';
						return i + 1;
					}
				}
	return MAXLINE;
}

/* Prints len characters in buf to stdout */
void print(char *buf, int len) {
    write(1, buf, len);
}

/* Returns the length of the string in buf */
int strlen(const char *buf) {
    int out = 0;

    while (buf[out] != '\0') {
        ++out;
    }

    return out;
}

void do_print(char *buf) {
    int len = strlen(buf);
    print(buf, len);
}

char* do_getarg(int i) {
    if (i >= glob_argc || i < 0) {
        return 0;
    }

    return glob_argv[i];
}


/* ---------- */

/* Returns 1 if str1 equals str2, and returns 0 otherwise */
int streq(char *str1, char *str2) {
    int len1 = strlen(str1);
    int len2 = strlen(str2);

    if (len1 != len2) {
        return 0;
    }
		int i;
    for (i = 0; i < len1; ++i) {
        if (str1[i] != str2[i]) {
            return 0;
        }
    }

    return 1;
}

/* simple function to split a line:
 *   char buffer[200];
 *   <read line into 'buffer'>
 *   char *argv[10];
 *   int argc = split(argv, 10, buffer);
 *   ... pointers to words are in argv[0], ... argv[argc-1]
 */
int split(char **argv, int max_argc, char *line)
{
	int i = 0;
	char *p = line;

	while (i < max_argc) {
		while (*p != 0 && (*p == ' ' || *p == '\t' || *p == '\n'))
			*p++ = 0;
		if (*p == 0)
			return i;
		argv[i++] = p;
		while (*p != 0 && *p != ' ' && *p != '\t' && *p != '\n')
			p++;
	}
	return i;
}

/* ---------- */

/* the guts of part 2
 *   read the ELF header
 *   for each section, if b_type == PT_LOAD:
 *     create mmap region
 *     read from file into region
 *   function call to hdr.e_entry
 *   munmap each mmap'ed region so we don't crash the 2nd time
 */
void load_header(const struct elf_phdr *phdr, int fd, void** unmap_addr, int* unmap_size) {
		if (phdr->p_type == PTLOAD) {
				//The addresses pased to mmap must be a multiple of 4096 for it to work in valgrind
				void* write_addr = phdr->p_vaddr + LOADADDR;
				void* map_addr = (void*) ROUND_DOWN((unsigned long) write_addr, 4096);
				int size = ROUND_UP(phdr->p_memsz, 4096);

				void *buf = mmap(map_addr, size,
                         PROT_READ | PROT_WRITE | PROT_EXEC,
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
				if (buf == MAP_FAILED) {
					do_print("mmap failed\n");
					exit(1);
				}

				// save address and size for unmapping
				*unmap_addr = map_addr;
				*unmap_size = size;

				lseek(fd, (int) phdr->p_offset, SEEK_SET);
				read(fd, write_addr, (int) phdr->p_filesz);
		}
}

// load the file named by filename into memory and run the program there
void runprogram(char* filename) {
	int fd;
	if ((fd = open(filename, O_RDONLY)) < 0) {
		return; //if opening the fd errors
	}

	// read the main header (offset 0)
	struct elf64_ehdr hdr;
	read(fd, &hdr, sizeof(hdr));

	// read program headers (offset hdr.e_phoff)
	int i, n = hdr.e_phnum;
	struct elf64_phdr phdrs[n]; //create an arr of phdrs with the size, n, set to phnum
	lseek(fd, hdr.e_phoff, SEEK_SET);
	read(fd, phdrs, sizeof(phdrs);

	// arrays to keep track of mmapped addresses
	void* unmap_addrs[hdr.e_phnum];
	int unmap_sizes[hdr.e_phnum];

	//look at each section in the program headers
	for (i=0; i<hdr.e_phnum; i++) {
		load_header(&phdrs[i], fd, &unmap_addrs[i], &unmap_sizes[i]);
	}

	//function call to the program entry point
	void (*f)();
	f = hdr.e_entry + LOADADDR;
	f();

	// munmap allocated regions
	for (i = 0; i < hdr.e_phnum; i++) {
		if (phdrs[i].p_type == PT_LOAD) {
			munmap(unmap_addr[i], unmap_sizes[i]);
		}
	}
	close(fd);
}



/* ---------- */

void main(void)
{
	vector[0] = do_readline;
	vector[1] = do_print;
	vector[2] = do_getarg;

	char* bulletpt = "> ";

	while(1) {
		print(bulletpt, strlen(bulletpt));

		//read line and split at whitespace
		char buf[MAXLINE];
		do_readline(buf, MAXLINE);
		char *argv[MAXARGC];
		int argc = split(argv, MAXARGC, buf);

		glob_argc = argc;
		glob_argv = argv;

		//run program
		if (argc > 0) {
				if (streq(argv[0], "quit")) {
					exit(0);
				}
				runprogram(argv[0]);
		}

	}

}
