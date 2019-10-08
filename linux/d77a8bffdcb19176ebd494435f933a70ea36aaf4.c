// KASAN: stack-out-of-bounds Read in xfrm_state_find (4)
// https://syzkaller.appspot.com/bug?id=d77a8bffdcb19176ebd494435f933a70ea36aaf4
// status:fixed
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

#define BITMASK_LEN(type, bf_len) (type)((1ull << (bf_len)) - 1)

#define BITMASK_LEN_OFF(type, bf_off, bf_len)                                  \
  (type)(BITMASK_LEN(type, (bf_len)) << (bf_off))

#define STORE_BY_BITMASK(type, addr, val, bf_off, bf_len)                      \
  if ((bf_off) == 0 && (bf_len) == 0) {                                        \
    *(type*)(addr) = (type)(val);                                              \
  } else {                                                                     \
    type new_val = *(type*)(addr);                                             \
    new_val &= ~BITMASK_LEN_OFF(type, (bf_off), (bf_len));                     \
    new_val |= ((type)(val)&BITMASK_LEN(type, (bf_len))) << (bf_off);          \
    *(type*)(addr) = new_val;                                                  \
  }

struct csum_inet {
  uint32_t acc;
};

static void csum_inet_init(struct csum_inet* csum)
{
  csum->acc = 0;
}

static void csum_inet_update(struct csum_inet* csum, const uint8_t* data,
                             size_t length)
{
  if (length == 0)
    return;

  size_t i;
  for (i = 0; i < length - 1; i += 2)
    csum->acc += *(uint16_t*)&data[i];

  if (length & 1)
    csum->acc += (uint16_t)data[length - 1];

  while (csum->acc > 0xffff)
    csum->acc = (csum->acc & 0xffff) + (csum->acc >> 16);
}

static uint16_t csum_inet_digest(struct csum_inet* csum)
{
  return ~csum->acc;
}

uint64_t r[1] = {0xffffffffffffffff};
void loop()
{
  long res;
  res = syscall(__NR_socket, 0xa, 0x2100000000000002, 0);
  if (res != -1)
    r[0] = res;
  *(uint16_t*)0x20000000 = 0xa;
  *(uint16_t*)0x20000002 = htobe16(0x4e22);
  *(uint32_t*)0x20000004 = 0;
  *(uint8_t*)0x20000008 = 0;
  *(uint8_t*)0x20000009 = 0;
  *(uint8_t*)0x2000000a = 0;
  *(uint8_t*)0x2000000b = 0;
  *(uint8_t*)0x2000000c = 0;
  *(uint8_t*)0x2000000d = 0;
  *(uint8_t*)0x2000000e = 0;
  *(uint8_t*)0x2000000f = 0;
  *(uint8_t*)0x20000010 = 0;
  *(uint8_t*)0x20000011 = 0;
  *(uint8_t*)0x20000012 = -1;
  *(uint8_t*)0x20000013 = -1;
  *(uint32_t*)0x20000014 = htobe32(0x7f000001);
  *(uint32_t*)0x20000018 = 0;
  syscall(__NR_connect, r[0], 0x20000000, 0x1c);
  *(uint8_t*)0x20000080 = 0xfe;
  *(uint8_t*)0x20000081 = 0x80;
  *(uint8_t*)0x20000082 = 0;
  *(uint8_t*)0x20000083 = 0;
  *(uint8_t*)0x20000084 = 0;
  *(uint8_t*)0x20000085 = 0;
  *(uint8_t*)0x20000086 = 0;
  *(uint8_t*)0x20000087 = 0;
  *(uint8_t*)0x20000088 = 0;
  *(uint8_t*)0x20000089 = 0;
  *(uint8_t*)0x2000008a = 0;
  *(uint8_t*)0x2000008b = 0;
  *(uint8_t*)0x2000008c = 0;
  *(uint8_t*)0x2000008d = 0;
  *(uint8_t*)0x2000008e = 0;
  *(uint8_t*)0x2000008f = 0xbb;
  *(uint64_t*)0x20000090 = htobe64(0);
  *(uint64_t*)0x20000098 = htobe64(1);
  *(uint16_t*)0x200000a0 = htobe16(0x4e20);
  *(uint16_t*)0x200000a2 = htobe16(0);
  *(uint16_t*)0x200000a4 = htobe16(0x4e20);
  *(uint16_t*)0x200000a6 = htobe16(0);
  *(uint16_t*)0x200000a8 = 2;
  *(uint8_t*)0x200000aa = 0;
  *(uint8_t*)0x200000ab = 0;
  *(uint8_t*)0x200000ac = 0;
  *(uint32_t*)0x200000b0 = 0;
  *(uint32_t*)0x200000b4 = 0;
  *(uint64_t*)0x200000b8 = 0;
  *(uint64_t*)0x200000c0 = 0;
  *(uint64_t*)0x200000c8 = 2;
  *(uint64_t*)0x200000d0 = 0;
  *(uint64_t*)0x200000d8 = 0x100000000;
  *(uint64_t*)0x200000e0 = 0;
  *(uint64_t*)0x200000e8 = 0;
  *(uint64_t*)0x200000f0 = 0;
  *(uint64_t*)0x200000f8 = 0;
  *(uint64_t*)0x20000100 = 0;
  *(uint64_t*)0x20000108 = 0;
  *(uint64_t*)0x20000110 = 0;
  *(uint32_t*)0x20000118 = 0;
  *(uint32_t*)0x2000011c = 0;
  *(uint8_t*)0x20000120 = 1;
  *(uint8_t*)0x20000121 = 0;
  *(uint8_t*)0x20000122 = 0;
  *(uint8_t*)0x20000123 = 0;
  *(uint8_t*)0x20000128 = 0xac;
  *(uint8_t*)0x20000129 = 0x14;
  *(uint8_t*)0x2000012a = 0x14;
  *(uint8_t*)0x2000012b = 0;
  *(uint32_t*)0x20000138 = htobe32(0);
  *(uint8_t*)0x2000013c = 0x2b;
  *(uint16_t*)0x20000140 = 0xa;
  *(uint32_t*)0x20000144 = htobe32(0xe0000001);
  *(uint32_t*)0x20000154 = 0;
  *(uint8_t*)0x20000158 = -1;
  *(uint8_t*)0x20000159 = 0;
  *(uint8_t*)0x2000015a = 1;
  *(uint32_t*)0x2000015c = 0;
  *(uint32_t*)0x20000160 = 0;
  *(uint32_t*)0x20000164 = 0;
  syscall(__NR_setsockopt, r[0], 0x29, 0x23, 0x20000080, 0xe8);
  *(uint64_t*)0x20000580 = 0;
  *(uint32_t*)0x20000588 = 0;
  *(uint64_t*)0x20000590 = 0x200026c0;
  *(uint64_t*)0x20000598 = 0;
  *(uint64_t*)0x200005a0 = 0x20002780;
  *(uint64_t*)0x200005a8 = 0;
  *(uint32_t*)0x200005b0 = 0;
  syscall(__NR_sendmsg, r[0], 0x20000580, 0);
  *(uint8_t*)0x20000040 = 0xaa;
  *(uint8_t*)0x20000041 = 0xaa;
  *(uint8_t*)0x20000042 = 0xaa;
  *(uint8_t*)0x20000043 = 0xaa;
  *(uint8_t*)0x20000044 = 0xaa;
  *(uint8_t*)0x20000045 = 0x19;
  *(uint8_t*)0x20000046 = 0;
  *(uint8_t*)0x20000047 = 0;
  *(uint8_t*)0x20000048 = 0;
  *(uint8_t*)0x20000049 = 0;
  *(uint8_t*)0x2000004a = 0;
  *(uint8_t*)0x2000004b = 0;
  *(uint16_t*)0x2000004c = htobe16(0x800);
  STORE_BY_BITMASK(uint8_t, 0x2000004e, 5, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x2000004e, 4, 4, 4);
  STORE_BY_BITMASK(uint8_t, 0x2000004f, 0, 0, 2);
  STORE_BY_BITMASK(uint8_t, 0x2000004f, 0, 2, 6);
  *(uint16_t*)0x20000050 = htobe16(0x1c);
  *(uint16_t*)0x20000052 = htobe16(0);
  *(uint16_t*)0x20000054 = htobe16(-1);
  *(uint8_t*)0x20000056 = 0;
  *(uint8_t*)0x20000057 = 0x89;
  *(uint16_t*)0x20000058 = 0;
  *(uint8_t*)0x2000005a = 0xac;
  *(uint8_t*)0x2000005b = 0x14;
  *(uint8_t*)0x2000005c = 0x14;
  *(uint8_t*)0x2000005d = 0;
  *(uint8_t*)0x2000005e = 0xac;
  *(uint8_t*)0x2000005f = 0x14;
  *(uint8_t*)0x20000060 = 0x14;
  *(uint8_t*)0x20000061 = 0xbb;
  *(uint16_t*)0x20000062 = htobe16(0x4e20);
  *(uint16_t*)0x20000064 = htobe16(0x4e20);
  *(uint16_t*)0x20000066 = htobe16(8);
  *(uint16_t*)0x20000068 = 0;
  *(uint32_t*)0x20000000 = 0;
  *(uint32_t*)0x20000004 = 2;
  *(uint32_t*)0x20000008 = 0;
  *(uint32_t*)0x2000000c = 0;
  *(uint32_t*)0x20000010 = 0x80;
  *(uint32_t*)0x20000014 = 0;
  struct csum_inet csum_1;
  csum_inet_init(&csum_1);
  csum_inet_update(&csum_1, (const uint8_t*)0x2000005a, 4);
  csum_inet_update(&csum_1, (const uint8_t*)0x2000005e, 4);
  uint16_t csum_1_chunk_2 = 0x1100;
  csum_inet_update(&csum_1, (const uint8_t*)&csum_1_chunk_2, 2);
  uint16_t csum_1_chunk_3 = 0x800;
  csum_inet_update(&csum_1, (const uint8_t*)&csum_1_chunk_3, 2);
  csum_inet_update(&csum_1, (const uint8_t*)0x20000062, 8);
  *(uint16_t*)0x20000068 = csum_inet_digest(&csum_1);
  struct csum_inet csum_2;
  csum_inet_init(&csum_2);
  csum_inet_update(&csum_2, (const uint8_t*)0x2000004e, 20);
  *(uint16_t*)0x20000058 = csum_inet_digest(&csum_2);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}