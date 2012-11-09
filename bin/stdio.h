#ifndef STDIO_H
#define STDIO_H

typedef struct file FILE;

struct file;

//@ predicate file(struct file* fp);

FILE* fopen(char* filename, char* mode); // todo: check that mode is a valid mode string
  /*@ requires [?f]chars(filename, ?fcs) &*& [?g]chars(mode, ?mcs) &*& mem('\0', fcs) == true &*& mem('\0', mcs) == true &*&
               (length(mcs) == 2 || length(mcs) == 3) &*&
               (nth(0, mcs) == 'r' || nth(0, mcs) == 'w' || nth(0, mcs) == 'a') &*&
               (length(mcs) == 3 ? nth(1, mcs) == '+' || nth(1, mcs) == 'b' : true);         
  @*/          
  //@ ensures [f]chars(filename, fcs) &*& [g]chars(mode, mcs) &*& result == 0 ? true : file(result);
  
int fread(void* buffer, int size, int n, FILE* fp);
  //@ requires chars(buffer, ?cs) &*& 0<=size &*& 0<=n &*& size * n <= length(cs) &*& file(fp); // TODO!
  //@ ensures chars(buffer, ?cs2) &*& length(cs2) == length(cs) &*& file(fp) &*& result <= n;
  
int fwrite(void* buffer, int size, int n, FILE* fp);
  //@ requires chars(buffer, ?cs) &*& 0<=size &*& 0<=n &*& size * n <= length(cs) &*& file(fp);
  //@ ensures chars(buffer, cs) &*& file(fp);
  
char* fgets(char* buffer, int n, FILE* fp);
  //@ requires chars(buffer, ?cs) &*& length(cs) == n &*& file(fp);
  //@ ensures chars(buffer, ?cs2) &*& length(cs2) == n &*& file(fp) &*& result == 0 ? true : mem('\0', cs2) == true;

int fseek (FILE* fp, /*long*/ int offset, int origin);
  //@ requires file(fp) &*& origin == 0 || origin == 1 || origin == 2;
  //@ ensures file(fp);
  
/* long */ int ftell(FILE* fp);
  //@ requires file(fp);
  //@ ensures file(fp);
  
void rewind(FILE* fp);
  //@ requires file(fp);
  //@ ensures file(fp);

int puts(char* format);
  //@ requires [?f]chars(format, ?cs) &*& mem('\0', cs) == true;
  //@ ensures [f]chars(format, cs);
  
int printf(char* format, int arg);
  //@ requires [?f]chars(format, ?cs) &*& cs == cons('%', cons('i', cons('\0', nil)));
  //@ ensures [f]chars(format, cs);
  
int scanf(char* format, int* arg);
  //@ requires [?f]chars(format, ?cs) &*& cs == cons('%', cons('i', cons('\0', nil))) &*& integer(arg, _);
  //@ ensures [f]chars(format, cs) &*& integer(arg, _);
  
int feof(FILE* fp);
  //@ requires file(fp);
  //@ ensures file(fp);
  
int fclose(FILE* fp); 
  //@ requires file(fp);
  //@ ensures true;

int getchar();
  //@ requires true;
  //@ ensures true;

void putchar(char c);
  //@ requires true;
  //@ ensures true;

#endif