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
  *(uint64_t*)0x20003dc0 = 0x20000000;
  memcpy(
      (void*)0x20000000,
      "\x14\x00\x00\x00\x10\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x0a\x20\x00\x00\x00\x00\x0a\x05\x14\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x01\x00\x00\x00\x09\x00\x01\x00\x73\x79\x7a\x30\x00\x00\x00\x00\x6c\x00"
      "\x00\x00\x16\x0a\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x02\x00"
      "\x09\x00\x01\x00\x73\x79\x7a\x30\x00\x00\x00\x00\x09\x00\x02\x00\x73\x79"
      "\x7a\x32\x00\x00\x00\x00\x40\x00\x03\x80\x2c\x00\x03\x80\x14\x00\x01\x00"
      "\x6c\x6f\x00\x00\x00\xfa\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x6d\x8c"
      "\x02\x00\x73\x69\x74\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x08\x00\x02\x40\x00\x00\x00\x00\x08\x00\x01\x40\x00\x00\x00\x00\x7e\xff"
      "\xff\xff\x11\x00\x01\x00\x00\x00\x0a\x02\x74\xc9\x8a\x01\x84\x6b\x90\xfb"
      "\x08\xca\xad\x26\x13\x4e\x71\xfb\xa1\xff\xf7\xc8\x1c\x36\xc9\x74\x8c\x9e"
      "\x51\xed\x67\x84\x09\xb2\x4e\x05\xd3\x12\x01\x96\xd4\x18\xfb\x02\xe7\xde"
      "\x46\xab\x48\x62\x57\x80\xc2\xf3\x8e\x0e\x56\xd1\xc8\x88\xbb\xaa\x8c\xd8"
      "\x1e\x39\xd4\xdf\xb0\x2a\x2a\x9e\x16\x2b\x71\x76\xca\xc4\xff\x45\xe0\x27"
      "\xec\x73\xdd\xb1\x76\xd2\x7c\x93\x63\xaa\xb2\x76\x22\xc7\x82\x58\x70\x2a"
      "\xcf\x7e\x80\x85\x14\x12\x5e\x11\xc1\x08\x5a\xef\xb3\x67\x71\xf2\x2e\x74"
      "\x9d\x73\x3d\x95\x9a\x4b\x2d\xb1\x38\x49\x99\x9e\x24\xa8\xe0\x61\x1c\xd9"
      "\xd6\xdc\xab\x75\x55\x20\xbc\xbc\x66\xbd\x47\x0b\x68\xe7\x97\xe4\x5b\x24"
      "\x17\x30\xdc\x1a\x1c\x56\xbc\x40\xf1\x6a\xce\x8e\x6e\x8d\xa7\xcc\x46\x92"
      "\xa3\xc9\xe2\xfa\x28\xc3\x75\xe9\xde\x3f\xe0\x89\x00\x84\x44\x4c\x61\xb6"
      "\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00",
      370);
  *(uint64_t*)0x20003dc8 = 0xb4;
  *(uint64_t*)0x20003e18 = 1;
  *(uint64_t*)0x20003e20 = 0;
  *(uint64_t*)0x20003e28 = 0;
  *(uint32_t*)0x20003e30 = 0;
  syscall(__NR_sendmsg, r[0], 0x20003e00ul, 0ul);
  return 0;
}
