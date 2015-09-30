#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

int main(int argc, char *argv[]) {
  // Machine code for:
  // 00000000000108f4 <main>:
  // 00000513                li      a0,0
  // 00008067                ret
  char code[] = {
    0x13, 0x05, 0x10, 0x00, // 0x00000513
    0x67, 0x80, 0x00, 0x00  // 0x00008067
  };

  if (argc < 2) {
    fprintf(stderr, "Usage: jit0-riscv64 <integer>\n");
    return 1;
  }

  // Overwrite immediate value "0" in the instruction
  // with the user's value.  This will make our code:
  //   li a0, <user's value>
  //   ret
  int num = atoi(argv[1]);
  code[2] = (num << 4) & 0xf0;
  code[3] = (num >> 4) & 0x0f;

  // Allocate writable/executable memory.
  // Note: real programs should not map memory both writable
  // and executable because it is a security risk.
  void *mem = mmap(NULL, sizeof(code), PROT_WRITE | PROT_EXEC,
                   MAP_ANON | MAP_PRIVATE, -1, 0);
  memcpy(mem, code, sizeof(code));

  // Clear caches to prevent self-modifying code execute failed due to I-cache
  // and D-cache not coherent.
  //
  // Please see:
  // http://community.arm.com/groups/processors/blog/2010/02/17/caches-and-self-modifying-code
#if defined(__GNUC__)
  __builtin___clear_cache((char*) mem, (char*) (mem + sizeof(code)));
#else
#error "Missing builtin to flush instruction cache."
#endif

  // The function will return the user's value.
  int (*func)() = mem;
  return func();
}
