// KASAN: stack-out-of-bounds Read in nft_range_dump
// https://syzkaller.appspot.com/bug?id=5af57aca98a625b08499
// status:0
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
  *(uint64_t*)0x20000280 = 0;
  *(uint32_t*)0x20000288 = 0;
  *(uint64_t*)0x20000290 = 0x20000240;
  *(uint64_t*)0x20000240 = 0x20000400;
  memcpy((void*)0x20000400,
         "\xac\x00\x00\x00\x00\x01\x19\x05\x00\x00\x00\x00\x00\x00\x00\xe4\x21"
         "\xd4\x00\x00\x3c\x00\x01\x00\x2c\x00\x01\x00\x14\x00\x03\x00\x16\xf8"
         "\xbc\xd3\x83\x2f\x9b\x15\x77\xde\xf0\x3e\xda\xd6\xb8\xbf\x14\x00\x04"
         "\x00\xfe\x80\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xbb"
         "\x0c\x00\x02\x00\x05\x00\x01\x00\x00\x00\x00\x00\x3c\x00\x02\x00\x0c"
         "\x00\x02\x00\x05\x00\x01\x00\x00\x00\x00\x00\x2c\x00\x01\x00\x14\x00"
         "\x03\x00\xfe\x88\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x01\x14\x00\x04\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x21\x08\x00\x07\x00\x00\x00\x00\x00\x18\x00\x06\x00\x14"
         "\x00\x05\x00\xfe\x88\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x7d\xc0\x46\x34\xd8\x3f\x59\x5f\x16\xb9\x63\x40\xef\x8e\x18\xd7"
         "\x38\x88\x38\x20\x36\x08\xe5\xc7\x82\x1f\x0e\x47\xf0\x7f\x84\x0a\xa4"
         "\x8a\x93\x3e\x75\x11\xdb\x09\x8e\xb6\x98\x9c\x62\x04\x04\xdb\x5f\x00"
         "\x00\x00\x00\x00\x91\x3b\x4e\xec\x66\x32\xd6\x42\xdf\xe1\x3a\xbf\xf0"
         "\x57\x76\x88\x8f\x29\xd5\xe8\x37\x00\x54\xf4\x38\xb2\xa6\x96\xee\x78"
         "\xd6\x20\x2c\xdd\x34\x57\x2b\x17\xdb\x49\x9c\xb8\x87\x4d\xca\xa2\xbe"
         "\x7b\x2f\x28\xa6\x65\x1f\x38\x37\x92\x7b\x27\x73\x22\xed\x77\x9b\x2c"
         "\x37\x67\x11\x2f\xeb\xd5\x16\x0e\xad\xfc\x90\x83\xe2\x5d\xdd\xa2\xee"
         "\x20\x59\xc5\xe5\x75\x06\x83\x1f\xda\x0f\x79\xed\xaa\xd7\x6d\x32\x70"
         "\x03\x29",
         325);
  *(uint64_t*)0x20000248 = 0xac;
  *(uint64_t*)0x20000298 = 1;
  *(uint64_t*)0x200002a0 = 0;
  *(uint64_t*)0x200002a8 = 0;
  *(uint32_t*)0x200002b0 = 0;
  syscall(__NR_sendmsg, -1, 0x20000280ul, 0ul);
  res = syscall(__NR_socket, 0x10ul, 3ul, 0xcul);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000640 = 0;
  *(uint32_t*)0x20000648 = 0;
  *(uint64_t*)0x20000650 = 0x20000200;
  *(uint64_t*)0x20000200 = 0;
  *(uint64_t*)0x20000208 = 0;
  *(uint64_t*)0x20000658 = 8;
  *(uint64_t*)0x20000660 = 0x6800;
  *(uint64_t*)0x20000668 = 0;
  *(uint32_t*)0x20000670 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000640ul, 0ul);
  return 0;
}
