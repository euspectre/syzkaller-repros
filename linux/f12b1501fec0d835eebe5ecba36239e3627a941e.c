// general protection fault in sctp_timeout_obj_to_nlattr
// https://syzkaller.appspot.com/bug?id=f12b1501fec0d835eebe5ecba36239e3627a941e
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

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  res = syscall(__NR_socket, 0x10, 3, 0xc);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20dddfc8 = 0;
  *(uint32_t*)0x20dddfd0 = 0;
  *(uint64_t*)0x20dddfd8 = 0x208a7000;
  *(uint64_t*)0x208a7000 = 0x20000000;
  memcpy((void*)0x20000000, "\x2c\x00\x00\x00\x04\x08\x05\x01\xff\xfc\x7f\xff"
                            "\xfd\xff\xff\x01\x0a\x00\x00\x00\x0c\x00\x03\x00"
                            "\x84\xff\xff\xff\x7d\x0a\x00\xb6\x0c\x00\x02\x00"
                            "\x00\x02\xfa\x17\x71\x89\x10\xa6",
         44);
  *(uint64_t*)0x208a7008 = 0x2c;
  *(uint64_t*)0x20dddfe0 = 1;
  *(uint64_t*)0x20dddfe8 = 0;
  *(uint64_t*)0x20dddff0 = 0;
  *(uint32_t*)0x20dddff8 = 0;
  syscall(__NR_sendmsg, r[0], 0x20dddfc8, 0);
  return 0;
}
