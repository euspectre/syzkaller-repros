// BUG: unable to handle kernel NULL pointer dereference in blkcipher_walk_done
// https://syzkaller.appspot.com/bug?id=8ab4f69bc3a6ab6aa84168ec6825e02808e0569c
// status:fixed
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <sys/syscall.h>
#include <unistd.h>

#include <stdint.h>
#include <string.h>

long r[43];
void loop()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0xf76000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  r[1] = syscall(__NR_socket, 0x26ul, 0x5ul, 0x0ul);
  *(uint16_t*)0x20590fa8 = (uint16_t)0x26;
  memcpy((void*)0x20590faa,
         "\x61\x65\x61\x64\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         14);
  *(uint32_t*)0x20590fb8 = (uint32_t)0x0;
  *(uint32_t*)0x20590fbc = (uint32_t)0x400000010000;
  memcpy((void*)0x20590fc0,
         "\x72\x66\x63\x37\x35\x33\x39\x28\x63\x74\x72\x28\x63\x61\x6d"
         "\x65\x6c\x6c\x69\x61\x29\x2c\x73\x68\x61\x31\x2d\x73\x73\x73"
         "\x65\x33\x29\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00",
         64);
  r[7] = syscall(__NR_bind, r[1], 0x20590fa8ul, 0x58ul);
  memcpy((void*)0x201ec000, "\x0a\x07\x75\xb0\xd5\xe3\x83\xe5\xb3\xb6"
                            "\x0c\xed\x5c\x54\xdb\xb7\x29\x5d\xf0\xdf"
                            "\x82\x17\xad\x40\x00\x00\x00\x00\x00\x00"
                            "\x00\xe6",
         32);
  r[9] = syscall(__NR_setsockopt, r[1], 0x117ul, 0x1ul, 0x201ec000ul,
                 0x20ul);
  r[10] = syscall(__NR_accept, r[1], 0x0ul, 0x0ul);
  *(uint64_t*)0x20f6d000 = (uint64_t)0x0;
  *(uint32_t*)0x20f6d008 = (uint32_t)0x0;
  *(uint64_t*)0x20f6d010 = (uint64_t)0x20f71000;
  *(uint64_t*)0x20f6d018 = (uint64_t)0x1;
  *(uint64_t*)0x20f6d020 = (uint64_t)0x20953000;
  *(uint64_t*)0x20f6d028 = (uint64_t)0x18;
  *(uint32_t*)0x20f6d030 = (uint32_t)0x0;
  *(uint64_t*)0x20f71000 = (uint64_t)0x20ce0000;
  *(uint64_t*)0x20f71008 = (uint64_t)0x0;
  *(uint64_t*)0x20953000 = (uint64_t)0x18;
  *(uint32_t*)0x20953008 = (uint32_t)0x117;
  *(uint32_t*)0x2095300c = (uint32_t)0x3;
  *(uint32_t*)0x20953010 = (uint32_t)0x1;
  r[24] = syscall(__NR_sendmsg, r[10], 0x20f6d000ul, 0x0ul);
  *(uint64_t*)0x2022efc8 = (uint64_t)0x20f6eff0;
  *(uint32_t*)0x2022efd0 = (uint32_t)0x10;
  *(uint64_t*)0x2022efd8 = (uint64_t)0x20892fb0;
  *(uint64_t*)0x2022efe0 = (uint64_t)0x5;
  *(uint64_t*)0x2022efe8 = (uint64_t)0x208adfba;
  *(uint64_t*)0x2022eff0 = (uint64_t)0x0;
  *(uint32_t*)0x2022eff8 = (uint32_t)0x8;
  *(uint64_t*)0x20892fb0 = (uint64_t)0x2000bf7c;
  *(uint64_t*)0x20892fb8 = (uint64_t)0x0;
  *(uint64_t*)0x20892fc0 = (uint64_t)0x20f6ef87;
  *(uint64_t*)0x20892fc8 = (uint64_t)0x0;
  *(uint64_t*)0x20892fd0 = (uint64_t)0x20737000;
  *(uint64_t*)0x20892fd8 = (uint64_t)0x0;
  *(uint64_t*)0x20892fe0 = (uint64_t)0x208a8f50;
  *(uint64_t*)0x20892fe8 = (uint64_t)0xb0;
  *(uint64_t*)0x20892ff0 = (uint64_t)0x20f6e000;
  *(uint64_t*)0x20892ff8 = (uint64_t)0x0;
  r[42] = syscall(__NR_recvmsg, r[10], 0x2022efc8ul, 0x2ul);
}

int main()
{
  loop();
  return 0;
}