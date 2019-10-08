// KMSAN: uninit-value in raw6_local_deliver (2)
// https://syzkaller.appspot.com/bug?id=1e0195a94087d49a2c0dcd20b29e94befd3000b7
// status:invalid
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_tun.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <net/if_arp.h>
#include <sched.h>
#include <signal.h>
#include <stdarg.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <unistd.h>

__attribute__((noreturn)) static void doexit(int status)
{
  volatile unsigned i;
  syscall(__NR_exit_group, status);
  for (i = 0;; i++) {
  }
}
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

const int kFailStatus = 67;
const int kRetryStatus = 69;

static void fail(const char* msg, ...)
{
  int e = errno;
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fprintf(stderr, " (errno %d)\n", e);
  doexit((e == ENOMEM || e == EAGAIN) ? kRetryStatus : kFailStatus);
}

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

static void vsnprintf_check(char* str, size_t size, const char* format,
                            va_list args)
{
  int rv;

  rv = vsnprintf(str, size, format, args);
  if (rv < 0)
    fail("tun: snprintf failed");
  if ((size_t)rv >= size)
    fail("tun: string '%s...' doesn't fit into buffer", str);
}

static void snprintf_check(char* str, size_t size, const char* format, ...)
{
  va_list args;

  va_start(args, format);
  vsnprintf_check(str, size, format, args);
  va_end(args);
}

#define COMMAND_MAX_LEN 128
#define PATH_PREFIX                                                            \
  "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin "
#define PATH_PREFIX_LEN (sizeof(PATH_PREFIX) - 1)

static void execute_command(bool panic, const char* format, ...)
{
  va_list args;
  char command[PATH_PREFIX_LEN + COMMAND_MAX_LEN];
  int rv;

  va_start(args, format);
  memcpy(command, PATH_PREFIX, PATH_PREFIX_LEN);
  vsnprintf_check(command + PATH_PREFIX_LEN, COMMAND_MAX_LEN, format, args);
  va_end(args);
  rv = system(command);
  if (rv) {
    if (panic)
      fail("command '%s' failed: %d", &command[0], rv);
  }
}

static int tunfd = -1;
static int tun_frags_enabled;

#define SYZ_TUN_MAX_PACKET_SIZE 1000

#define TUN_IFACE "syz_tun"

#define LOCAL_MAC "aa:aa:aa:aa:aa:aa"
#define REMOTE_MAC "aa:aa:aa:aa:aa:bb"

#define LOCAL_IPV4 "172.20.20.170"
#define REMOTE_IPV4 "172.20.20.187"

#define LOCAL_IPV6 "fe80::aa"
#define REMOTE_IPV6 "fe80::bb"

#define IFF_NAPI 0x0010
#define IFF_NAPI_FRAGS 0x0020

static void initialize_tun(void)
{
  tunfd = open("/dev/net/tun", O_RDWR | O_NONBLOCK);
  if (tunfd == -1) {
    printf("tun: can't open /dev/net/tun: please enable CONFIG_TUN=y\n");
    printf("otherwise fuzzing or reproducing might not work as intended\n");
    return;
  }
  const int kTunFd = 252;
  if (dup2(tunfd, kTunFd) < 0)
    fail("dup2(tunfd, kTunFd) failed");
  close(tunfd);
  tunfd = kTunFd;

  struct ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, TUN_IFACE, IFNAMSIZ);
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI | IFF_NAPI | IFF_NAPI_FRAGS;
  if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0) {
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    if (ioctl(tunfd, TUNSETIFF, (void*)&ifr) < 0)
      fail("tun: ioctl(TUNSETIFF) failed");
  }
  if (ioctl(tunfd, TUNGETIFF, (void*)&ifr) < 0)
    fail("tun: ioctl(TUNGETIFF) failed");
  tun_frags_enabled = (ifr.ifr_flags & IFF_NAPI_FRAGS) != 0;

  execute_command(1, "sysctl -w net.ipv6.conf.%s.accept_dad=0", TUN_IFACE);

  execute_command(1, "sysctl -w net.ipv6.conf.%s.router_solicitations=0",
                  TUN_IFACE);

  execute_command(1, "ip link set dev %s address %s", TUN_IFACE, LOCAL_MAC);
  execute_command(1, "ip addr add %s/24 dev %s", LOCAL_IPV4, TUN_IFACE);
  execute_command(1, "ip -6 addr add %s/120 dev %s", LOCAL_IPV6, TUN_IFACE);
  execute_command(1, "ip neigh add %s lladdr %s dev %s nud permanent",
                  REMOTE_IPV4, REMOTE_MAC, TUN_IFACE);
  execute_command(1, "ip -6 neigh add %s lladdr %s dev %s nud permanent",
                  REMOTE_IPV6, REMOTE_MAC, TUN_IFACE);
  execute_command(1, "ip link set dev %s up", TUN_IFACE);
}

#define DEV_IPV4 "172.20.20.%d"
#define DEV_IPV6 "fe80::%02hx"
#define DEV_MAC "aa:aa:aa:aa:aa:%02hx"

static void initialize_netdevices(void)
{
  unsigned i;
  const char* devtypes[] = {"ip6gretap", "bridge", "vcan", "bond", "team"};
  const char* devnames[] = {"lo",
                            "sit0",
                            "bridge0",
                            "vcan0",
                            "tunl0",
                            "gre0",
                            "gretap0",
                            "ip_vti0",
                            "ip6_vti0",
                            "ip6tnl0",
                            "ip6gre0",
                            "ip6gretap0",
                            "erspan0",
                            "bond0",
                            "veth0",
                            "veth1",
                            "team0",
                            "veth0_to_bridge",
                            "veth1_to_bridge",
                            "veth0_to_bond",
                            "veth1_to_bond",
                            "veth0_to_team",
                            "veth1_to_team"};
  const char* devmasters[] = {"bridge", "bond", "team"};

  for (i = 0; i < sizeof(devtypes) / (sizeof(devtypes[0])); i++)
    execute_command(0, "ip link add dev %s0 type %s", devtypes[i], devtypes[i]);
  execute_command(0, "ip link add type veth");

  for (i = 0; i < sizeof(devmasters) / (sizeof(devmasters[0])); i++) {
    execute_command(
        0, "ip link add name %s_slave_0 type veth peer name veth0_to_%s",
        devmasters[i], devmasters[i]);
    execute_command(
        0, "ip link add name %s_slave_1 type veth peer name veth1_to_%s",
        devmasters[i], devmasters[i]);
    execute_command(0, "ip link set %s_slave_0 master %s0", devmasters[i],
                    devmasters[i]);
    execute_command(0, "ip link set %s_slave_1 master %s0", devmasters[i],
                    devmasters[i]);
    execute_command(0, "ip link set veth0_to_%s up", devmasters[i]);
    execute_command(0, "ip link set veth1_to_%s up", devmasters[i]);
  }
  execute_command(0, "ip link set bridge_slave_0 up");
  execute_command(0, "ip link set bridge_slave_1 up");

  for (i = 0; i < sizeof(devnames) / (sizeof(devnames[0])); i++) {
    char addr[32];
    snprintf_check(addr, sizeof(addr), DEV_IPV4, i + 10);
    execute_command(0, "ip -4 addr add %s/24 dev %s", addr, devnames[i]);
    snprintf_check(addr, sizeof(addr), DEV_IPV6, i + 10);
    execute_command(0, "ip -6 addr add %s/120 dev %s", addr, devnames[i]);
    snprintf_check(addr, sizeof(addr), DEV_MAC, i + 10);
    execute_command(0, "ip link set dev %s address %s", devnames[i], addr);
    execute_command(0, "ip link set dev %s up", devnames[i]);
  }
}

#define MAX_FRAGS 4
struct vnet_fragmentation {
  uint32_t full;
  uint32_t count;
  uint32_t frags[MAX_FRAGS];
};

static uintptr_t syz_emit_ethernet(uintptr_t a0, uintptr_t a1, uintptr_t a2)
{
  if (tunfd < 0)
    return (uintptr_t)-1;

  uint32_t length = a0;
  char* data = (char*)a1;

  struct vnet_fragmentation* frags = (struct vnet_fragmentation*)a2;
  struct iovec vecs[MAX_FRAGS + 1];
  uint32_t nfrags = 0;
  if (!tun_frags_enabled || frags == NULL) {
    vecs[nfrags].iov_base = data;
    vecs[nfrags].iov_len = length;
    nfrags++;
  } else {
    bool full = true;
    uint32_t i, count = 0;
    full = frags->full;
    count = frags->count;
    if (count > MAX_FRAGS)
      count = MAX_FRAGS;
    for (i = 0; i < count && length != 0; i++) {
      uint32_t size = 0;
      size = frags->frags[i];
      if (size > length)
        size = length;
      vecs[nfrags].iov_base = data;
      vecs[nfrags].iov_len = size;
      nfrags++;
      data += size;
      length -= size;
    }
    if (length != 0 && (full || nfrags == 0)) {
      vecs[nfrags].iov_base = data;
      vecs[nfrags].iov_len = length;
      nfrags++;
    }
  }
  return writev(tunfd, vecs, nfrags);
}

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  setsid();

  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = 160 << 20;
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 8 << 20;
  setrlimit(RLIMIT_MEMLOCK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 136 << 20;
  setrlimit(RLIMIT_FSIZE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_STACK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &rlim);

  if (unshare(CLONE_NEWNS)) {
  }
  if (unshare(CLONE_NEWIPC)) {
  }
  if (unshare(0x02000000)) {
  }
  if (unshare(CLONE_NEWUTS)) {
  }
  if (unshare(CLONE_SYSVSEM)) {
  }
}

static int do_sandbox_none(void)
{
  if (unshare(CLONE_NEWPID)) {
  }
  int pid = fork();
  if (pid < 0)
    fail("sandbox fork failed");
  if (pid)
    return pid;

  sandbox_common();
  if (unshare(CLONE_NEWNET)) {
  }
  initialize_tun();
  initialize_netdevices();

  loop();
  doexit(1);
}

uint64_t r[1] = {0xffffffffffffffff};
void loop()
{
  long res = 0;
  res = syscall(__NR_socket, 0xa, 2, 0);
  if (res != -1)
    r[0] = res;
  memcpy(
      (void*)0x20000100,
      "\xae\x13\xa5\x90\x16\x39\x4b\xe4\x1f\x68\xc0\xad\x75\x88\x45\xc0\x39\x35"
      "\x0a\x5c\xe1\x5d\x73\xd0\x24\x62\x0a\x43\x32\xdc\x9f\x33\x55\x64\x9a\x75"
      "\x54\x9d\x6f\x17\x62\x8f\xe1\x7b\xdf\x84\xba\xed\x97\x89\xb2\xdb\xe3\x6b"
      "\xd2\x58\xc6\x62\x62\x92\x04\xa9\xe4\x99\x24\xc8\x5e\xc2\x63\x9f\x08\xbb"
      "\x76\xe2\xf6\x2f\x2d\xf7\xd8\xf5\x28\xe2\xc5\xbb\x5d\xe6\x5a\x40\x17\x68"
      "\x38\xc4\xf2\x67\xf0\xa3\x34\xe7\x66\xc3\x77\x97\xe2\xc2\xeb\xf1\x98\x65"
      "\x06\xd6\x79\x8a\x71\x87\xca\x56\xe3\x43\x16\xfc\x10\xc3\xce\x37\x54\xf2"
      "\x7a\x26\xa4\x30\xd1\x03\x8e\x10\xf4\x53\x07\xe0\x6f\xa0\x09\x1a\x54\xbf"
      "\x5a\x08\x2f\xb8\x8b\x2d\xaa\xf2\x59\x34\x1f\x31\xb3\x65\xae\x4f\xdd\xef"
      "\xae\x55\x77\x93\x6a\xdd\xc7\xcd\x18\xab\xd2\x5b\x36\x3b\x51\xf3\x59\x71"
      "\x62\x5c\x23\xb6\x3b\x8b\xe1\x0b\x87\xe3\x44\x51\xe2\x1a\xd6\x83\xcc\xa5"
      "\x95\xd4\x06\x1f\x9f\x16\xe1\xcd\xc6\x53\x30\x8f\x58\x8d\xf1\x2e\x63\xb8"
      "\x68\x47\x03\x81\xb1\x9f\xd4\xdd\x48\x1a\x4c\x62\x8a\x20\xe5\x6a\x9f\x73"
      "\x1b\x85\xd6\xb9\xe6\xa5\x80\xfa\xb8\x5e\xbc\x05\x7b\x42\xdf\x4b\x5c\x8a"
      "\xab\x4d\x4f\xfe\x99\x5e\x10\x60\x7a\x8c\x4e\x18\x91\x44\x4a\x02\x00\x27"
      "\x7f\xa4\x18\x75\xd8\xa1\x33\x5e\x41\x37\x50\x9c\xda\x3e\xcf\x11\x6a\x1c"
      "\x8f\xe4\x7f\x02\x13\xcd\x3e\x0f\xc1\x91\x88\x9d\x43\xf3\x5d\x07\xe9\xac"
      "\x67\xed\x26\xfd\xa8\x65\x66\xc5\xa9\x44\xee\xe1\x08\xa4\xa4\x40\xa2\xc7"
      "\xd7\x71\x76\x0d\x2d\xa9\xd6\x6d\xc9\x67\x18\xdc\xb0\xd4\xec\x15\x23\xb2"
      "\x38\x54\xe6\x87\x7a\x1d\x5f\x3a\x23\x5e\x2c\x8b\x35\xfc\x38\x00\x85\x34"
      "\x0a\x6a\x07\x09\x53\xc2\xf2\xfb\x09\xfc\x5d\xf0\x5b\xdf\xcb\x19\x3b\xca"
      "\x62\x6e\x36\xfb\x56\xb9\x60\x76\x5f\x86\xba\x75\xfe\x34\x88\x6d\x60\xba"
      "\x2c\x1d\xca\xb6\x91\xa1\x59\x2d\xac\xd1\xef\xec\x69\x2a\x71\xb5\x7d\x1d"
      "\x9a\xda\x23\x26\x73\x59\x5e\xa8\x10\xa4\x5e\x9e\xf4\x6f\xdc\x23\x0a\xb0"
      "\x91\xd1\x9f\x42\x59\xb7\xc5\x80\x61\xc1\x3e\x4d\x79\x1c\xe7\x11\x65\xd2"
      "\x11\xd2\x8b\x43\x6c\x56\xc5\xb3\x70\x36\x3d\xc1\x96\xa8\x42\xa1\x1f\x10"
      "\x5e\x43\x74\xf1\x82\x71\xcb\x70\xe6\xdc\xf8\xf4\x1d\x6d\xe7\x82\xf0\xe7"
      "\xd7\xa0\x96\x46\x68\xbd\xc9\x73\x39\x55\xa0\xde\x88\xe7\x59\xa3\xfd\x9e"
      "\x58\xfb\x66\xfe\x86\xf8\xd3\xa0\x1f\xc4\x58\xc4\xe4\x17\x39\x96\x68\x08"
      "\xca\x4d\x0e\x9d\xac\x50\xda\x01\xfe\x6a\x7a\x04\xef\xfc\x99\xf6\x51\xa9"
      "\x79\x90\xbe\xe5\x09\xf5\x3b\x09\xae\x4e\x56\x9f\xa3\xbe\x12\xc1\x55\xa2"
      "\x91\xd9\xc7\x89\x76\xef\x48\x40\x2b\x6b\xca\x76\xc8\xf4\xae\x3a\xd7\x4c"
      "\xe3\x7f\xb9\x1c\x4e\xe4\x62\x6f\x26\xc4\x62\x0a\x0f\xaa\x04\x7f\xe2\x35"
      "\xf9\x36\x5b\x12\xb7\xaf\x90\x4b\xea\xd7\x90\xfb\xfb\xc6\xef\x56\x65\x02"
      "\x4f\x6e\x97\x58\xbd\xa6\x34\xef\xa5\xa4\xfb\x5d\x94\x36\xe7\x17\x5a\xac"
      "\xd1\xa9\x97\x14\xae\x3a\xe5\xf3\xf6\x47\x60\xa4\x71\xb2\x8d\xcb\xf7\x2e"
      "\xa8\x1c\x45\x42\xf3\xf1\xe9\xed\x43\x13\x51\x42\xf1\x8c\xce\xc4\x96\xeb"
      "\x00\x64\x63\x10\xb6\xef\x7e\xef\x64\xbc\xcc\xb5\x8d\x66\xce\x60\x5a\x9f"
      "\xad\x56\x13\xc2\xbd\xe8\xe1\xd0\x3f\xb9\xe9\x27\x0d\x96\x63\xe7\xd6\xcd"
      "\xc7\x3e\x98\xf8\x95\x17\xda\x3b\x7a\xb0\x72\xf9\x27\x45\x23\xf3\xa7\x59"
      "\x98\x2f\x28\xf4\x16\xa2\x40\x7d\x1a\x94\xea\xc4\x28\xb7\xe8\x3b\xd8\x13"
      "\xa4\xad\x20\x52\x97\x7c\xd2\xf2\x12\xd5\x8b\x3b\x9c\x52\xfd\x4c\x26\x72"
      "\xb0\x9a\xda\x31\xd4\xb5\x78\x1c\x22\x77\xc2\x17\x44\x4f\x34\xff\x7b\xc4"
      "\x81\xc5\x6b\x71\xa6\x39\x5a\xbd\x34\x24\x93\x78\x35\xba\x20\x08\xce\x74"
      "\x22\x4a\x13\x52\x30\x53\xef\x8e\x6b\xb0\x85\xcb\x10\x0f\xd9\xf6\x1c\x39"
      "\x0d\x32\x77\x40\xe4\xee\x74\x61\x83\x2c\x37\x12\x07\xbe\xe7\xa6\x40\xc3"
      "\x61\x2a\x7f\xda\xf4\x14\xeb\x00\x0f\x2d\xaa\xca\xdf\xe5\xfc\x9e\xad\x95"
      "\xdf\x58\x46\x9b\xde\x75\x22\xc9\x27\x41\xbb\x82\x4f\x0d\x57\x5f\xa8\x9e"
      "\xde\xf3\x95\x73\xc3\xaa\x2a\x48\xf4\xda\x4e\xea\xfe\x0f\xa0\x14\x3c\xc8"
      "\xf8\x98\x61\x90\xa5\xe8\xf4\x97\xe5\x04\x01\x10\xc0\x8a\x41\x14\x39\xfa"
      "\xe4\x98\xbb\x92\x3a\x1f\xb1\xf1\x2d\x42\xfb\xdf\xf4\xc7\x92\x97\xb0\x35"
      "\x30\x5e\xf3\x52\x61\x34\x23\xfb\x95\x8d\x2e\xe6\xda\x28\xb4\x68\x99\xfb"
      "\x6e\x6d\x58\xca\xcc\x7b\xec\x67\xc4\x6b\x05\x88\x6f\x05\x09\x73\xed\x48"
      "\x7e\xb9\xd9\xd1\xb4\x3f\x4d\xf2\xe5\x1a\xd0\xc5\xed\xf3\x92\x14\x9f\xb7"
      "\xaa\xc9\xc9\xd0\x3a\xf6\xe8\xf0\xf6\xa6\x55\xb5\x7b\xd4\x68\x5d\x41\x79"
      "\xac\x08\xd0\xf8\xb6\xe2\x6a\x14\xd6\xc9\xeb\x7b\x4e\x16\xe2\xf3\x6e\x79"
      "\xdd\x31\x41\x51\x7e\x0f\x3f\x01\xa6\x44\xa7\x8f\xf2\x25\x16\xbf\x78\x70"
      "\xa6\x85\x1a\xc0\x3e\x38\xcf\xc9\x25\x54\x3f\x2c\x28\x53\xe9\xe4\x91\x8d"
      "\xb7\x40\x44\xe2\x86\xaa\xbe\x1f\x56\x42\xce\x99\xf7\xa5\x4a\xc1\xfe\x8d"
      "\x5f\x93\x27\x92\x80\x51\x13\xc7\xfb\x31\x30\x8a\xc1\xe3\x0e\x84\x01\x5b"
      "\x67\xe7\xaa\xf4\x0f\x70\x35\x02\x5c\x5b\xe4\x73\x14\x0e\xb2\x9d\x04\x50"
      "\x63\x09\x36\x9b\x7f\x20\xb8\x7d\xfb\x45\xa4\x0d\xcc\x03\xc5\x46\xe4\x83"
      "\x97\x81\x35\xfe\x1c\x9e\x60\xc3\x8d\x5b\x60\x90\x4a\x86\xd0\x5e\xab\x03"
      "\xb0\x83\x9f\xdc\x5e\x10\x78\xb2\xa6\xc4\x3f\x45\x84\xc8\x7b\xbd\xf5\xa9"
      "\x09\x77\xd1\xa7\xe7\xb7\x30\x49\xa7\x06\x59\x3a\xa6\x93\xe9\x87\x13\xb7"
      "\x03\x81\x3b\x10\x04\x85\x08\x02\x14\x60\x2c\x04\x8f\xde\x5f\xad\xe4\xe0"
      "\x7c\x12\xa5\x57\x22\x56\x5c\x71\xde\x28\x7b\x77\xc2\x27\x1c\x36\x3a\x6b"
      "\x18\x2a\x47\x58\x44\xe9\xc6\xc8\x42\xdc\xbe\x29\x6e\x55\xf6\x07\x5d\xc7"
      "\x19\x22\x38\x24\x5d\xd4\x2e\x49\x76\x35\x0a\x3f\xc6\xa2\x6e\x84\x08\x40"
      "\x4e\x85\xab\xa0\x11\x29\x75\xac\xa9\x76\xbc\xee\xd4\xfe\xd2\xc1\x17\x5c"
      "\x4e\x2a\x67\xf0\x1b\x21\x3b\xa8\x6f\x38\xd8\x6d\xb5\xeb\xdf\xb2\x27\x99"
      "\xe3\xed\x69\x42\xee\x0e\x06\x0b\x75\x0e\x61\x00\xe3\x54\xe5\x06\xe8\xd3"
      "\xa7\x5a\x41\xe5\x3e\x03\xe8\x4b\xcf\x33\xb1\xfc\x21\x06\x6b\xc1\x4c\x7e"
      "\xfb\x6c\xc9\x31\xf1\xb6\xdc\xad\xf0\xe2\x13\xfd\x9c\x67\xc9\x1f\x9a\x60"
      "\x63\x3f\x0a\x79\x27\x12\x56\xa9\x09\xfb\x03\xd4\x45\xd5\x37\xe1\x36\xa6"
      "\x01\xd4\x4e\x79\xbd\x35\x30\xab\xc2\x32\x4f\x83\xcd\x94\x0e\xee\x40\x87"
      "\x53\x7d\x6d\x86\x81\xf9\xb2\xc4\x6f\xec\x7d\x77\x0a\x25\xd2\x38\x74\x6c"
      "\x7f\x3f\x40\xcb\xa6\xc9\x76\x52\x58\x81\xcb\x5b\xd0\x82\xda\x5e\xd8\x01"
      "\xb1\x9a\x92\x1b\x34\x24\xa7\x6e\x20\x51\xa8\xc3\xfd\x4c\x77\xfc\x08\x60"
      "\xac\x69\x0a\x8d\xa3\x08\xb9\xee\x76\x18\xef\xe8\xc9\xba\x64\x26\x6d\xde"
      "\x9b\xff\xb8\x12\xaf\xe5\xdb\xe4\x6b\x04\xa1\xc2\xf5\x0b\x5c\xd6\x7a\x3e"
      "\x23\x07\xaa\xa3\x53\x61\xd9\x71\x63\xb4\xd1\x87\xc6\x59\x6b\x58\x9a\xdf"
      "\xe0\x65\xb4\x0c\x04\x95\x42\x43\xf9\x66\xdc\x7f\xa4\xb6\xc1\xd0\xe6\x9c"
      "\xf0\x51\x5e\xff\x27\x3a\x34\x1e\x5f\x70\x5d\xb0\x30\xef\xd8\x82\x34\xa9"
      "\xa4\xba\x31\x7b\xc8\xa6\x7f\x05\xf1\x07\x44\x59\xac\xed\x4c\xc7\x51\x34"
      "\xf6\x3b\x57\xe3\xb8\x95\xd6\xf6\x4a\xef\x11\x08\x34\x46\xdb\x0d\xb6\xac"
      "\x2e\x14\x00\xa7\x58\x6e\xca\x5e\x13\x10\x7f\x63\x57\xe0\xba\xbc\x25\x9a"
      "\x46\x35\xc6\x8d\x74\x11\xaa\x2b\xf7\x7a\x60\x6e\xea\x5b\xc5\xea\xd7\xa5"
      "\xb7\x34\x6f\xf2\x12\x1f\xe7\x6d\x8b\x44\x0a\x31\x5c\xbf\x05\x6c\x5f\x80"
      "\x02\x05\x74\x3a\x08\xf5\xf4\x76\x29\x38\x9c\x77\x74\xe1\x05\x87\x26\xb6"
      "\xf1\x76\xa7\xa8\x18\xf5\x08\xaf\x5a\x46\x6b\x3b\xa3\xf5\x08\xe8\x55\x4d"
      "\x27\x0f\x51\x78\xd1\xac\xb3\xaa\x4c\x9b\xa8\x01\x7a\x34\xb8\x4e\x07\x58"
      "\xe2\x33\x8c\xd5\x81\x0f\x65\x9d\x6f\xc1\x14\x57\x0e\x2a\xee\xf8\x14\x5b"
      "\xaf\xec\xc8\xad\xcc\xc2\xa5\xf5\x08\xb6\xf3\x22\xad\x0f\xab\xa7\x41\x87"
      "\x6c\x3e\x62\x08\xc5\xef\x00\xb7\x17\x6c\x8b\x7b\x68\x53\xf2\x00\x55\xe4"
      "\x46\x26\x5b\x97\x45\x5c\x1f\x7b\xe8\xa1\x2a\x4f\x87\x38\x43\x2b\x09\x79"
      "\x75\xda\x01\xb0\x62\x0a\x33\x9e\xe7\xa9\xcf\x0f\xb7\x81\x0a\x66\x36\xc9"
      "\x69\x12\x91\x87\x29\xf3\x52\x89\x5e\xbe\x00\xee\x10\xe6\xf6\x76\x5d\xa8"
      "\x87\x16\xe6\x97\x58\xb7\x39\xce\xcd\x1e\xa5\x73\x28\x60\x31\x6b\x00\xdd"
      "\x08\x3d\xac\xb8\x1a\x5d\x80\x58\x84\xde\xb3\xd6\xe8\xc0\x6b\x51\x44\x6e"
      "\x1d\xae\xa5\xd4\x63\xd3\x5b\x7b\xbc\xf2\x2c\x85\x5b\x29\x32\x7b\x9f\x20"
      "\xf6\x7c\x74\x71\xf1\xbe\xd0\xc7\xc3\xc1\x65\x70\xd7\x98\x86\x8f\x04\x17"
      "\x91\x54\xe3\xbd\x6b\xd1\xcc\x3b\xb4\x19\xb0\xff\xba\xaf\xf4\x95\xfc\xd7"
      "\x14\x4d\x52\x60\xaa\x2b\xe0\xe6\x1e\xb9\x2e\xa2\xa0\x58\x5c\xc3\xaf\x20"
      "\x8e\x9a\x29\x71\x3e\x4d\xa8\xbd\xe1\xd4\x28\x51\xd2\x80\x64\xde\x24\xf4"
      "\x95\x2d\xc2\x67\x84\x09\xa9\xfc\x4a\xee\xb6\x42\x1a\x13\x96\xe4\xdb\x46"
      "\xeb\x3d\x99\xf6\x42\x02\x75\xf3\xb3\x1e\x6b\x99\x19\xa4\x6e\x44\x66\xaa"
      "\x05\xe3\x05\xd2\x15\x9f\xd2\x1e\x76\x2f\xbc\x0a\x21\x77\x96\x18\xa4\x16"
      "\x0a\x6b\x04\xb9\xe7\xcc\x81\x12\x97\xdd\xd7\x3f\x5c\xf4\xf6\xfb\x89\x68"
      "\xdc\x71\x32\x2f\x70\xb2\x8c\x93\x83\x0f\xa6\x57\xf4\xdf\xf8\xd5\xda\x73"
      "\xf9\x08\xa8\xb6\x13\xd8\x0f\x4f\x81\xf1\xb3\x5d\x0d\xa9\x8e\xc8\x04\xe9"
      "\xf4\x80\xcc\xce\x5b\xcd\x0b\x32\x8c\x0e\xfd\xc0\xaa\x81\xaa\x4a\x9c\x3f"
      "\x8a\xdd\x38\x89\x64\x3a\x4a\x26\x40\xca\x85\x66\x1e\xac\x58\x1b\x73\x99"
      "\x7f\x17\x97\xfe\x2f\xd6\x7f\x22\x72\x9e\xf4\x3b\xfd\x41\x73\xe1\x73\xe1"
      "\x51\xdb\xc1\x73\x42\x4e\x7b\x0f\x69\xdd\x2e\xfe\x1c\xd6\xdd\x92\x64\xbf"
      "\xb3\x1e\xe0\x81\x62\x76\x5c\x2a\x2c\xc6\xd4\x57\xd2\x0d\xea\x0f\x36\x37"
      "\x11\xe2\x63\xf0\xe3\x50\xf0\x96\x75\x92\x97\xa9\x5e\x55\xc2\x36\x98\xad"
      "\xca\x39\x35\xf0\xc5\x40\x6d\x9b\xc0\xa5\xbd\xe8\xc3\x55\xb9\xec\x2b\xbc"
      "\x3c\x7b\xdc\x05\x54\x96\xb9\x56\x18\x2c\xae\x6f\xee\x24\xde\xf8\x3b\x14"
      "\x59\x86\x16\xd8\x89\x3a\x7a\x72\x94\x83\xcb\x97\x80\x57\x65\xd2\x27\xbd"
      "\x4c\xee\x52\xa8\x3e\x41\xcb\xdb\xe9\x63\xb9\x70\x88\xfc\x78\xe0\xa0\x0b"
      "\xf6\x86\x72\xe3\xb8\xf9\x65\x2c\x5e\x3a\xde\x8b\xc4\x23\xcd\x89\x97\x80"
      "\x63\x4d\x17\x90\x27\x0f\x04\x71\x5f\xd7\x8a\xd5\xaf\xbf\x0f\x96\xb2\x9e"
      "\x0d\x98\x8c\x34\xa4\xa1\x30\x5c\xae\x86\x99\xbe\x94\xc8\xb5\x03\x9c\x0f"
      "\xf1\x92\x18\x37\xb8\x9e\x6b\x14\xa8\x94\x7b\xd8\x5c\xcd\x67\x45\x58\xca"
      "\x82\x8e\x1e\x46\xa3\x51\xb7\x8d\x0d\xc8\x9e\x89\x49\x96\x90\x99\x7f\x10"
      "\xd7\x69\x87\x64\x41\xe0\x25\x69\xf8\xfa\x2b\xa1\x69\xac\xac\x73\x2e\x76"
      "\x0c\x42\x34\x93\x93\x2b\xac\x11\x4a\xf5\xce\x81\x51\xf1\xa5\x81\xb5\x1d"
      "\x0a\x74\x51\x97\x83\x23\x79\x98\x29\x71\x22\xc8\x82\x3b\x61\x58\xd9\xb8"
      "\x3d\x93\x5d\x3a\x7c\xec\xc3\xca\x86\xbe\x3b\x73\x57\xa4\xd9\x1e\x87\x3f"
      "\x9d\x98\x5d\x9b\xc7\xb0\x91\x1a\xf8\x67\x90\x62\x90\x3d\x6a\x64\x94\x14"
      "\x03\x88\xc0\x6f\xa0\x64\xde\xf3\x36\xe6\x0c\xd3\x3b\x1f\xc0\x44\xed\x1b"
      "\xee\xd1\x7b\x98\x57\xdb\x63\xed\xa1\x9e\x07\xc0\x83\xc5\x26\x6f\x0d\xd4"
      "\x4a\xab\x15\xc0\x5a\x90\xdb\x3c\x56\x25\xa8\x5d\x9b\xcc\x1d\x93\xf9\x41"
      "\xcb\x96\x90\x16\x78\x43\x5e\xe5\x63\xac\xee\x33\xa3\xc2\x13\xf5\xc8\x57"
      "\x91\xf7\x84\x10\xb0\xb8\x19\x16\xda\x51\x6b\xed\x7b\x4a\xc5\x6b\xba\xe4"
      "\xb2\x6d\x46\xaa\xa0\x23\xe2\x73\xa8\x59\xc2\x5a\x11\xf2\xd8\x16\x83\x78"
      "\xb4\x77\x67\x81\x07\x46\x33\xc8\x68\x60\x35\x0f\xf8\x18\x70\xd8\x31\x4d"
      "\xe3\x5a\x5c\xee\x6d\xb5\x85\x76\x21\x95\x3a\xd2\x7b\x2e\xeb\x57\x30\x39"
      "\x0d\x45\x0b\x3c\x80\xce\x7a\xd2\xb1\x01\xf8\x84\xdb\x67\x16\xf5\xc3\xaa"
      "\x48\xcd\xbd\x36\xac\x89\xdf\xbd\xf5\x25\xd1\x74\x72\xa3\x6c\x4f\x68\xa6"
      "\x38\x51\x2e\x4d\x99\x43\x6e\x63\x58\xbb\x82\x9c\x10\x35\x0c\x6e\x65\x93"
      "\x40\xaf\x89\xa4\x20\x0b\xd2\xf0\x20\xa2\x0c\xd3\x5d\xaa\xf2\x84\x21\x15"
      "\x34\x41\xb1\x8d\x83\xf7\xef\x78\x02\x72\x6e\x04\xe5\x1c\xd0\x2b\xf1\x75"
      "\x31\xe9\x08\xbf\x85\x10\x97\x89\x95\x52\x93\x8b\xa8\x30\xf3\x24\x72\x34"
      "\xec\x92\x72\x2a\xa6\x5a\x5b\x72\x32\x3b\x64\x95\xa6\x1b\x40\xc8\xc7\xce"
      "\x9e\xdf\x3b\x43\x52\x45\xd6\xbf\x11\x36\x0e\xaa\x80\x8c\xea\xd2\x1d\xf0"
      "\x1a\x6d\x3d\x60\x70\x70\xd4\x54\x1d\xd6\xce\x15\x80\x58\x9b\x45\x39\x69"
      "\xbe\x88\xe4\xb3\x89\x44\xbf\x77\x66\xb9\xcf\xbb\xe4\x5b\x7b\x7c\x4e\x7f"
      "\x0a\x56\x6b\xf8\x5d\x64\x6d\x4a\x50\x48\xf4\x9a\xe4\xff\x15\x07\xb2\xf6"
      "\x45\xb8\xed\x07\x8a\x1b\xba\x0e\x0e\xdc\x4c\x72\xfe\x35\x27\xb0\x3a\x8a"
      "\xac\x27\x7f\x72\x13\x03\x93\x64\x78\x9c\x5f\xd3\x16\xfe\xd6\xee\xf8\x2d"
      "\xad\x3d\xcf\xcd\xa4\x16\xf0\xf5\x6a\x11\xc0\xa3\x75\x8e\x95\x0e\xf1\xfa"
      "\x13\xab\xc0\x78\xde\x28\x86\xee\x0c\xeb\x49\x49\x52\x87\xc0\xcd\xd7\x2a"
      "\x99\xfb\x14\xf6\xa9\xc4\xad\x09\xcb\xf2\xb3\x52\x0c\xee\x0a\x4d\x4b\xcf"
      "\xef\x28\xd7\xa7\x07\x9c\xc2\x71\xac\xad\x46\x8c\xa6\x35\xc1\x8a\x34\x94"
      "\x5e\x6d\x18\x97\x29\xb9\x23\x49\x03\x7d\xfa\x0a\x5a\x8f\x35\x9d\x6a\xb0"
      "\x10\x96\xf0\xee\x36\x22\x62\xb5\xd4\x87\xf4\xbd\xb2\xfe\x65\x75\xe1\xaa"
      "\x22\x8c\x2a\xdc\xd6\x2d\x66\xdf\xc5\x57\xbd\x5a\x3c\xf0\x92\x04\x62\x74"
      "\x7c\xa9\x02\x3f\xb2\xa6\x49\xcf\xa6\x27\x22\x15\x62\x6a\xf5\x2f\x94\x35"
      "\x04\x98\x9e\xe4\x8d\x9d\xcb\x10\x07\x07\xe6\x60\xb2\x71\xd7\xdb\xae\xd4"
      "\xf3\x73\xe0\xe8\xd5\x09\xc4\xa4\x37\x7b\x41\xc6\x54\x18\x1e\xd1\x23\x6e"
      "\xf5\x97\x88\x44\xdf\x60\x28\xf2\xab\x36\xad\xf7\x8f\x0c\xa8\xdd\x92\x18"
      "\x31\xfc\x34\x44\x47\x1f\x26\xe1\xe8\x99\x95\xee\xf8\x6c\x53\x41\xf7\x43"
      "\x2d\xdf\x9a\x02\xbd\xfc\x5b\xed\xa1\xd7\xae\xe5\x5c\xf7\x22\xa4\x6f\xb6"
      "\x6b\x90\xea\x7c\x9f\xfa\xda\xc5\x3b\x7b\x52\xfb\xf7\x3a\x44\x7a\xce\x7f"
      "\x8c\x9e\x78\x4d\xc6\x87\x33\x96\xc9\xb9\xd4\x85\x70\xaa\x7f\x2b\xbf\x30"
      "\x7e\xd4\x69\x04\x62\x78\x2b\x40\xe3\x0e\x89\x64\x53\x8c\x3a\x05\xa4\x0d"
      "\x52\x4f\xdf\x76\x58\xae\xac\x81\xea\x9a\xe3\xd5\x42\xdb\x40\xd7\x97\x71"
      "\x85\xf0\x40\x74\x4c\x80\xe4\x24\x0f\x72\x74\x85\xd6\x65\xbf\x52\x98\xab"
      "\x70\x2d\x42\x98\x82\x99\x01\x19\x8e\x9a\x88\x30\x25\xfb\x04\x0e\xb7\xc9"
      "\xd0\x67\x5d\xc6\xcd\x4d\xa6\x21\xd2\x93\x21\x34\xcb\xde\x65\xb7\x7e\xa3"
      "\x53\xef\x27\x32\x2b\x2a\xc1\xb1\x51\x4e\x0d\x04\xc1\x01\x02\xbd\x3a\xe6"
      "\x44\x5b\x7d\xd3\x8d\x22\x60\xb1\x94\x7d\xd5\x89\x01\x06\x27\xfa\x27\x3f"
      "\x8d\x41\x31\x71\xcf\xd4\xb3\x24\x4a\xb6\x1e\x0f\xce\x5b\x66\x91\xb8\x8e"
      "\x73\x02\x99\x5d\x53\x20\x81\x9d\xd3\xd7\x7f\x6d\x9b\x26\x10\x3b\x15\xf8"
      "\xe7\x80\xc4\x14\x16\xec\xbc\x32\xb3\xb7\x28\x58\x91\x75\x08\x05\xa6\xa1"
      "\xc9\x12\xe6\xf8\x09\xf6\x48\xc5\x7d\xf9\x1e\x9d\xab\xdf\xe1\x98\xcc\x2d"
      "\xee\x29\xa1\xc7\x4e\xc7\xd0\xf0\x4f\x1a\x05\x5b\xcd\x81\x81\x6b\x96\xfa"
      "\x9c\x52\xb5\x5f\x1b\xfe\x97\x64\x17\x9b\xb5\x69\xa6\x4d\xc5\x97\x3b\x75"
      "\xea\x45\x75\x7c\xc7\xf8\x27\x1f\x86\xc5\x97\x58\x6b\x6d\x68\x35\x9b\xb4"
      "\xea\xa2\xd3\xaa\x75\x47\x62\x46\xdf\xcf\xdb\x02\x23\x92\x0e\x54\x5e\x09"
      "\x6f\x7f\xa8\xe5\x8d\x39\xbf\x46\xd4\xa6\xbc\x31\x8d\x93\xba\xbf\x85\x98"
      "\x69\x12\x58\xa2\x98\xd3\x11\x3c\xae\xc1\x88\x9d\x91\x29\x8e\x30\xe2\xca"
      "\xf2\x52\x7f\x93\xcd\x8d\xe7\x4b\xb5\x0b\xd4\x49\xfd\x21\xac\x56\xd7\xd2"
      "\x56\x2b\x0f\xe8\xca\xa1\x6c\x99\x98\xd7\x07\x87\xc7\x35\x6e\xf5\xc9\xa9"
      "\x24\xa8\xbf\x3b\x79\x1e\x6a\x42\xdc\x3c\x30\xc3\x8d\x35\xdf\x8c\x56\x43"
      "\xdf\x05\x91\x5f\xdb\x70\x7b\x7e\xb2\x82\x7f\x29\x78\xa6\xff\xd6\xf8\xab"
      "\x82\xe7\xef\x0f\x4e\x30\x25\x9a\x10\x6b\xfe\xe0\x4c\xc4\xb2\xd8\xad\x2f"
      "\x96\xe9\xb1\x3a\xee\x78\xfe\x76\x0a\x75\xaf\x7e\x4b\x7d\x7c\xe6\xf2\x90"
      "\x4a\xdc\xd7\x77\x80\x93\x68\xae\xb0\xb7\x3b\x3d\x88\x8f\x03\x8b\x15\x47"
      "\x5e\x78\xae\xbc\x08\xb9\xd1\xc8\xea\x21\xa8\x95\x62\x4a\xe4\xbb\x51\xdf"
      "\x32\x0d\x81\x8e\x69\xa4\x52\x45\xe7\xdb\xc9\xdf\x6d\x9a\x47\xad\x1e\x93"
      "\x3f\x15\xfd\x94\x51\xc4\xdc\xb0\x31\xc5\x1f\xe3\x50\x0a\xf3\x6b\x66\x0e"
      "\x09\x61\x0b\xa4\x59\x9b\xbf\xcd\x70\x6f\x3f\xc5\x70\xa9\xdc\x86\x36\x9a"
      "\xa1\x00\x19\x22\xc0\xb7\x43\x1b\x10\x45\xf0\xaa\x34\x8d\xef\xf4\xb1\x48"
      "\x96\xc4\xa7\x96\xd8\x81\xa1\x05\x53\x0b\x23\x72\xe7\x0c\xfe\x57\xd6\x56"
      "\x73\x85\xea\x58\x18\x3d\xd3\x86\xa5\x12\x88\xcb\xe9\xe9\x11\x28\x96\x78"
      "\xbb\xe0\x86\x0b\xea\x07\x88\xf5\xcb\x34\xe1\x1d\x78\xf4\x2e\x7b\xa6\x2d"
      "\x2a\xbd\x90\x2e\xce\xd2\x8f\xc4\xc8\x2c\xc5\x84\x8a\x61\xe8\x76\x6f\x72"
      "\xde\x0b\x69\x1f\xe5\xe3\x6e\x3b\x4b\xec\xc5\x57\x08\x86\x5b\x31\x39\xd1"
      "\x6d\xf4\xea\x07\xb6\x49\x9f\xcc\xaa\xa5\x47\x81\x49\x82\x13\x6d\xac\x38"
      "\x3b\x02\xc4\xca\x9f\x74\x35\x1d\x4b\xbc\x55\x4a\xf3\x04\x1a\xb9\x1b\x4c"
      "\x56\xdc\xcf\x91\x5a\x64\x95\x25\x96\xe5\xec\xa9\x16\x6f\x0d\x8e\x5c\x34"
      "\x72\xaf\x48\x25\x84\x39\x6b\x14\x74\x03\x84\xc1\x3f\x7f\xe6\xa8\x43\x4a"
      "\x34\x99\xb2\x1c\x6f\xf9\xab\x4b\x71\xcd\x65\x2f\x5a\x8e\x2d\xaf\xc1\x2f"
      "\xb6\xd6\xf8\xaf\xc6\xbb\xc0\xc9\xbb\xd3\x85\x25\x08\xec\xf1\xe0\x1e\x72"
      "\xdd\xb2\x90\xc7\x8a\x56\x17\x6b\xaa\x7a\x40\x0f\x1a\x57\x21\x9f\x0c\x16"
      "\x82\xd2\xb7\x89\xcb\x1b\xa4\x1f\x1f\x43\x42\xea\x53\x93\xe6\xe9\x67\x47"
      "\x8c\xe7\xab\xdd\x39\x8a\x11\xf5\xc2\xd0\x08\x85\x68\x26\x46\x35\xbd\xf2"
      "\x73\xf2\xcd\x55\xb7\x42\x7b\x52\x2c\x4b\x21\xb7\xe6\xb9\xbd\x1a\x85\xf9"
      "\xea\xfa\x0d\x13\xbd\xc5\x33\x83\x60\x82\x9d\x1f\x39\x11\x9c\x88\xd3\x4d"
      "\x1e\xa6\x46\xc3\x9f\xf0\xb3\xe5\xbf\x54\xf1\x3c\x0d\x25\x1d\x9e\x49\x2c"
      "\x26\x2d\xdc\xd2\x7c\x3c\x74\x91\x55\x4d\x8d\x06\xa9\xcc\x63\xee\x70\xf5"
      "\x73\xcd\x4c\xb1\x83\x27\x86\x39\x3f\xf8\xfa\x63\xe0\xb0\x26\x5e\x0b\x5e"
      "\xbd\xd2\xb8\xbd\xda\x44\x22\x08\x75\x7d\x8f\xaa\xf5\x35\xd7\xb0\x6b\xe8"
      "\x6a\xf9\x18\x41\x9a\xa6\x39\x44\xdc\x5a\x8c\x30\x3d\xb1\x94\xa2\x25\xac"
      "\x40\xbd\x75\xe9\xc4\x4d\x67\x30\x9d\xc3\x4e\x2e\x87\x48\x9c\x72\x98\xe7"
      "\x46\x02\x97\x0c\xe8\xa4\xdb\x57\x85\x41\x7c\x6e\xb8\x43\xd4\x25\x58\xd8"
      "\x30\xc8\xce\x0a\x87\x50\xac\x4c\x6f\xda\x09\x3d\xac\xa9\x69\xfe\xba\x5f"
      "\xc6\xae\x8d\x10\xd5\xe2\xe0\xfd\x2e\x8b\xd8\xc7\x10\x44\x34\xeb\xee\x08"
      "\x20\x3c\xad\x69\x58\x78\xd8\x3f\x62\xc0\xa0\xb3\x7f\x08\x0c\xf7\xfc\x40"
      "\x10\x44\x0c\xee\x1d\x8f\xa7\xd7\x1b\xc9\x43\x58\xa9\xd6\xf8\xcc\x91\x10"
      "\x17\x0a\x8c\x8c\x47\x67\x08\x17\x98\x21\x05\x12\xd3\x8c\xc6\x2b\x0a\x66"
      "\x7c\xd0\xf5\x3c\x9f\xe7\x1a\xc2\x1a\x15\x81\xd7\x17\x66\xfd\x16\xe2\x4e"
      "\xee\x8e\xbb\x5e\x14\x75\xea\x1a\x8c\x5f\x0d\x7e\x2a\x53\xc4\x67\x48\x27"
      "\xbf\xd8\x3f\xd2\xd5\xd6\xf3\x2b\x60\x7a\x31\x7f\x16\xdc\x35\x36\xa6\xed"
      "\x4d\xb0\xfe\x50\x6c\x66\xa6\xef\xfa\xa6",
      4096);
  *(uint16_t*)0x20000040 = 0xa;
  *(uint16_t*)0x20000042 = htobe16(0x4e23);
  *(uint32_t*)0x20000044 = 0xc9;
  *(uint8_t*)0x20000048 = -1;
  *(uint8_t*)0x20000049 = 2;
  *(uint8_t*)0x2000004a = 0;
  *(uint8_t*)0x2000004b = 0;
  *(uint8_t*)0x2000004c = 0;
  *(uint8_t*)0x2000004d = 0;
  *(uint8_t*)0x2000004e = 0;
  *(uint8_t*)0x2000004f = 0;
  *(uint8_t*)0x20000050 = 0;
  *(uint8_t*)0x20000051 = 0;
  *(uint8_t*)0x20000052 = 0;
  *(uint8_t*)0x20000053 = 0;
  *(uint8_t*)0x20000054 = 0;
  *(uint8_t*)0x20000055 = 0;
  *(uint8_t*)0x20000056 = 0;
  *(uint8_t*)0x20000057 = 1;
  *(uint32_t*)0x20000058 = 7;
  syscall(__NR_sendto, r[0], 0x20000100, 0x1000, 0x4004, 0x20000040, 0x1c);
  memcpy((void*)0x200f8000, "\xcd\x39\x0b\x08\x1b\xf2", 6);
  *(uint8_t*)0x200f8006 = 0xaa;
  *(uint8_t*)0x200f8007 = 0xaa;
  *(uint8_t*)0x200f8008 = 0xaa;
  *(uint8_t*)0x200f8009 = 0xaa;
  *(uint8_t*)0x200f800a = 0;
  *(uint8_t*)0x200f800b = 0;
  *(uint16_t*)0x200f800c = htobe16(0x86dd);
  STORE_BY_BITMASK(uint8_t, 0x200f800e, 0, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x200f800e, 6, 4, 4);
  memcpy((void*)0x200f800f, "\x08\xde\x06", 3);
  *(uint16_t*)0x200f8012 = htobe16(0x30);
  *(uint8_t*)0x200f8014 = 0x3a;
  *(uint8_t*)0x200f8015 = 0;
  *(uint8_t*)0x200f8016 = 0;
  *(uint8_t*)0x200f8017 = 0;
  *(uint8_t*)0x200f8018 = 0;
  *(uint8_t*)0x200f8019 = 0;
  *(uint8_t*)0x200f801a = 0;
  *(uint8_t*)0x200f801b = 0;
  *(uint8_t*)0x200f801c = 0;
  *(uint8_t*)0x200f801d = 0;
  *(uint8_t*)0x200f801e = 0;
  *(uint8_t*)0x200f801f = 0;
  *(uint8_t*)0x200f8020 = -1;
  *(uint8_t*)0x200f8021 = -1;
  *(uint32_t*)0x200f8022 = htobe32(0xe0000002);
  *(uint8_t*)0x200f8026 = -1;
  *(uint8_t*)0x200f8027 = 2;
  *(uint8_t*)0x200f8028 = 0;
  *(uint8_t*)0x200f8029 = 0;
  *(uint8_t*)0x200f802a = 0;
  *(uint8_t*)0x200f802b = 0;
  *(uint8_t*)0x200f802c = 0;
  *(uint8_t*)0x200f802d = 0;
  *(uint8_t*)0x200f802e = 0;
  *(uint8_t*)0x200f802f = 0;
  *(uint8_t*)0x200f8030 = 0;
  *(uint8_t*)0x200f8031 = 0;
  *(uint8_t*)0x200f8032 = 0;
  *(uint8_t*)0x200f8033 = 0;
  *(uint8_t*)0x200f8034 = 0;
  *(uint8_t*)0x200f8035 = 1;
  *(uint8_t*)0x200f8036 = 2;
  *(uint8_t*)0x200f8037 = 0;
  *(uint16_t*)0x200f8038 = 0;
  *(uint32_t*)0x200f803a = htobe32(0);
  STORE_BY_BITMASK(uint8_t, 0x200f803e, 0, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x200f803e, 6, 4, 4);
  memcpy((void*)0x200f803f, "\x94\x33\xdf", 3);
  *(uint16_t*)0x200f8042 = htobe16(0);
  *(uint8_t*)0x200f8044 = 0x3a;
  *(uint8_t*)0x200f8045 = 0;
  *(uint64_t*)0x200f8046 = htobe64(0);
  *(uint64_t*)0x200f804e = htobe64(1);
  *(uint64_t*)0x200f8056 = htobe64(0);
  *(uint64_t*)0x200f805e = htobe64(1);
  struct csum_inet csum_1;
  csum_inet_init(&csum_1);
  csum_inet_update(&csum_1, (const uint8_t*)0x200f8016, 16);
  csum_inet_update(&csum_1, (const uint8_t*)0x200f8026, 16);
  uint32_t csum_1_chunk_2 = 0x30000000;
  csum_inet_update(&csum_1, (const uint8_t*)&csum_1_chunk_2, 4);
  uint32_t csum_1_chunk_3 = 0x3a000000;
  csum_inet_update(&csum_1, (const uint8_t*)&csum_1_chunk_3, 4);
  csum_inet_update(&csum_1, (const uint8_t*)0x200f8036, 48);
  *(uint16_t*)0x200f8038 = csum_inet_digest(&csum_1);
  syz_emit_ethernet(0x66, 0x200f8000, 0);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  int pid = do_sandbox_none();
  int status = 0;
  while (waitpid(pid, &status, __WALL) != pid) {
  }
  return 0;
}