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

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0xcul);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20003e00 = 0;
  *(uint32_t*)0x20003e08 = 0;
  *(uint64_t*)0x20003e10 = 0x20003dc0;
  *(uint64_t*)0x20003dc0 = 0x20000340;
  memcpy(
      (void*)0x20000340,
      "\x14\x00\x00\x00\x10\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x0a\x20\x00\x00\x00\x00\x0a\x05\x14\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x01\x00\x00\x00\x09\x00\x01\x00\x73\x79\x7a\x30\x00\x00\x00\x00\x6c\x00"
      "\x00\x00\x16\x0a\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00"
      "\x09\x00\x01\x00\x73\x79\x7a\x30\x00\x00\x00\x00\x09\x00\x02\x00\x73\x79"
      "\x7a\x32\x00\x00\x00\x00\x40\x00\x03\x80\x2c\x00\x03\x80\x14\x00\x01\x00"
      "\x6c\x6f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0c\x14\x00"
      "\x01\x00\x73\x69\x74\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x08\x00\x02\x40\x00\x00\x00\x00\x08\x00\x01\x40\x00\x00\x00\x00\x20\x00"
      "\x00\x00\x12\x0a\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x09"
      "\x00\x02\x00\x73\x79\x7a\x31\x00\x00\x00\x00\x00\x11\x00\x01\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0a\xba\x56\xc0\x87\x03\x24\x64\x34"
      "\x4c\x01\x3c\x00\x00\x00\x00",
      223);
  *(uint64_t*)0x20003dc8 = 0xd4;
  *(uint64_t*)0x20003e18 = 1;
  *(uint64_t*)0x20003e20 = 0;
  *(uint64_t*)0x20003e28 = 0;
  *(uint32_t*)0x20003e30 = 0;
  syscall(__NR_sendmsg, r[0], 0x20003e00ul, 0ul);
  return 0;
}
