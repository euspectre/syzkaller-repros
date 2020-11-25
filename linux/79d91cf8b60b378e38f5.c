// UBSAN: undefined-behaviour in hash_ipport_create
// https://syzkaller.appspot.com/bug?id=79d91cf8b60b378e38f5
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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  res = syscall(__NR_pipe, 0x20000080ul);
  if (res != -1) {
    r[0] = *(uint32_t*)0x20000080;
    r[1] = *(uint32_t*)0x20000084;
  }
  *(uint64_t*)0x200000c0 = 0x20000180;
  memcpy((void*)0x20000180, "\x77\x69\x0a\xdd\xcf\xbe\x1f\xbb\x66\xec", 10);
  *(uint64_t*)0x200000c8 = 0xff3b;
  syscall(__NR_vmsplice, r[1], 0x200000c0ul, 1ul, 1ul);
  syscall(__NR_close, r[1]);
  syscall(__NR_socket, 0x10ul, 3ul, 0xc);
  *(uint64_t*)0x200000c0 = 0;
  *(uint32_t*)0x200000c8 = 0;
  *(uint64_t*)0x200000d0 = 0x20000080;
  *(uint64_t*)0x20000080 = 0x20000180;
  *(uint32_t*)0x20000180 = 0x58;
  *(uint8_t*)0x20000184 = 2;
  *(uint8_t*)0x20000185 = 6;
  *(uint16_t*)0x20000186 = 0x801;
  *(uint32_t*)0x20000188 = 0;
  *(uint32_t*)0x2000018c = 0;
  *(uint8_t*)0x20000190 = 0;
  *(uint8_t*)0x20000191 = 0;
  *(uint16_t*)0x20000192 = htobe16(0);
  *(uint16_t*)0x20000194 = 0x11;
  *(uint16_t*)0x20000196 = 3;
  memcpy((void*)0x20000198, "hash:ip,port\000", 13);
  *(uint16_t*)0x200001a8 = 5;
  *(uint16_t*)0x200001aa = 4;
  *(uint8_t*)0x200001ac = 0;
  *(uint16_t*)0x200001b0 = 9;
  *(uint16_t*)0x200001b2 = 2;
  memcpy((void*)0x200001b4, "syz1\000", 5);
  *(uint16_t*)0x200001bc = 5;
  *(uint16_t*)0x200001be = 1;
  *(uint8_t*)0x200001c0 = 6;
  *(uint16_t*)0x200001c4 = 5;
  *(uint16_t*)0x200001c6 = 5;
  *(uint8_t*)0x200001c8 = 0xa;
  *(uint16_t*)0x200001cc = 0xc;
  STORE_BY_BITMASK(uint16_t, , 0x200001ce, 7, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x200001cf, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x200001cf, 1, 7, 1);
  *(uint16_t*)0x200001d0 = 8;
  STORE_BY_BITMASK(uint16_t, , 0x200001d2, 0x12, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x200001d3, 1, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x200001d3, 0, 7, 1);
  *(uint32_t*)0x200001d4 = htobe32(-1);
  *(uint64_t*)0x20000088 = 0x58;
  *(uint64_t*)0x200000d8 = 1;
  *(uint64_t*)0x200000e0 = 0;
  *(uint64_t*)0x200000e8 = 0;
  *(uint32_t*)0x200000f0 = 0;
  syscall(__NR_sendmsg, -1, 0x200000c0ul, 0ul);
  syscall(__NR_splice, r[0], 0ul, r[1], 0ul, 0x100000000ul, 0ul);
  return 0;
}