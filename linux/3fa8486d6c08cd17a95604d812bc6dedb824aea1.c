// kernel BUG at mm/vmscan.c:LINE! (2)
// https://syzkaller.appspot.com/bug?id=3fa8486d6c08cd17a95604d812bc6dedb824aea1
// status:open
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);

  memcpy((void*)0x20000080, "/dev/nullb0\000", 12);
  syscall(__NR_openat, 0xffffffffffffff9c, 0x20000080, 0x4000000004002, 0);
  syscall(__NR_madvise, 0x20000000, 0x600003, 0x15);
  return 0;
}