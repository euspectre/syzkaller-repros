// general protection fault in blkcipher_walk_done (2)
// https://syzkaller.appspot.com/bug?id=cee2e7e30c4a5a745a698b9331bb598567262537
// status:dup
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

long r[2];
void loop()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xf92000, 3, 0x32, -1, 0);
  r[0] = syscall(__NR_socket, 0x26, 5, 0);
  *(uint16_t*)0x20f91fa8 = 0x26;
  memcpy((void*)0x20f91faa,
         "\x61\x65\x61\x64\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 14);
  *(uint32_t*)0x20f91fb8 = 0;
  *(uint32_t*)0x20f91fbc = 0;
  memcpy((void*)0x20f91fc0,
         "\x72\x66\x63\x37\x35\x33\x39\x28\x63\x74\x72\x2d\x73\x65\x72\x70\x65"
         "\x6e\x74\x2d\x73\x73\x65\x32\x2c\x72\x6d\x64\x32\x35\x36\x29\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         64);
  syscall(__NR_bind, r[0], 0x20f91fa8, 0x58);
  memcpy((void*)0x203c1000, "\x0a\x07\x75\xb0\xd5\xe3\x83\xe5\xb3\xb6\x0c\xed"
                            "\x5c\x54\xdb\xb7\x29\x5d\xf0\xdf\x82\x17\xad\x40"
                            "\x00\x00\x00\x00\x00\x00\x00\xe6",
         32);
  syscall(__NR_setsockopt, r[0], 0x117, 1, 0x203c1000, 0x20);
  r[1] = syscall(__NR_accept, r[0], 0, 0);
  *(uint64_t*)0x20de1000 = 0;
  *(uint32_t*)0x20de1008 = 0;
  *(uint64_t*)0x20de1010 = 0x20f87ff0;
  *(uint64_t*)0x20de1018 = 1;
  *(uint64_t*)0x20de1020 = 0x20f8e000;
  *(uint64_t*)0x20de1028 = 0;
  *(uint32_t*)0x20de1030 = 0;
  *(uint64_t*)0x20f87ff0 = 0x201c9000;
  *(uint64_t*)0x20f87ff8 = 0x400;
  memcpy(
      (void*)0x201c9000,
      "\x5d\xfb\xc3\x3d\xc1\x9c\xb8\x70\x84\x3d\xf3\x02\x73\xb3\x81\xfa\xa8\xd6"
      "\x2a\x74\xea\xc9\x3d\x92\x5f\x73\x14\x76\x83\xc8\x0e\x60\x33\x71\x91\xa5"
      "\x8d\xf0\xc2\xc6\xd5\xb8\x70\xca\x6e\x04\xca\x02\x1e\xab\x5e\x60\x6a\x54"
      "\x00\xab\x3f\x2b\x56\xcf\xac\x84\x40\x12\x00\x88\x5e\x8e\xd0\x98\xaa\x66"
      "\x15\xba\x7d\x2e\x3a\x54\x5e\x7c\x9e\xbb\x48\x84\x3f\x0a\x66\x11\x47\x64"
      "\x42\x88\xac\x33\xc0\x34\xe2\x1d\x98\x66\xce\xa3\x16\x44\x1b\x3f\xca\x92"
      "\x94\x25\xac\x31\xac\x96\xfa\xe3\x8c\x27\x9f\xaa\x18\xcb\xd3\xfc\x29\x2a"
      "\xab\x32\xde\x88\x5d\x74\x9a\xf3\xdb\x38\x9d\x1e\x12\x8e\x78\x08\x0b\x2c"
      "\x00\x2a\x5f\xa9\xc8\xab\x59\xac\x3a\xaa\x03\x44\x84\x9a\x06\x16\xe8\x11"
      "\xf0\x92\xef\x32\xd1\x56\xb4\xdb\x5b\x36\xb9\x68\x94\x37\xda\x01\xe8\xf9"
      "\x6d\x5a\x7b\x94\x81\x60\x2b\xc3\x59\x98\x91\x12\x9d\x5a\xa3\xb8\x84\xcd"
      "\x91\x5f\x01\x2b\xf4\x05\x46\x3e\xef\x85\x8d\x87\x49\x7d\xf9\x69\x35\xbd"
      "\x9f\x53\xc7\x34\xc1\x8b\x29\x8a\xd7\xa9\xb3\x78\xe8\xc5\x4c\x28\xa8\x9e"
      "\xa5\x7a\x59\x45\xaf\xd1\xb2\x3f\x6a\x45\x99\x37\x26\x9a\x10\x6e\x46\x7a"
      "\x29\x76\x6c\xbe\x0c\xc9\x6f\xd3\x06\xd6\x87\xdb\x2f\x13\x30\x84\x74\x5e"
      "\xa5\x73\xf3\x26\x39\x19\x88\x12\x34\xd9\x3a\xc5\x4d\xe6\x5e\xc1\x91\x13"
      "\x12\xfa\x2b\x2a\xfc\x9b\x16\x5e\x5d\x8a\xb8\x67\x82\xef\xf4\xfc\xfb\x00"
      "\xa1\x23\x63\x86\x28\x3f\x41\xe0\x95\x47\x40\xab\x75\x14\xc8\x50\x12\x6c"
      "\x40\x7f\xe2\x87\xec\x7a\x91\x76\xf3\xb2\xcc\xdd\x5b\x97\x01\x48\x84\x61"
      "\x88\xff\x57\xce\x85\xc4\x51\x63\xaf\x7d\xab\xcf\x89\x7f\x35\x8a\x20\xe7"
      "\x19\xc6\x9c\xd0\xc8\x93\xc3\x61\xdd\x8f\x5f\x5d\x86\x5a\xa8\x32\x4b\xe5"
      "\x76\xac\xb6\x60\x24\xb4\x60\x75\xaf\x67\x42\xd5\xf4\x4b\xf3\x4c\x86\x2f"
      "\x4f\xee\x23\x2c\x5d\x64\xc5\xbe\x17\x6d\x4f\x7d\x42\x2e\x69\xb7\xe5\xca"
      "\x45\xbe\x4c\xca\x3e\x6a\xe0\x4d\x90\x2f\x4d\x4f\xbd\x3c\x61\x37\x8d\x03"
      "\xd7\x84\x8e\x1e\xd6\x65\xf7\xe5\xa7\xcf\xe3\x9c\x7b\xb1\xd8\x50\xcf\x18"
      "\x88\x92\x30\x5f\xdc\x51\xf3\x2b\xc7\x22\xd2\x2b\x01\xf9\x77\x89\xd1\x9f"
      "\xcb\x6d\x7b\xf0\x43\xfb\x79\xfe\xde\x42\x7f\x43\x39\xc5\xe7\x0f\xb5\x3d"
      "\xf8\x15\xed\x0e\xd8\x6c\x29\x5b\xa5\xc8\x12\x7b\xe8\x3c\x02\x5f\xc0\x5e"
      "\x5f\x2e\x78\x21\x91\x60\x6c\x46\x3f\x77\xfc\x5c\xba\x3d\x19\x78\x71\x85"
      "\x58\x23\xc3\xb7\x92\x45\xa2\x93\x62\x18\x14\xc7\xc1\x13\x53\x51\x95\x75"
      "\xa8\xb9\xb6\x5a\x14\x92\x78\xa1\xa4\xa8\xf6\x99\xee\x0f\x7b\x4c\x3a\x29"
      "\x4b\xa6\x20\x2b\xaf\x49\x4b\x8a\x31\x58\x51\x3f\xab\x94\xb9\x82\xe2\x2a"
      "\x13\x85\x44\xc9\xdb\x8a\xfd\xf9\xf7\x1a\xb9\x30\x7f\x23\x71\x06\x34\xca"
      "\x6c\x7f\x7a\x02\x32\x76\xeb\x8f\x95\x30\xa8\x63\x29\xd8\x1e\xd4\xc2\xe4"
      "\x3d\xd1\x0d\x3d\x4b\xd0\xf9\x78\xd2\x5d\x58\x54\xd5\x43\x74\x48\x72\x1d"
      "\x7f\x5a\x2e\x3d\x1f\x99\xd7\xf3\xf6\xd3\x6e\x35\xed\x4d\x49\x21\x7d\xeb"
      "\xbc\xaf\xe5\xb0\x81\x55\x43\x60\xbf\xd5\x19\xbc\x42\x81\x33\x77\x49\x1b"
      "\x33\xb3\x90\x00\x24\xb5\xbb\xf2\x95\x57\xc1\xd3\x65\x38\xbf\x5c\xa3\x0b"
      "\x43\x47\xe3\x79\x94\x4e\x7d\x75\x46\x5f\xd7\x94\x90\xd8\x6a\x70\x22\x6c"
      "\x70\x21\x6f\xeb\x5c\xc2\xed\xaa\x13\x25\x72\x62\x78\xc1\x27\xe2\x84\xe6"
      "\xc9\xfb\x14\xb9\xcf\x09\xf5\x1d\xc6\x27\xf1\x4c\x7d\x5d\xdf\x82\xd4\xfe"
      "\x6c\x92\xa7\xa5\x18\x0e\x56\x89\xf2\x07\xea\x6e\x80\x6d\x99\x6f\xaf\xbb"
      "\x0e\xea\x32\xc0\xf1\x52\x5d\x06\x30\x72\x8e\xd9\x02\x0e\xd7\x6e\xd3\xb5"
      "\x74\x39\xba\x65\x32\x89\x25\x97\x47\x47\x28\x71\x6a\xb6\x36\x4a\x70\x7a"
      "\xa4\x34\xde\xf1\xb5\xd6\x54\xed\xe8\x5a\x17\x30\xa9\x73\xc9\xec\x08\x59"
      "\x98\xd4\x64\x2d\x47\x3d\xa3\x91\x8b\xe2\xd7\x61\x48\x4a\x61\x23\xd1\x07"
      "\x75\xc5\x00\xf5\x5a\x98\x13\x74\x95\x74\x32\xcf\xfe\xcc\xf1\x43\x8a\x34"
      "\x22\x94\x59\x05\x00\x00\x00\x9a\x6c\x3f\x71\x43\x2f\x9d\x3d\x30\x3a\x75"
      "\x07\xd1\x43\x07\x56\x31\x46\xa7\x6f\x03\xf3\xd6\x1b\x56\x50\x11\x38\x5c"
      "\xd4\x18\x16\x0b\xd2\x8b\x31\x2f\xba\x09\xe6\x70\x52\x7f\xff\x40\x0f\x5e"
      "\xd1\x7b\x09\x12\x95\xbf\x0f\x8d\xd3\xef\xf7\x74\x92\x99\x74\x76\x88\x62"
      "\x5d\x80\x03\xef\x91\x26\x96\x1d\x93\x73\x2b\x16\x98\x9e\xca\x6d\xad\x08"
      "\xbf\x0f\xe5\x7f\x6d\x5b\xbb\xb6\x03\x77\xf7\xe0\xc1\x2b\x33\xbd\x84\xae"
      "\x07\xbb\xc1\xf2\xb3\x03\x13\x83\x08\x95\x2e\x78\x9e\x30\x60\xa0\xfd\x33"
      "\x83\x49\xff\x80\xcc\x72\xb7\x6a\x20\xf6\xf8\x9c\x1e\x19\xd9\xe6\xdc\x43"
      "\xf9\x95\x31\x3d\xea\xad\x1b\xfd\x41\xa3\xf5\x77\xdf\x70\xaa\xbb\x43\x47"
      "\x93\x15\xd9\xe2\x24\xe7\x46\x77\x0e\x6f\xbd\x4c\xa9\xbb\x4a\xab",
      1024);
  syscall(__NR_sendmsg, r[1], 0x20de1000, 0);
  *(uint64_t*)0x20318fc8 = 0x20f8affa;
  *(uint32_t*)0x20318fd0 = 6;
  *(uint64_t*)0x20318fd8 = 0x20f87fb0;
  *(uint64_t*)0x20318fe0 = 1;
  *(uint64_t*)0x20318fe8 = 0x20ec7000;
  *(uint64_t*)0x20318ff0 = 0x6b;
  *(uint32_t*)0x20318ff8 = 0;
  *(uint64_t*)0x20f87fb0 = 0x20348000;
  *(uint64_t*)0x20f87fb8 = 0x1000;
  syscall(__NR_recvmsg, r[1], 0x20318fc8, 0);
}

int main()
{
  loop();
  return 0;
}
