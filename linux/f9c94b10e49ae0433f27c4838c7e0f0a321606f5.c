// general protection fault in udpv6_queue_rcv_skb
// https://syzkaller.appspot.com/bug?id=f9c94b10e49ae0433f27c4838c7e0f0a321606f5
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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  res = syscall(__NR_socket, 0x10, 3, 6);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x2014f000 = 0;
  *(uint32_t*)0x2014f008 = 0;
  *(uint64_t*)0x2014f010 = 0x200bfff0;
  *(uint64_t*)0x200bfff0 = 0x20006440;
  *(uint32_t*)0x20006440 = 0xb8;
  *(uint16_t*)0x20006444 = 0x19;
  *(uint16_t*)0x20006446 = 1;
  *(uint32_t*)0x20006448 = 0;
  *(uint32_t*)0x2000644c = 0;
  *(uint8_t*)0x20006450 = -1;
  *(uint8_t*)0x20006451 = 1;
  *(uint8_t*)0x20006452 = 3;
  *(uint8_t*)0x20006453 = 3;
  *(uint8_t*)0x20006454 = 0;
  *(uint8_t*)0x20006455 = 3;
  *(uint8_t*)0x20006456 = 0;
  *(uint8_t*)0x20006457 = 0;
  *(uint8_t*)0x20006458 = 0;
  *(uint8_t*)0x20006459 = 0;
  *(uint8_t*)0x2000645a = 0;
  *(uint8_t*)0x2000645b = 0;
  *(uint8_t*)0x2000645c = 0;
  *(uint8_t*)0x2000645d = 0;
  *(uint8_t*)0x2000645e = 0;
  *(uint8_t*)0x2000645f = 1;
  *(uint32_t*)0x20006460 = htobe32(0xe0000008);
  *(uint16_t*)0x20006470 = htobe16(0);
  *(uint16_t*)0x20006472 = htobe16(0);
  *(uint16_t*)0x20006474 = htobe16(0);
  *(uint16_t*)0x20006476 = htobe16(0);
  *(uint16_t*)0x20006478 = 0xa;
  *(uint8_t*)0x2000647a = 0;
  *(uint8_t*)0x2000647b = 0x60;
  *(uint8_t*)0x2000647c = 0;
  *(uint32_t*)0x20006480 = 0;
  *(uint32_t*)0x20006484 = 0;
  *(uint64_t*)0x20006488 = 0;
  *(uint64_t*)0x20006490 = 0;
  *(uint64_t*)0x20006498 = 0;
  *(uint64_t*)0x200064a0 = 0;
  *(uint64_t*)0x200064a8 = 0;
  *(uint64_t*)0x200064b0 = 0;
  *(uint64_t*)0x200064b8 = 0;
  *(uint64_t*)0x200064c0 = 0;
  *(uint64_t*)0x200064c8 = 0;
  *(uint64_t*)0x200064d0 = 0;
  *(uint64_t*)0x200064d8 = 0;
  *(uint64_t*)0x200064e0 = 0;
  *(uint32_t*)0x200064e8 = 0;
  *(uint32_t*)0x200064ec = 0;
  *(uint8_t*)0x200064f0 = 0;
  *(uint8_t*)0x200064f1 = 0;
  *(uint8_t*)0x200064f2 = 0;
  *(uint8_t*)0x200064f3 = 0;
  *(uint64_t*)0x200bfff8 = 0xb8;
  *(uint64_t*)0x2014f018 = 1;
  *(uint64_t*)0x2014f020 = 0;
  *(uint64_t*)0x2014f028 = 0;
  *(uint32_t*)0x2014f030 = 0;
  syscall(__NR_sendmsg, r[0], 0x2014f000, 0);
  syscall(__NR_ioctl, -1, 0x800454cf, 0);
  res = syscall(__NR_socket, 0xa, 0x80002, 0x88);
  if (res != -1)
    r[1] = res;
  *(uint16_t*)0x20000080 = 0xa;
  *(uint16_t*)0x20000082 = htobe16(0x4e23);
  *(uint32_t*)0x20000084 = htobe32(0);
  memcpy((void*)0x20000088,
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000",
         16);
  *(uint32_t*)0x20000098 = 0;
  syscall(__NR_bind, r[1], 0x20000080, 0x1c);
  *(uint16_t*)0x20000440 = 0xa;
  *(uint16_t*)0x20000442 = htobe16(0x4e23);
  *(uint32_t*)0x20000444 = htobe32(0);
  *(uint8_t*)0x20000448 = -1;
  *(uint8_t*)0x20000449 = 2;
  *(uint8_t*)0x2000044a = 0;
  *(uint8_t*)0x2000044b = 0;
  *(uint8_t*)0x2000044c = 0;
  *(uint8_t*)0x2000044d = 0;
  *(uint8_t*)0x2000044e = 0;
  *(uint8_t*)0x2000044f = 0;
  *(uint8_t*)0x20000450 = 0;
  *(uint8_t*)0x20000451 = 0;
  *(uint8_t*)0x20000452 = 0;
  *(uint8_t*)0x20000453 = 0;
  *(uint8_t*)0x20000454 = 0;
  *(uint8_t*)0x20000455 = 0;
  *(uint8_t*)0x20000456 = 0;
  *(uint8_t*)0x20000457 = 1;
  *(uint32_t*)0x20000458 = 0;
  syscall(__NR_sendto, r[1], 0, 0, 0x4048080, 0x20000440, 0x1c);
  syscall(__NR_sendto, r[1], 0, 0, 0, 0, 0);
  syscall(__NR_close, -1);
  syscall(__NR_ppoll, 0, 0, 0, 0, 0);
  return 0;
}
