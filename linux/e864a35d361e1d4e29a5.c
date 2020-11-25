// kernel BUG at lib/string.c:LINE! (5)
// https://syzkaller.appspot.com/bug?id=e864a35d361e1d4e29a5
// status:6
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include <linux/loop.h>

static unsigned long long procid;

struct fs_image_segment {
  void* data;
  uintptr_t size;
  uintptr_t offset;
};

#define IMAGE_MAX_SEGMENTS 4096
#define IMAGE_MAX_SIZE (129 << 20)

#define sys_memfd_create 319

static unsigned long fs_image_segment_check(unsigned long size,
                                            unsigned long nsegs,
                                            struct fs_image_segment* segs)
{
  if (nsegs > IMAGE_MAX_SEGMENTS)
    nsegs = IMAGE_MAX_SEGMENTS;
  for (size_t i = 0; i < nsegs; i++) {
    if (segs[i].size > IMAGE_MAX_SIZE)
      segs[i].size = IMAGE_MAX_SIZE;
    segs[i].offset %= IMAGE_MAX_SIZE;
    if (segs[i].offset > IMAGE_MAX_SIZE - segs[i].size)
      segs[i].offset = IMAGE_MAX_SIZE - segs[i].size;
    if (size < segs[i].offset + segs[i].offset)
      size = segs[i].offset + segs[i].offset;
  }
  if (size > IMAGE_MAX_SIZE)
    size = IMAGE_MAX_SIZE;
  return size;
}
static int setup_loop_device(long unsigned size, long unsigned nsegs,
                             struct fs_image_segment* segs,
                             const char* loopname, int* memfd_p, int* loopfd_p)
{
  int err = 0, loopfd = -1;
  size = fs_image_segment_check(size, nsegs, segs);
  int memfd = syscall(sys_memfd_create, "syzkaller", 0);
  if (memfd == -1) {
    err = errno;
    goto error;
  }
  if (ftruncate(memfd, size)) {
    err = errno;
    goto error_close_memfd;
  }
  for (size_t i = 0; i < nsegs; i++) {
    if (pwrite(memfd, segs[i].data, segs[i].size, segs[i].offset) < 0) {
    }
  }
  loopfd = open(loopname, O_RDWR);
  if (loopfd == -1) {
    err = errno;
    goto error_close_memfd;
  }
  if (ioctl(loopfd, LOOP_SET_FD, memfd)) {
    if (errno != EBUSY) {
      err = errno;
      goto error_close_loop;
    }
    ioctl(loopfd, LOOP_CLR_FD, 0);
    usleep(1000);
    if (ioctl(loopfd, LOOP_SET_FD, memfd)) {
      err = errno;
      goto error_close_loop;
    }
  }
  *memfd_p = memfd;
  *loopfd_p = loopfd;
  return 0;

error_close_loop:
  close(loopfd);
error_close_memfd:
  close(memfd);
error:
  errno = err;
  return -1;
}

static long syz_mount_image(volatile long fsarg, volatile long dir,
                            volatile unsigned long size,
                            volatile unsigned long nsegs,
                            volatile long segments, volatile long flags,
                            volatile long optsarg)
{
  struct fs_image_segment* segs = (struct fs_image_segment*)segments;
  int res = -1, err = 0, loopfd = -1, memfd = -1, need_loop_device = !!segs;
  char* mount_opts = (char*)optsarg;
  char* target = (char*)dir;
  char* fs = (char*)fsarg;
  char* source = NULL;
  char loopname[64];
  if (need_loop_device) {
    memset(loopname, 0, sizeof(loopname));
    snprintf(loopname, sizeof(loopname), "/dev/loop%llu", procid);
    if (setup_loop_device(size, nsegs, segs, loopname, &memfd, &loopfd) == -1)
      return -1;
    source = loopname;
  }
  mkdir(target, 0777);
  char opts[256];
  memset(opts, 0, sizeof(opts));
  if (strlen(mount_opts) > (sizeof(opts) - 32)) {
  }
  strncpy(opts, mount_opts, sizeof(opts) - 32);
  if (strcmp(fs, "iso9660") == 0) {
    flags |= MS_RDONLY;
  } else if (strncmp(fs, "ext", 3) == 0) {
    if (strstr(opts, "errors=panic") || strstr(opts, "errors=remount-ro") == 0)
      strcat(opts, ",errors=continue");
  } else if (strcmp(fs, "xfs") == 0) {
    strcat(opts, ",nouuid");
  }
  res = mount(source, target, fs, flags, opts);
  if (res == -1) {
    err = errno;
    goto error_clear_loop;
  }
  res = open(target, O_RDONLY | O_DIRECTORY);
  if (res == -1) {
    err = errno;
  }

error_clear_loop:
  if (need_loop_device) {
    ioctl(loopfd, LOOP_CLR_FD, 0);
    close(loopfd);
    close(memfd);
  }
  errno = err;
  return res;
}

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);

  memcpy((void*)0x20000000, "btrfs\000", 6);
  memcpy((void*)0x20000100, "./file0\000", 8);
  *(uint64_t*)0x20000200 = 0x20010000;
  memcpy((void*)0x20010000,
         "\xac\xe8\x32\x62\xc2\x61\xa5\x68\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x81\x5c"
         "\xaf\x9a\xdc\x43\x4d\x2a\xac\x54\x76\x4b\x83\x33\xd7\x65\x00\x00\x01"
         "\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x5f\x42\x48\x52"
         "\x66\x53\x5f\x4d\x05\x00\x00\x00\x00\x00\x00\x00\x00\x10\x50\x00\x00"
         "\x00\x00\x00\x00\x10\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00"
         "\x00\x00\x80\x00\x00\x00\x00\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00"
         "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x10\x00\x00\x00"
         "\x10\x00\x00\x00\x10\x00\x00\x61\x00\x00\x00\x04\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x45\x03\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x01\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x72\x00"
         "\x00\x00\x00\x00\x00\x10\x00\x00\x00\x10\x00\x00\x00\x10\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xa9\xb0\xe7\x44\x0f"
         "\x1a\x44\x9e\xa0\xf7\x25\xfe\x40\x41\x93\x25\x81\x5c\xaf\x9a\xdc\x43"
         "\x4d\x2a\xac\x54\x76\x4b\x83\x33\xd7\x65\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         320);
  *(uint64_t*)0x20000208 = 0x140;
  *(uint64_t*)0x20000210 = 0x10000;
  *(uint64_t*)0x20000218 = 0x20010200;
  memcpy((void*)0x20010200, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff"
                            "\xff\xff\xff\xff\xff\xff\xff\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint64_t*)0x20000220 = 0x20;
  *(uint64_t*)0x20000228 = 0x10220;
  *(uint64_t*)0x20000230 = 0x20010300;
  memcpy((void*)0x20010300,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00"
         "\x00\x00\xe4\x00\x00\x10\x00\x00\x00\x00\x00\x00\x00\x40\x00\x00\x00"
         "\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00"
         "\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x10\x00\x00"
         "\x00\x10\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x10\x00\x00\x00\x00\x00\xa9\xb0\xe7\x44\x0f\x1a\x44\x9e\xa0\xf7"
         "\x25\xfe\x40\x41\x93\x25\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         128);
  *(uint64_t*)0x20000238 = 0x80;
  *(uint64_t*)0x20000240 = 0x10320;
  *(uint64_t*)0x20000248 = 0x20010400;
  memcpy(
      (void*)0x20010400,
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x10\x50\x00\x00\x00\x00"
      "\x00\x05\x00\x00\x00\x00\x00\x00\x00\x00\x10\x10\x00\x00\x00\x00\x00\x04"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x20\x50\x00\x00\x00\x00\x00\x05\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x50\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x70\x50\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x80\x50\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x01\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\x01\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x50\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x80\x10"
      "\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x60\x10\x00\x00"
      "\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x40\x10\x00\x00\x00\x00"
      "\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x70\x10\x00\x00\x00\x00\x00\x02"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x50\x10\x00\x00\x00\x00\x00\x01\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x60\x00\x00\x00"
      "\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x10\x50\x00\x00\x00\x00\x00\x03\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x10\x00\x00\x00\x00\x00\x03\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x30\x50\x00\x00\x00\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x40\x50\x00\x00\x00\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\x00\x20\x50"
      "\x00\x00\x00\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\x00\x50\x10\x00\x00"
      "\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00"
      "\x00\x00\x60\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x50\x50\x00\x00\x00\x00"
      "\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x10\x10\x00\x00\x00\x00\x00\x04"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x60\x50\x00\x00\x00\x00\x00\x04\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x50\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x70\x50\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x80\x50\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x01\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\x01\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
      640);
  *(uint64_t*)0x20000250 = 0x280;
  *(uint64_t*)0x20000258 = 0x10b20;
  *(uint64_t*)0x20000260 = 0;
  *(uint64_t*)0x20000268 = 0;
  *(uint64_t*)0x20000270 = 0x100000;
  *(uint64_t*)0x20000278 = 0x20010800;
  memcpy((void*)0x20010800,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x19"
         "\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00"
         "\x00\x00\x00\x00\x05\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00"
         "\x00\x01\x00\x00\x10\x00\x00\x01\x00\x01\x00\x01\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x69\x00\x00\x00\x00\x00\xa9\xb0\xe7\x44\x0f\x1a\x44"
         "\x9e\xa0\xf7\x25\xfe\x40\x41\x93\x25\x00\x00\x19\x00\x00\x00\x00\x00"
         "\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x05"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x10"
         "\x00\x00\x01\x00\x01\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x50"
         "\x00\x00\x00\x00\x00\xa9\xb0\xe7\x44\x0f\x1a\x44\x9e\xa0\xf7\x25\xfe"
         "\x40\x41\x93\x25\x00\x00\x40\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x10\x00\x00\x00\x10\x00\x00\x00\x10\x00\x00\x01\x00\x00"
         "\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x00\x00"
         "\xa9\xb0\xe7\x44\x0f\x1a\x44\x9e\xa0\xf7\x25\xfe\x40\x41\x93\x25\x01"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00"
         "\x72\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x10\x00\x00\x00\x10\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xa9\xb0\xe7"
         "\x44\x0f\x1a\x44\x9e\xa0\xf7\x25\xfe\x40\x41\x93\x25\x81\x5c\xaf\x9a"
         "\xdc\x43\x4d\x2a\xac\x54\x76\x4b\x83\x33\xd7\x65\xf9\xc0\x35\xfc\x8d"
         "\x23\x9a\x54\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x81\x5c\xaf\x9a\xdc\x43\x4d"
         "\x2a\xac\x54\x76\x4b\x83\x33\xd7\x65\x00\x10\x10\x00\x00\x00\x00\x00"
         "\x01\x00\x00\x00\x00\x00\x00\x01\xd1\x2f\x3c\x01\xe1\x42\x4d\xe7\x82"
         "\x50\x85\xde\xbc\x6a\xf3\xd1\x04\x00\x00\x00\x00\x00\x00\x00\x03\x00"
         "\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00"
         "\x00\x00\xd8\x01\x00\x00\x00\x00\x00\x00\x00\x39\x0f\x00\x00\x62\x00"
         "\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\xe4\x00\x00\x10\x00\x00\x00"
         "\x00\x00\xe9\x0e\x00\x00\x50\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00"
         "\x00\xe4\x00\x00\x50\x00\x00\x00\x00\x00\x99\x0e\x00\x00\x50\x00\x00"
         "\x00\x00\x01\x00\x00\x00\x00\x00\x00\xe4\x00\x00\x69\x00\x00\x00\x00"
         "\x00\x49\x0e\x00\x00\x50\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         576);
  *(uint64_t*)0x20000280 = 0x240;
  *(uint64_t*)0x20000288 = 0x100ea0;
  *(uint64_t*)0x20000290 = 0;
  *(uint64_t*)0x20000298 = 0;
  *(uint64_t*)0x200002a0 = 0x101ea0;
  *(uint64_t*)0x200002a8 = 0;
  *(uint64_t*)0x200002b0 = 0;
  *(uint64_t*)0x200002b8 = 0x102f40;
  *(uint64_t*)0x200002c0 = 0;
  *(uint64_t*)0x200002c8 = 0;
  *(uint64_t*)0x200002d0 = 0x103fc0;
  *(uint64_t*)0x200002d8 = 0;
  *(uint64_t*)0x200002e0 = 0;
  *(uint64_t*)0x200002e8 = 0x105000;
  *(uint64_t*)0x200002f0 = 0;
  *(uint64_t*)0x200002f8 = 0;
  *(uint64_t*)0x20000300 = 0x106000;
  *(uint64_t*)0x20000308 = 0;
  *(uint64_t*)0x20000310 = 0;
  *(uint64_t*)0x20000318 = 0x106f00;
  *(uint64_t*)0x20000320 = 0;
  *(uint64_t*)0x20000328 = 0;
  *(uint64_t*)0x20000330 = 0x107fa0;
  *(uint64_t*)0x20000338 = 0;
  *(uint64_t*)0x20000340 = 0;
  *(uint64_t*)0x20000348 = 0x108f00;
  *(uint64_t*)0x20000350 = 0;
  *(uint64_t*)0x20000358 = 0;
  *(uint64_t*)0x20000360 = 0x500000;
  *(uint64_t*)0x20000368 = 0x20011900;
  memcpy((void*)0x20011900,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x2e\x2e\x03\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xed"
         "\x41\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x64\x31\x5f\x5f\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x64\x31\x5f\x5f\x00\x00\x00\x00\x00\x00\x00\x00\x64\x31"
         "\x5f\x5f\x00\x00\x00\x00\x00\x00\x00\x00\x64\x31\x5f\x5f\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x5d\x69\xa1\x2b\x1d\x85\x97\xa4\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x81\x5c\xaf\x9a\xdc\x43\x4d\x2a\xac\x54\x76\x4b\x83\x33"
         "\xd7\x65\x00\x10\x50\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00"
         "\x01\xd1\x2f\x3c\x01\xe1\x42\x4d\xe7\x82\x50\x85\xde\xbc\x6a\xf3\xd1"
         "\x05\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x0a"
         "\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x84\x00\x00\x00\x00"
         "\x00\x00\x00\x00\xe4\x0d\x00\x00\xb7\x01\x00\x00\x04\x00\x00\x00\x00"
         "\x00\x00\x00\x84\x00\x00\x00\x00\x00\x00\x00\x00\x2d\x0c\x00\x00\xb7"
         "\x01\x00\x00\x05\x00\x00\x00\x00\x00\x00\x00\x0c\x06\x00\x00\x00\x00"
         "\x00\x00\x00\x1c\x0c\x00\x00\x11\x00\x00\x00\x05\x00\x00\x00\x00\x00"
         "\x00\x00\x84\x00\x00\x00\x00\x00\x00\x00\x00\x65\x0a\x00\x00\xb7\x01"
         "\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00"
         "\x00\x00\xc5\x09\x00\x00\xa0\x00\x00\x00\x06\x00\x00\x00\x00\x00\x00"
         "\x00\x0c\x06\x00\x00\x00\x00\x00\x00\x00\xb9\x09\x00\x00\x0c\x00\x00"
         "\x00\x06\x00\x00\x00\x00\x00\x00\x00\x54\xd2\xc2\xbf\x8d\x00\x00\x00"
         "\x00\x94\x09\x00\x00\x25\x00\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00"
         "\x84\x00\x00\x00\x00\x00\x00\x00\x00\xdd\x07\x00\x00\xb7\x01\x00\x00"
         "\x09\x00\x00\x00\x00\x00\x00\x00\x84\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x26\x06\x00\x00\xb7\x01\x00\x00\xf7\xff\xff\xff\xff\xff\xff\xff\x84"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x6f\x04\x00\x00\xb7\x01\x00\x00\x00",
         544);
  *(uint64_t*)0x20000370 = 0x220;
  *(uint64_t*)0x20000378 = 0x500f40;
  *(uint64_t*)0x20000380 = 0;
  *(uint64_t*)0x20000388 = 0;
  *(uint64_t*)0x20000390 = 0x501560;
  *(uint64_t*)0x20000398 = 0;
  *(uint64_t*)0x200003a0 = 0;
  *(uint64_t*)0x200003a8 = 0x501720;
  *(uint64_t*)0x200003b0 = 0;
  *(uint64_t*)0x200003b8 = 0;
  *(uint64_t*)0x200003c0 = 0x501840;
  *(uint64_t*)0x200003c8 = 0;
  *(uint64_t*)0x200003d0 = 0;
  *(uint64_t*)0x200003d8 = 0x5018e0;
  *(uint64_t*)0x200003e0 = 0;
  *(uint64_t*)0x200003e8 = 0;
  *(uint64_t*)0x200003f0 = 0x5019e0;
  *(uint64_t*)0x200003f8 = 0;
  *(uint64_t*)0x20000400 = 0;
  *(uint64_t*)0x20000408 = 0x501b60;
  *(uint64_t*)0x20000410 = 0;
  *(uint64_t*)0x20000418 = 0;
  *(uint64_t*)0x20000420 = 0x501c00;
  *(uint64_t*)0x20000428 = 0;
  *(uint64_t*)0x20000430 = 0;
  *(uint64_t*)0x20000438 = 0x501c80;
  *(uint64_t*)0x20000440 = 0;
  *(uint64_t*)0x20000448 = 0;
  *(uint64_t*)0x20000450 = 0x501d20;
  *(uint64_t*)0x20000458 = 0;
  *(uint64_t*)0x20000460 = 0;
  *(uint64_t*)0x20000468 = 0x501e40;
  *(uint64_t*)0x20000470 = 0;
  *(uint64_t*)0x20000478 = 0;
  *(uint64_t*)0x20000480 = 0x501ee0;
  *(uint64_t*)0x20000488 = 0x20012800;
  memcpy(
      (void*)0x20012800,
      "\x6e\xc8\x9b\x13\xd0\x28\x62\x42\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x81\x5c\xaf\x9a"
      "\xdc\x43\x4d\x2a\xac\x54\x76\x4b\x83\x33\xd7\x65\x00\x20\x50\x00\x00\x00"
      "\x00\x00\x01\x00\x00\x00\x00\x00\x00\x01\xd1\x2f\x3c\x01\xe1\x42\x4d\xe7"
      "\x82\x50\x85\xde\xbc\x6a\xf3\xd1\x05\x00\x00\x00\x00\x00\x00\x00\x02\x00"
      "\x00\x00\x00\x00\x00\x00\x0b\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x00"
      "\x00\xc0\x00\x00\x40\x00\x00\x00\x00\x00\x83\x0f\x00\x00\x18\x00\x00\x00"
      "\x00\x10\x10\x00\x00\x00\x00\x00\xa9\x00\x00\x00\x00\x00\x00\x00\x00\x62"
      "\x0f\x00\x00\x21\x00\x00\x00\x00\x00\x50\x00\x00\x00\x00\x00\xa9\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x41\x0f\x00\x00\x21\x00\x00\x00\x00\x00\x50\x00"
      "\x00\x00\x00\x00\xc0\x00\x00\x19\x00\x00\x00\x00\x00\x29\x0f\x00\x00\x18"
      "\x00\x00\x00\x00\x10\x50\x00\x00\x00\x00\x00\xa9\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x08\x0f\x00\x00\x21\x00\x00\x00\x00\x20\x50\x00\x00\x00\x00\x00"
      "\xa9\x00\x00\x00\x00\x00\x00\x00\x00\xe7\x0e\x00\x00\x21\x00\x00\x00\x00"
      "\x70\x50\x00\x00\x00\x00\x00\xa9\x00\x00\x00\x00\x00\x00\x00\x00\xc6\x0e"
      "\x00\x00\x21\x00\x00\x00\x00\x80\x50\x00\x00\x00\x00\x00\xa9\x00\x00\x00"
      "\x00\x00\x00\x00\x00\xa5\x0e\x00\x00\x21\x00\x00\x00\x00\x90\x50\x00\x00"
      "\x00\x00\x00\xa9\x00\x00\x00\x00\x00\x00\x00\x00\x84\x0e\x00\x00\x21\x00"
      "\x00\x00\x00\xa0\x50\x00\x00\x00\x00\x00\xa9\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x63\x0e\x00\x00\x21\x00\x00\x00\x00\x00\x69\x00\x00\x00\x00\x00\xc0"
      "\x00\x00\x19\x00\x00\x00\x00\x00\x4b\x0e\x00\x00\x18\x00\x00\x00\x00\x00"
      "\x69\x00\x00\x00\x00\x00\xc0\x00\x00\x19\x00\x00\x00\x00\x00\x4b\x0e\x00"
      "\x00\x18\x00\x00\x00\x00\x00\x69\x00\x00\x00\x00\x00\xc0\x00\x00\x19\x00"
      "\x00\x00\x00\x00\x2a\x0e\x00\x00\x18\x00\x00\x00\x00\x00\x50\x00\x00\x00"
      "\x00\x00\xc0\x00\x00\x19\x00\x00\x00\x00\x00\xf3\x0e\x00\x00\x18",
      448);
  *(uint64_t*)0x20000490 = 0x1c0;
  *(uint64_t*)0x20000498 = 0x502000;
  *(uint64_t*)0x200004a0 = 0;
  *(uint64_t*)0x200004a8 = 0;
  *(uint64_t*)0x200004b0 = 0x502e60;
  *(uint64_t*)0x200004b8 = 0;
  *(uint64_t*)0x200004c0 = 0;
  *(uint64_t*)0x200004c8 = 0x503ee0;
  *(uint64_t*)0x200004d0 = 0;
  *(uint64_t*)0x200004d8 = 0;
  *(uint64_t*)0x200004e0 = 0x504f40;
  *(uint64_t*)0x200004e8 = 0;
  *(uint64_t*)0x200004f0 = 0;
  *(uint64_t*)0x200004f8 = 0x505560;
  *(uint64_t*)0x20000500 = 0;
  *(uint64_t*)0x20000508 = 0;
  *(uint64_t*)0x20000510 = 0x505720;
  *(uint64_t*)0x20000518 = 0;
  *(uint64_t*)0x20000520 = 0;
  *(uint64_t*)0x20000528 = 0x505840;
  *(uint64_t*)0x20000530 = 0;
  *(uint64_t*)0x20000538 = 0;
  *(uint64_t*)0x20000540 = 0x5058e0;
  *(uint64_t*)0x20000548 = 0;
  *(uint64_t*)0x20000550 = 0;
  *(uint64_t*)0x20000558 = 0x5059e0;
  *(uint64_t*)0x20000560 = 0;
  *(uint64_t*)0x20000568 = 0;
  *(uint64_t*)0x20000570 = 0x505b60;
  *(uint64_t*)0x20000578 = 0;
  *(uint64_t*)0x20000580 = 0;
  *(uint64_t*)0x20000588 = 0x505c00;
  *(uint64_t*)0x20000590 = 0;
  *(uint64_t*)0x20000598 = 0;
  *(uint64_t*)0x200005a0 = 0x505c80;
  *(uint64_t*)0x200005a8 = 0;
  *(uint64_t*)0x200005b0 = 0;
  *(uint64_t*)0x200005b8 = 0x505d20;
  *(uint64_t*)0x200005c0 = 0;
  *(uint64_t*)0x200005c8 = 0;
  *(uint64_t*)0x200005d0 = 0x505e40;
  *(uint64_t*)0x200005d8 = 0;
  *(uint64_t*)0x200005e0 = 0;
  *(uint64_t*)0x200005e8 = 0x505ee0;
  *(uint64_t*)0x200005f0 = 0;
  *(uint64_t*)0x200005f8 = 0;
  *(uint64_t*)0x20000600 = 0x506000;
  *(uint64_t*)0x20000608 = 0x20014100;
  memcpy((void*)0x20014100,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x05\x00"
         "\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00"
         "\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\xb0\x09\x00\x00"
         "\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00"
         "\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\xb0\xf7\xff\xff\xff"
         "\xff\xff\xff\xff\x01\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00"
         "\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\xb0\x07\x00\x00\x00\x00"
         "\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00"
         "\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\xb0\x04\x00\x00\x00\x00\x00"
         "\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00"
         "\x00\x02\x00\x00\x00\x00\x00\x00\x00\xb0\x02\x00\x00\x00\x00\x00\x00"
         "\x00\x01\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00"
         "\x02\x00\x00\x00\x00\x00\x00\x00\xb0\x01\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x70\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x05"
         "\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x04\x00"
         "\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\xb0\x05\x00"
         "\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00"
         "\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\xb0\x03\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00"
         "\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\xec\xa6\xe5\xff\xab"
         "\x30\xb0\xdd\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x81\x5c\xaf\x9a\xdc\x43\x4d"
         "\x2a\xac\x54\x76\x4b\x83\x33\xd7\x65\x00\x70\x50\x00\x00\x00\x00\x00"
         "\x01\x00\x00\x00\x00\x00\x00\x01\xd1\x2f\x3c\x01\xe1\x42\x4d\xe7\x82"
         "\x50\x85\xde\xbc\x6a\xf3\xd1\x04\x00\x00\x00\x00\x00\x00\x00\x04\x00"
         "\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00"
         "\x00\x00\xcc\x00\x00\x10\x00\x00\x00\x00\x00\x6b\x0f\x00\x00\x30\x00"
         "\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\xcc\x00\x00\x50\x00\x00\x00"
         "\x00\x00\x3b\x0f\x00\x00\x30\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00"
         "\x00\xcc\x00\x00\x69\x00\x00\x00\x00\x00\x0b\x0f\x00\x00\x30\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         544);
  *(uint64_t*)0x20000610 = 0x220;
  *(uint64_t*)0x20000618 = 0x506ea0;
  *(uint64_t*)0x20000620 = 0;
  *(uint64_t*)0x20000628 = 0;
  *(uint64_t*)0x20000630 = 0x507f60;
  *(uint64_t*)0x20000638 = 0x20014500;
  memcpy((void*)0x20014500,
         "\x8e\x3c\xee\xb9\x83\x17\xa9\xc5\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x81\x5c"
         "\xaf\x9a\xdc\x43\x4d\x2a\xac\x54\x76\x4b\x83\x33\xd7\x65\x00\x90\x50"
         "\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x01\xd1\x2f\x3c\x01"
         "\xe1\x42\x4d\xe7\x82\x50\x85\xde\xbc\x6a\xf3\xd1\x04\x00\x00\x00\x00"
         "\x00\x00\x00\xf7\xff\xff\xff\xff\xff\xff\xff\x02\x00\x00\x00\x00\x00"
         "\x01\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\xfb"
         "\x0e\x00\x00\xa0\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x0c\x00"
         "\x01\x00\x00\x00\x00\x00\x00\xef\x0e\x00\x00\x0c\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00",
         160);
  *(uint64_t*)0x20000640 = 0xa0;
  *(uint64_t*)0x20000648 = 0x509000;
  *(uint64_t*)0x20000650 = 0;
  *(uint64_t*)0x20000658 = 0;
  *(uint64_t*)0x20000660 = 0x509f40;
  *(uint8_t*)0x20014900 = 0;
  syz_mount_image(0x20000000, 0x20000100, 0, 0x2f, 0x20000200, 0, 0x20014900);
  return 0;
}