// suspicious RCU usage at ./include/linux/rcupdate.h:LINE (2)
// https://syzkaller.appspot.com/bug?id=3f7deb7eef7977c9e3b39565cd48fe0a6f316ba0
// status:fixed
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

long r[1];
void loop()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
  r[0] = syscall(__NR_socket, 0x10, 3, 6);
  *(uint64_t*)0x20004000 = 0x20003ff4;
  *(uint32_t*)0x20004008 = 0xc;
  *(uint64_t*)0x20004010 = 0x20004ff0;
  *(uint64_t*)0x20004018 = 1;
  *(uint64_t*)0x20004020 = 0;
  *(uint64_t*)0x20004028 = 0;
  *(uint32_t*)0x20004030 = 0;
  *(uint16_t*)0x20003ff4 = 0x10;
  *(uint16_t*)0x20003ff6 = 0;
  *(uint32_t*)0x20003ff8 = 0;
  *(uint32_t*)0x20003ffc = 0;
  *(uint64_t*)0x20004ff0 = 0x20001dc8;
  *(uint64_t*)0x20004ff8 = 0x144;
  *(uint32_t*)0x20001dc8 = 0x144;
  *(uint16_t*)0x20001dcc = 0x1a;
  *(uint16_t*)0x20001dce = 1;
  *(uint32_t*)0x20001dd0 = 0x70bd25;
  *(uint32_t*)0x20001dd4 = 0x25dfdbfb;
  *(uint8_t*)0x20001dd8 = 0xfe;
  *(uint8_t*)0x20001dd9 = 0x80;
  *(uint8_t*)0x20001dda = 0;
  *(uint8_t*)0x20001ddb = 0;
  *(uint8_t*)0x20001ddc = 0;
  *(uint8_t*)0x20001ddd = 5;
  *(uint8_t*)0x20001dde = 0;
  *(uint8_t*)0x20001ddf = 0;
  *(uint8_t*)0x20001de0 = 0;
  *(uint8_t*)0x20001de1 = 0;
  *(uint8_t*)0x20001de2 = 0;
  *(uint8_t*)0x20001de3 = 0;
  *(uint8_t*)0x20001de4 = 0;
  *(uint8_t*)0x20001de5 = 0;
  *(uint8_t*)0x20001de6 = 0;
  *(uint8_t*)0x20001de7 = 0xaa;
  *(uint64_t*)0x20001de8 = htobe64(0);
  *(uint64_t*)0x20001df0 = htobe64(1);
  *(uint16_t*)0x20001df8 = htobe16(0x4e20);
  *(uint16_t*)0x20001dfa = 0;
  *(uint16_t*)0x20001dfc = htobe16(0x4e20);
  *(uint16_t*)0x20001dfe = 0;
  *(uint16_t*)0x20001e00 = 0;
  *(uint8_t*)0x20001e02 = 0;
  *(uint8_t*)0x20001e03 = 0;
  *(uint8_t*)0x20001e04 = 0;
  *(uint32_t*)0x20001e08 = 0;
  *(uint32_t*)0x20001e0c = 0;
  *(uint8_t*)0x20001e10 = 0;
  *(uint8_t*)0x20001e11 = 0;
  *(uint8_t*)0x20001e12 = 0;
  *(uint8_t*)0x20001e13 = 0;
  *(uint8_t*)0x20001e14 = 0;
  *(uint8_t*)0x20001e15 = 0;
  *(uint8_t*)0x20001e16 = 0;
  *(uint8_t*)0x20001e17 = 0;
  *(uint8_t*)0x20001e18 = 0;
  *(uint8_t*)0x20001e19 = 0;
  *(uint8_t*)0x20001e1a = -1;
  *(uint8_t*)0x20001e1b = -1;
  *(uint32_t*)0x20001e1c = htobe32(0x7f000001);
  *(uint32_t*)0x20001e20 = htobe32(0x4d2);
  *(uint8_t*)0x20001e24 = 0x33;
  *(uint32_t*)0x20001e28 = htobe32(0xe0000002);
  *(uint64_t*)0x20001e38 = 0;
  *(uint64_t*)0x20001e40 = 0;
  *(uint64_t*)0x20001e48 = 0;
  *(uint64_t*)0x20001e50 = 1;
  *(uint64_t*)0x20001e58 = 0;
  *(uint64_t*)0x20001e60 = 0;
  *(uint64_t*)0x20001e68 = 0;
  *(uint64_t*)0x20001e70 = 0;
  *(uint64_t*)0x20001e78 = 0;
  *(uint64_t*)0x20001e80 = 0;
  *(uint64_t*)0x20001e88 = 4;
  *(uint64_t*)0x20001e90 = 0;
  *(uint32_t*)0x20001e98 = 0;
  *(uint32_t*)0x20001e9c = 0;
  *(uint32_t*)0x20001ea0 = 0;
  *(uint32_t*)0x20001ea4 = 0x70bd25;
  *(uint32_t*)0x20001ea8 = 0x34ff;
  *(uint16_t*)0x20001eac = 0xa;
  *(uint8_t*)0x20001eae = 0;
  *(uint8_t*)0x20001eaf = 0;
  *(uint8_t*)0x20001eb0 = 0;
  *(uint16_t*)0x20001eb8 = 0xc;
  *(uint16_t*)0x20001eba = 0x1c;
  *(uint32_t*)0x20001ebc = 0x7f;
  *(uint8_t*)0x20001ec0 = 1;
  *(uint16_t*)0x20001ec4 = 0x48;
  *(uint16_t*)0x20001ec6 = 1;
  memcpy((void*)0x20001ec8,
         "\x64\x69\x67\x65\x73\x74\x5f\x6e\x75\x6c\x6c\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         64);
  *(uint32_t*)0x20001f08 = 0;
  syscall(__NR_sendmsg, r[0], 0x20004000, 0);
}

int main()
{
  loop();
  return 0;
}