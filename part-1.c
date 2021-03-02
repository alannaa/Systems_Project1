/*
 * file:        part-1.c
 * description: Part 1, CS5600 Project 1, Spring 2021
 */

/* THE ONLY INCLUDE FILE */
#include "sysdefs.h"

#define MAXLEN 201

// Attempts to read up to len bytes from file descriptor fd into the buffer starting at ptr
// On success, the number of bytes read is returned
// --> 0 indicates end of file (can't read any more, - bytes read)
// And the file position is advanced by this number (num bytes read)
int read(int fd, void *ptr, int len){
    return syscall(__NR_read, fd, ptr, len);
}

// Writes up to len bytes from the buffer starting at ptr to file at file descriptor fd
// On success, num bytes written returnes
// On error, -1 returned and errno set to indicate cause of error
int write(int fd, void *ptr, int len){
    return syscall(__NR_write, fd, ptr, len);
}

// Terminates the calling process
void exit(int err){
    syscall(__NR_exit, err);
}


/* ---------- */


/* read one line from stdin (file descriptor 0) into a buffer: */
// Returns the number of bytes read in
int readln(char* buf, int maxlen){
    for(int i = 0; i < MAXLEN; i++){
        int len = read(0, (void*) buf, len);

        if (buf[len] == '\n'){
            buf[len] = '\0';
            return len;
        }
    }
    return MAXLEN;
}

// prints a line without \n
void print(char* buf, int len){
    write(1, buf, len);
}

/* print a string to stdout (file descriptor 1) */
void writeln(char* buf, int len){
    write(1, buf, len);
    char nu = '\n';
    write(1, &nu, 1);
}

int strlen(char* str){
    int i = 0;
    while (str[i] != '\0') {
        i++;
    }
    return i;
}


/* ---------- */


void main(void){

    char* hellomsg = "Hello, type lines of input, or 'quit'";
    char* bulletpt = "> ";
    char* youtyped = "you typed: ";

    writeln(hellomsg, strlen(hellomsg));

	 while(1){
      char buf[MAXLEN];
      print(bulletpt, strlen(bulletpt));
      readln(buf, MAXLEN);

      if(buf[0] == 'q' && buf[1] == 'u' && buf[2] == 'i' && buf[3] == 't'){
          exit(0);
      }

      print(youtyped, strlen(youtyped));
      writeln(buf, strlen(buf));
    }
}
