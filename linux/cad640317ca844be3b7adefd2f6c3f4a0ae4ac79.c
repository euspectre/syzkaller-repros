// KASAN: use-after-free Write in selinux_sb_copy_data
// https://syzkaller.appspot.com/bug?id=cad640317ca844be3b7adefd2f6c3f4a0ae4ac79
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

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  syscall(__NR_syslog, 3, 0x200000c0, 0x37a8ec531be3c41f);
  memcpy((void*)0x20000080, "./file0", 8);
  syscall(__NR_mkdir, 0x20000080, 0);
  memcpy((void*)0x20000580, "./file0", 8);
  memcpy((void*)0x200005c0, "./file0", 8);
  memcpy((void*)0x20000600, "btrfs", 6);
  syscall(__NR_mount, 0x20000580, 0x200005c0, 0x20000600, 0x4000, 0x20000640);
  return 0;
}
