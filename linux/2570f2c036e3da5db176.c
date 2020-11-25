// WARNING in __nf_unregister_net_hook (2)
// https://syzkaller.appspot.com/bug?id=2570f2c036e3da5db176
// status:6
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
  res = syscall(__NR_socket, 0x10ul, 3ul, 0xc);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x2000c2c0 = 0;
  *(uint32_t*)0x2000c2c8 = 0;
  *(uint64_t*)0x2000c2d0 = 0x20000000;
  *(uint64_t*)0x20000000 = 0x20000040;
  *(uint32_t*)0x20000040 = 0x14;
  *(uint16_t*)0x20000044 = 0x10;
  *(uint16_t*)0x20000046 = 1;
  *(uint32_t*)0x20000048 = 0;
  *(uint32_t*)0x2000004c = 0;
  *(uint8_t*)0x20000050 = 0;
  *(uint8_t*)0x20000051 = 0;
  *(uint16_t*)0x20000052 = htobe16(0xa);
  *(uint32_t*)0x20000054 = 0x20;
  *(uint8_t*)0x20000058 = 0;
  *(uint8_t*)0x20000059 = 0xa;
  *(uint16_t*)0x2000005a = 0x101;
  *(uint32_t*)0x2000005c = 0;
  *(uint32_t*)0x20000060 = 0;
  *(uint8_t*)0x20000064 = 2;
  *(uint8_t*)0x20000065 = 0;
  *(uint16_t*)0x20000066 = htobe16(0);
  *(uint16_t*)0x20000068 = 9;
  *(uint16_t*)0x2000006a = 1;
  memcpy((void*)0x2000006c, "syz0\000", 5);
  *(uint32_t*)0x20000074 = 0x50;
  *(uint8_t*)0x20000078 = 3;
  *(uint8_t*)0x20000079 = 0xa;
  *(uint16_t*)0x2000007a = 0x201;
  *(uint32_t*)0x2000007c = 0;
  *(uint32_t*)0x20000080 = 0;
  *(uint8_t*)0x20000084 = 2;
  *(uint8_t*)0x20000085 = 0;
  *(uint16_t*)0x20000086 = htobe16(0xfffe);
  *(uint16_t*)0x20000088 = 9;
  *(uint16_t*)0x2000008a = 1;
  memcpy((void*)0x2000008c, "syz0\000", 5);
  *(uint16_t*)0x20000094 = 0x1c;
  STORE_BY_BITMASK(uint16_t, , 0x20000096, 4, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20000097, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20000097, 1, 7, 1);
  *(uint16_t*)0x20000098 = 8;
  STORE_BY_BITMASK(uint16_t, , 0x2000009a, 2, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x2000009b, 1, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x2000009b, 0, 7, 1);
  *(uint32_t*)0x2000009c = htobe32(0);
  *(uint16_t*)0x200000a0 = 8;
  STORE_BY_BITMASK(uint16_t, , 0x200000a2, 1, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x200000a3, 1, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x200000a3, 0, 7, 1);
  *(uint32_t*)0x200000a4 = htobe32(0);
  *(uint16_t*)0x200000a8 = 8;
  STORE_BY_BITMASK(uint16_t, , 0x200000aa, 2, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x200000ab, 1, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x200000ab, 0, 7, 1);
  *(uint32_t*)0x200000ac = htobe32(0x40ff94a6);
  *(uint16_t*)0x200000b0 = 9;
  *(uint16_t*)0x200000b2 = 3;
  memcpy((void*)0x200000b4, "syz2\000", 5);
  *(uint16_t*)0x200000bc = 8;
  *(uint16_t*)0x200000be = 7;
  memcpy((void*)0x200000c0, "nat\000", 4);
  *(uint32_t*)0x200000c4 = 0x34;
  *(uint8_t*)0x200000c8 = 0;
  *(uint8_t*)0x200000c9 = 0xa;
  *(uint16_t*)0x200000ca = 3;
  *(uint32_t*)0x200000cc = 0;
  *(uint32_t*)0x200000d0 = 0;
  *(uint8_t*)0x200000d4 = 2;
  *(uint8_t*)0x200000d5 = 0;
  *(uint16_t*)0x200000d6 = htobe16(0);
  *(uint16_t*)0x200000d8 = 0xc;
  STORE_BY_BITMASK(uint16_t, , 0x200000da, 4, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x200000db, 1, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x200000db, 0, 7, 1);
  *(uint64_t*)0x200000dc = htobe64(2);
  *(uint16_t*)0x200000e4 = 9;
  *(uint16_t*)0x200000e6 = 1;
  memcpy((void*)0x200000e8, "syz0\000", 5);
  *(uint16_t*)0x200000f0 = 8;
  STORE_BY_BITMASK(uint16_t, , 0x200000f2, 2, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x200000f3, 1, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x200000f3, 0, 7, 1);
  *(uint32_t*)0x200000f4 = htobe32(1);
  *(uint32_t*)0x200000f8 = 0x14;
  *(uint8_t*)0x200000fc = 2;
  *(uint8_t*)0x200000fd = 0xa;
  *(uint16_t*)0x200000fe = 0x101;
  *(uint32_t*)0x20000100 = 0;
  *(uint32_t*)0x20000104 = 0;
  *(uint8_t*)0x20000108 = 0;
  *(uint8_t*)0x20000109 = 0;
  *(uint16_t*)0x2000010a = htobe16(0x200);
  *(uint32_t*)0x2000010c = 0x14;
  *(uint16_t*)0x20000110 = 0x11;
  *(uint16_t*)0x20000112 = 1;
  *(uint32_t*)0x20000114 = 0;
  *(uint32_t*)0x20000118 = 0;
  *(uint8_t*)0x2000011c = 0;
  *(uint8_t*)0x2000011d = 0;
  *(uint16_t*)0x2000011e = htobe16(0xa);
  *(uint64_t*)0x20000008 = 0xe0;
  *(uint64_t*)0x2000c2d8 = 1;
  *(uint64_t*)0x2000c2e0 = 0;
  *(uint64_t*)0x2000c2e8 = 0;
  *(uint32_t*)0x2000c2f0 = 0x4040000;
  syscall(__NR_sendmsg, r[0], 0x2000c2c0ul, 0ul);
  return 0;
}