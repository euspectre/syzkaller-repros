// KASAN: null-ptr-deref Read in PageHuge
// https://syzkaller.appspot.com/bug?id=3a21fd5ecfc677248e77
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

#define BITMASK(bf_off, bf_len) (((1ull << (bf_len)) - 1) << (bf_off))
#define STORE_BY_BITMASK(type, htobe, addr, val, bf_off, bf_len)               \
  *(type*)(addr) =                                                             \
      htobe((htobe(*(type*)(addr)) & ~BITMASK((bf_off), (bf_len))) |           \
            (((type)(val) << (bf_off)) & BITMASK((bf_off), (bf_len))))

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  *(uint32_t*)0x2001d000 = 1;
  *(uint32_t*)0x2001d004 = 0x70;
  *(uint8_t*)0x2001d008 = 0;
  *(uint8_t*)0x2001d009 = 0;
  *(uint8_t*)0x2001d00a = 0;
  *(uint8_t*)0x2001d00b = 0;
  *(uint32_t*)0x2001d00c = 0;
  *(uint64_t*)0x2001d010 = 0;
  *(uint64_t*)0x2001d018 = 0;
  *(uint64_t*)0x2001d020 = 0;
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 0, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 1, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 2, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 3, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 4, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 5, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 6, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 7, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 8, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 9, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 10, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 11, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 12, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 13, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 14, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 15, 2);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 17, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 18, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 19, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 20, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 21, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 22, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 23, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 24, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 25, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 26, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 27, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 28, 1);
  STORE_BY_BITMASK(uint64_t, , 0x2001d028, 0, 29, 35);
  *(uint32_t*)0x2001d030 = 0;
  *(uint32_t*)0x2001d034 = 0;
  *(uint64_t*)0x2001d038 = 0;
  *(uint64_t*)0x2001d040 = 0;
  *(uint64_t*)0x2001d048 = 0;
  *(uint64_t*)0x2001d050 = 0;
  *(uint32_t*)0x2001d058 = 0;
  *(uint32_t*)0x2001d05c = 0;
  *(uint64_t*)0x2001d060 = 0;
  *(uint32_t*)0x2001d068 = 0;
  *(uint16_t*)0x2001d06c = 0;
  *(uint16_t*)0x2001d06e = 0;
  res = syscall(__NR_perf_event_open, 0x2001d000ul, 0, 0ul, -1, 0ul);
  if (res != -1)
    r[0] = res;
  syscall(__NR_mmap, 0x20000000ul, 0x3000ul, 0x10006ul, 0x80011ul, r[0], 0ul);
  syscall(__NR_mincore, 0x20001000ul, 0x4000ul, 0ul);
  return 0;
}