// kernel BUG at fs/f2fs/segment.c:LINE!
// https://syzkaller.appspot.com/bug?id=9546e292c2f552add1af9c50cd48fc24df304b7f
// status:fixed
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/futex.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_tun.h>
#include <linux/ip.h>
#include <linux/loop.h>
#include <linux/tcp.h>
#include <net/if_arp.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
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
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

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

static __thread int skip_segv;
static __thread jmp_buf segv_env;

static void segv_handler(int sig, siginfo_t* info, void* uctx)
{
  uintptr_t addr = (uintptr_t)info->si_addr;
  const uintptr_t prog_start = 1 << 20;
  const uintptr_t prog_end = 100 << 20;
  if (__atomic_load_n(&skip_segv, __ATOMIC_RELAXED) &&
      (addr < prog_start || addr > prog_end)) {
    _longjmp(segv_env, 1);
  }
  doexit(sig);
}

static void install_segv_handler()
{
  struct sigaction sa;

  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = SIG_IGN;
  syscall(SYS_rt_sigaction, 0x20, &sa, NULL, 8);
  syscall(SYS_rt_sigaction, 0x21, &sa, NULL, 8);

  memset(&sa, 0, sizeof(sa));
  sa.sa_sigaction = segv_handler;
  sa.sa_flags = SA_NODEFER | SA_SIGINFO;
  sigaction(SIGSEGV, &sa, NULL);
  sigaction(SIGBUS, &sa, NULL);
}

#define NONFAILING(...)                                                        \
  {                                                                            \
    __atomic_fetch_add(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
    if (_setjmp(segv_env) == 0) {                                              \
      __VA_ARGS__;                                                             \
    }                                                                          \
    __atomic_fetch_sub(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
  }

static void use_temporary_dir()
{
  char tmpdir_template[] = "./syzkaller.XXXXXX";
  char* tmpdir = mkdtemp(tmpdir_template);
  if (!tmpdir)
    fail("failed to mkdtemp");
  if (chmod(tmpdir, 0777))
    fail("failed to chmod");
  if (chdir(tmpdir))
    fail("failed to chdir");
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

extern unsigned long long procid;

struct fs_image_segment {
  void* data;
  uintptr_t size;
  uintptr_t offset;
};

#define IMAGE_MAX_SEGMENTS 4096
#define IMAGE_MAX_SIZE (129 << 20)

#define SYZ_memfd_create 319

static uintptr_t syz_mount_image(uintptr_t fs, uintptr_t dir, uintptr_t size,
                                 uintptr_t nsegs, uintptr_t segments,
                                 uintptr_t flags, uintptr_t opts)
{
  char loopname[64];
  int loopfd, err = 0, res = -1;
  uintptr_t i;
  struct fs_image_segment* segs = (struct fs_image_segment*)segments;

  if (nsegs > IMAGE_MAX_SEGMENTS)
    nsegs = IMAGE_MAX_SEGMENTS;
  for (i = 0; i < nsegs; i++) {
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
  int memfd = syscall(SYZ_memfd_create, "syz_mount_image", 0);
  if (memfd == -1) {
    err = errno;
    goto error;
  }
  if (ftruncate(memfd, size)) {
    err = errno;
    goto error_close_memfd;
  }
  for (i = 0; i < nsegs; i++) {
    if (pwrite(memfd, segs[i].data, segs[i].size, segs[i].offset) < 0) {
    }
  }
  snprintf(loopname, sizeof(loopname), "/dev/loop%llu", procid);
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
  mkdir((char*)dir, 0777);
  NONFAILING(if (strcmp((char*)fs, "iso9660") == 0) flags |= MS_RDONLY);
  if (mount(loopname, (char*)dir, (char*)fs, flags, (char*)opts)) {
    err = errno;
    goto error_clear_loop;
  }
  res = 0;
error_clear_loop:
  ioctl(loopfd, LOOP_CLR_FD, 0);
error_close_loop:
  close(loopfd);
error_close_memfd:
  close(memfd);
error:
  errno = err;
  return res;
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

static void execute_one();
extern unsigned long long procid;

void loop()
{
  while (1) {
    execute_one();
  }
}

struct thread_t {
  int created, running, call;
  pthread_t th;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;
static int collide;

static void* thr(void* arg)
{
  struct thread_t* th = (struct thread_t*)arg;
  for (;;) {
    while (!__atomic_load_n(&th->running, __ATOMIC_ACQUIRE))
      syscall(SYS_futex, &th->running, FUTEX_WAIT, 0, 0);
    execute_call(th->call);
    __atomic_fetch_sub(&running, 1, __ATOMIC_RELAXED);
    __atomic_store_n(&th->running, 0, __ATOMIC_RELEASE);
    syscall(SYS_futex, &th->running, FUTEX_WAKE);
  }
  return 0;
}

static void execute(int num_calls)
{
  int call, thread;
  running = 0;
  for (call = 0; call < num_calls; call++) {
    for (thread = 0; thread < sizeof(threads) / sizeof(threads[0]); thread++) {
      struct thread_t* th = &threads[thread];
      if (!th->created) {
        th->created = 1;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, 128 << 10);
        pthread_create(&th->th, &attr, thr, th);
      }
      if (!__atomic_load_n(&th->running, __ATOMIC_ACQUIRE)) {
        th->call = call;
        __atomic_fetch_add(&running, 1, __ATOMIC_RELAXED);
        __atomic_store_n(&th->running, 1, __ATOMIC_RELEASE);
        syscall(SYS_futex, &th->running, FUTEX_WAKE);
        if (collide && call % 2)
          break;
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 20 * 1000 * 1000;
        syscall(SYS_futex, &th->running, FUTEX_WAIT, 1, &ts);
        if (running)
          usleep((call == num_calls - 1) ? 10000 : 1000);
        break;
      }
    }
  }
}

unsigned long long procid;
void execute_call(int call)
{
  switch (call) {
  case 0:
    NONFAILING(memcpy((void*)0x20000040, "./file0", 8));
    syscall(__NR_chown, 0x20000040, 0, 0);
    break;
  case 1:
    NONFAILING(memcpy((void*)0x20000000, "f2fs", 5));
    NONFAILING(memcpy((void*)0x20000100, "./file0", 8));
    NONFAILING(*(uint64_t*)0x20000200 = 0x20010000);
    NONFAILING(*(uint64_t*)0x20000208 = 0);
    NONFAILING(*(uint64_t*)0x20000210 = 0x400);
    NONFAILING(*(uint64_t*)0x20000218 = 0x20010100);
    NONFAILING(*(uint64_t*)0x20000220 = 0);
    NONFAILING(*(uint64_t*)0x20000228 = 0x860);
    NONFAILING(*(uint64_t*)0x20000230 = 0x20010200);
    NONFAILING(*(uint64_t*)0x20000238 = 0);
    NONFAILING(*(uint64_t*)0x20000240 = 0xa80);
    NONFAILING(*(uint64_t*)0x20000248 = 0x20010300);
    NONFAILING(*(uint64_t*)0x20000250 = 0);
    NONFAILING(*(uint64_t*)0x20000258 = 0xb80);
    NONFAILING(*(uint64_t*)0x20000260 = 0x20010400);
    NONFAILING(memcpy(
        (void*)0x20010400,
        "\x10\x20\xf5\xf2\x01\x00\x07\x00\x09\x00\x00\x00\x03\x00\x00\x00\x0c"
        "\x00\x00\x00\x09\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x28\x00\x00\x00\x00\x00\x00\x0c\x00\x00\x00\x13\x00\x00"
        "\x00\x02\x00\x00\x00\x02\x00\x00\x00\x02\x00\x00\x00\x01\x00\x00\x00"
        "\x0c\x00\x00\x00\x00\x02\x00\x00\x00\x02\x00\x00\x00\x06\x00\x00\x00"
        "\x0a\x00\x00\x00\x0e\x00\x00\x00\x10\x00\x00\x03\x00\x00\x00\x01\x00"
        "\x00\x00\x02\x00\x00\x00\x5b\xe8\xf8\x91\x37\x11\x43\x91\xb6\x1d\x22"
        "\x28\x94\x8a\xeb\x15\x73\x00\x79\x00\x7a\x00\x30\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00",
        160));
    NONFAILING(*(uint64_t*)0x20000268 = 0xa0);
    NONFAILING(*(uint64_t*)0x20000270 = 0x1400);
    NONFAILING(*(uint64_t*)0x20000278 = 0x20010500);
    NONFAILING(*(uint64_t*)0x20000280 = 0);
    NONFAILING(*(uint64_t*)0x20000288 = 0x1860);
    NONFAILING(*(uint64_t*)0x20000290 = 0x20010600);
    NONFAILING(*(uint64_t*)0x20000298 = 0);
    NONFAILING(*(uint64_t*)0x200002a0 = 0x1a80);
    NONFAILING(*(uint64_t*)0x200002a8 = 0x20010700);
    NONFAILING(*(uint64_t*)0x200002b0 = 0);
    NONFAILING(*(uint64_t*)0x200002b8 = 0x1b80);
    NONFAILING(*(uint64_t*)0x200002c0 = 0x20010800);
    NONFAILING(memcpy(
        (void*)0x20010800,
        "\x0d\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x0c"
        "\x00\x00\x00\x00\x00\x00\x00\x0a\x00\x00\x00\x0b\x00\x00\x00\x06\x00"
        "\x00\x00\x0b\x00\x00\x00\x0a\x00\x00\x00\x09\x00\x00\x00\xff\xff\xff"
        "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff"
        "\x0d\x00\x0b\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x08"
        "\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff"
        "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x09\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x45\x00\x00\x00"
        "\x06\x00\x00\x00\x01\x00\x00\x00\x0b\x00\x00\x00\x0b\x00\x00\x00\x0e"
        "\x00\x00\x00\x40\x00\x00\x00\x40\x00\x00\x00\xfc\x0f\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00",
        192));
    NONFAILING(*(uint64_t*)0x200002c8 = 0xc0);
    NONFAILING(*(uint64_t*)0x200002d0 = 0x200000);
    NONFAILING(*(uint64_t*)0x200002d8 = 0x20010900);
    NONFAILING(memcpy(
        (void*)0x20010900,
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x7d\x96\x92\x6f\x0b\x00"
        "\x03\x00\x00\x00\x00\x03\x00\x00\x00\x0c\x26\x00\x00\x04\x00\x00\x00"
        "\x00\x04\x00\x00\x00\x01\x24\x00\x00\x05\x00\x00\x00\x00\x05\x00\x00"
        "\x00\x04\x26\x00\x00\x06\x00\x00\x00\x00\x06\x00\x00\x00\x03\x24\x00"
        "\x00\x07\x00\x00\x00\x00\x07\x00\x00\x00\x04\x24\x00\x00\x08\x00\x00"
        "\x00\x00\x08\x00\x00\x00\x05\x24\x00\x00\x09\x00\x00\x00\x00\x09\x00"
        "\x00\x00\x06\x24\x00\x00\x0a\x00\x00\x00\x00\x0a\x00\x00\x00\x07\x24"
        "\x00\x00\x0b\x00\x00\x00\x00\x0b\x00\x00\x00\x0b\x26\x00\x00\x0c\x00"
        "\x00\x00\x00\x0c\x00\x00\x00\x08\x24\x00\x00\x0d\x00\x00\x00\x00\x0d"
        "\x00\x00\x00\x0a\x24\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00",
        192));
    NONFAILING(*(uint64_t*)0x200002e0 = 0xc0);
    NONFAILING(*(uint64_t*)0x200002e8 = 0x200fe0);
    NONFAILING(*(uint64_t*)0x200002f0 = 0x20010a00);
    NONFAILING(*(uint64_t*)0x200002f8 = 0);
    NONFAILING(*(uint64_t*)0x20000300 = 0x2011e0);
    NONFAILING(*(uint64_t*)0x20000308 = 0x20010b00);
    NONFAILING(*(uint64_t*)0x20000310 = 0);
    NONFAILING(*(uint64_t*)0x20000318 = 0x201240);
    NONFAILING(*(uint64_t*)0x20000320 = 0x20010c00);
    NONFAILING(*(uint64_t*)0x20000328 = 0);
    NONFAILING(*(uint64_t*)0x20000330 = 0x201280);
    NONFAILING(*(uint64_t*)0x20000338 = 0x20010d00);
    NONFAILING(*(uint64_t*)0x20000340 = 0);
    NONFAILING(*(uint64_t*)0x20000348 = 0x2012e0);
    NONFAILING(*(uint64_t*)0x20000350 = 0x20010e00);
    NONFAILING(*(uint64_t*)0x20000358 = 0);
    NONFAILING(*(uint64_t*)0x20000360 = 0x201320);
    NONFAILING(*(uint64_t*)0x20000368 = 0x20010f00);
    NONFAILING(*(uint64_t*)0x20000370 = 0);
    NONFAILING(*(uint64_t*)0x20000378 = 5);
    NONFAILING(*(uint64_t*)0x20000380 = 0x20011000);
    NONFAILING(*(uint64_t*)0x20000388 = 0);
    NONFAILING(*(uint64_t*)0x20000390 = 0x2013e0);
    NONFAILING(*(uint64_t*)0x20000398 = 0x20011100);
    NONFAILING(*(uint64_t*)0x200003a0 = 0);
    NONFAILING(*(uint64_t*)0x200003a8 = 0x202000);
    NONFAILING(*(uint64_t*)0x200003b0 = 0x20011200);
    NONFAILING(*(uint64_t*)0x200003b8 = 0);
    NONFAILING(*(uint64_t*)0x200003c0 = 0x202e00);
    NONFAILING(*(uint64_t*)0x200003c8 = 0x20011300);
    NONFAILING(*(uint64_t*)0x200003d0 = 0);
    NONFAILING(*(uint64_t*)0x200003d8 = 0x202fe0);
    NONFAILING(*(uint64_t*)0x200003e0 = 0x20011400);
    NONFAILING(*(uint64_t*)0x200003e8 = 0);
    NONFAILING(*(uint64_t*)0x200003f0 = 0x203fe0);
    NONFAILING(*(uint64_t*)0x200003f8 = 0x20011500);
    NONFAILING(memcpy(
        (void*)0x20011500,
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x0d\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x0c\x00\x00"
        "\x00\x00\x00\x00\x00\x0a\x00\x00\x00\x0b\x00\x00\x00\x06\x00\x00\x00"
        "\x0b\x00\x00\x00\x0a\x00\x00\x00\x09\x00\x00\x00\xff\xff\xff\xff\xff"
        "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x0d\x00"
        "\x0b\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x08\x00\x00"
        "\x00\x01\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff"
        "\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x09\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x45\x00\x00\x00\x06\x00"
        "\x00\x00\x01\x00\x00\x00\x0b\x00\x00\x00\x0b\x00\x00\x00\x0e\x00\x00"
        "\x00\x40\x00\x00\x00\x40\x00\x00\x00\xfc\x0f\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00",
        224));
    NONFAILING(*(uint64_t*)0x20000400 = 0xe0);
    NONFAILING(*(uint64_t*)0x20000408 = 0x204fe0);
    NONFAILING(*(uint64_t*)0x20000410 = 0x20011600);
    NONFAILING(memcpy(
        (void*)0x20011600,
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x7d\x96\x92\x6f",
        32));
    NONFAILING(*(uint64_t*)0x20000418 = 0x20);
    NONFAILING(*(uint64_t*)0x20000420 = 0x205fe0);
    NONFAILING(*(uint64_t*)0x20000428 = 0x20011700);
    NONFAILING(*(uint64_t*)0x20000430 = 0);
    NONFAILING(*(uint64_t*)0x20000438 = 0x400000);
    NONFAILING(*(uint64_t*)0x20000440 = 0x20011800);
    NONFAILING(*(uint64_t*)0x20000448 = 0);
    NONFAILING(*(uint64_t*)0x20000450 = 0x400fe0);
    NONFAILING(*(uint64_t*)0x20000458 = 0x20011900);
    NONFAILING(*(uint64_t*)0x20000460 = 0);
    NONFAILING(*(uint64_t*)0x20000468 = 0x4011e0);
    NONFAILING(*(uint64_t*)0x20000470 = 0x20011a00);
    NONFAILING(*(uint64_t*)0x20000478 = 0);
    NONFAILING(*(uint64_t*)0x20000480 = 0x401240);
    NONFAILING(*(uint64_t*)0x20000488 = 0x20011b00);
    NONFAILING(*(uint64_t*)0x20000490 = 0);
    NONFAILING(*(uint64_t*)0x20000498 = 0x401280);
    NONFAILING(*(uint64_t*)0x200004a0 = 0x20011c00);
    NONFAILING(*(uint64_t*)0x200004a8 = 0);
    NONFAILING(*(uint64_t*)0x200004b0 = 0x4012e0);
    NONFAILING(*(uint64_t*)0x200004b8 = 0x20011d00);
    NONFAILING(*(uint64_t*)0x200004c0 = 0);
    NONFAILING(*(uint64_t*)0x200004c8 = 0x401320);
    NONFAILING(*(uint64_t*)0x200004d0 = 0x20011e00);
    NONFAILING(*(uint64_t*)0x200004d8 = 0);
    NONFAILING(*(uint64_t*)0x200004e0 = 0x401380);
    NONFAILING(*(uint64_t*)0x200004e8 = 0x20011f00);
    NONFAILING(*(uint64_t*)0x200004f0 = 0);
    NONFAILING(*(uint64_t*)0x200004f8 = 0x4013e0);
    NONFAILING(*(uint64_t*)0x20000500 = 0x20012000);
    NONFAILING(*(uint64_t*)0x20000508 = 0);
    NONFAILING(*(uint64_t*)0x20000510 = 0x402000);
    NONFAILING(*(uint64_t*)0x20000518 = 0x20012100);
    NONFAILING(*(uint64_t*)0x20000520 = 0);
    NONFAILING(*(uint64_t*)0x20000528 = 0x402e00);
    NONFAILING(*(uint64_t*)0x20000530 = 0x20012200);
    NONFAILING(*(uint64_t*)0x20000538 = 0);
    NONFAILING(*(uint64_t*)0x20000540 = 0x402fe0);
    NONFAILING(*(uint64_t*)0x20000548 = 0x20012300);
    NONFAILING(*(uint64_t*)0x20000550 = 0);
    NONFAILING(*(uint64_t*)0x20000558 = 0x403fe0);
    NONFAILING(*(uint64_t*)0x20000560 = 0x20012400);
    NONFAILING(*(uint64_t*)0x20000568 = 0);
    NONFAILING(*(uint64_t*)0x20000570 = 0x404fe0);
    NONFAILING(*(uint64_t*)0x20000578 = 0x20012500);
    NONFAILING(*(uint64_t*)0x20000580 = 0);
    NONFAILING(*(uint64_t*)0x20000588 = 0x405fe0);
    NONFAILING(*(uint64_t*)0x20000590 = 0x20012600);
    NONFAILING(*(uint64_t*)0x20000598 = 0);
    NONFAILING(*(uint64_t*)0x200005a0 = 0xa00000);
    NONFAILING(*(uint64_t*)0x200005a8 = 0x20012700);
    NONFAILING(*(uint64_t*)0x200005b0 = 0);
    NONFAILING(*(uint64_t*)0x200005b8 = 0x2000000);
    NONFAILING(*(uint64_t*)0x200005c0 = 0x20012800);
    NONFAILING(*(uint64_t*)0x200005c8 = 0);
    NONFAILING(*(uint64_t*)0x200005d0 = 0x2000940);
    NONFAILING(*(uint64_t*)0x200005d8 = 0x20012900);
    NONFAILING(*(uint64_t*)0x200005e0 = 0);
    NONFAILING(*(uint64_t*)0x200005e8 = 0x2001000);
    NONFAILING(*(uint64_t*)0x200005f0 = 0x20012a00);
    NONFAILING(*(uint64_t*)0x200005f8 = 0);
    NONFAILING(*(uint64_t*)0x20000600 = 0x2001940);
    NONFAILING(*(uint64_t*)0x20000608 = 0x20012b00);
    NONFAILING(*(uint64_t*)0x20000610 = 0);
    NONFAILING(*(uint64_t*)0x20000618 = 0x2002000);
    NONFAILING(*(uint64_t*)0x20000620 = 0x20012c00);
    NONFAILING(*(uint64_t*)0x20000628 = 0);
    NONFAILING(*(uint64_t*)0x20000630 = 0x2002940);
    NONFAILING(*(uint64_t*)0x20000638 = 0x20012d00);
    NONFAILING(*(uint64_t*)0x20000640 = 0);
    NONFAILING(*(uint64_t*)0x20000648 = 0x2003000);
    NONFAILING(*(uint64_t*)0x20000650 = 0x20012e00);
    NONFAILING(*(uint64_t*)0x20000658 = 0);
    NONFAILING(*(uint64_t*)0x20000660 = 0x2003940);
    NONFAILING(*(uint64_t*)0x20000668 = 0x20012f00);
    NONFAILING(*(uint64_t*)0x20000670 = 0);
    NONFAILING(*(uint64_t*)0x20000678 = 0x2004000);
    NONFAILING(*(uint64_t*)0x20000680 = 0x20013000);
    NONFAILING(*(uint64_t*)0x20000688 = 0);
    NONFAILING(*(uint64_t*)0x20000690 = 0x2004940);
    NONFAILING(*(uint64_t*)0x20000698 = 0x20013100);
    NONFAILING(*(uint64_t*)0x200006a0 = 0);
    NONFAILING(*(uint64_t*)0x200006a8 = 0x2005000);
    NONFAILING(*(uint64_t*)0x200006b0 = 0x20013200);
    NONFAILING(*(uint64_t*)0x200006b8 = 0);
    NONFAILING(*(uint64_t*)0x200006c0 = 0x2005940);
    NONFAILING(*(uint64_t*)0x200006c8 = 0x20013300);
    NONFAILING(*(uint64_t*)0x200006d0 = 0);
    NONFAILING(*(uint64_t*)0x200006d8 = 0x2006000);
    NONFAILING(*(uint64_t*)0x200006e0 = 0x20013400);
    NONFAILING(*(uint64_t*)0x200006e8 = 0);
    NONFAILING(*(uint64_t*)0x200006f0 = 0x2006940);
    NONFAILING(*(uint64_t*)0x200006f8 = 0x20013500);
    NONFAILING(*(uint64_t*)0x20000700 = 0);
    NONFAILING(*(uint64_t*)0x20000708 = 0x2007000);
    NONFAILING(*(uint64_t*)0x20000710 = 0x20013600);
    NONFAILING(*(uint64_t*)0x20000718 = 0);
    NONFAILING(*(uint64_t*)0x20000720 = 0x2007940);
    NONFAILING(*(uint64_t*)0x20000728 = 0x20013700);
    NONFAILING(*(uint64_t*)0x20000730 = 0);
    NONFAILING(*(uint64_t*)0x20000738 = 0x2008000);
    NONFAILING(*(uint64_t*)0x20000740 = 0x20013800);
    NONFAILING(*(uint64_t*)0x20000748 = 0);
    NONFAILING(*(uint64_t*)0x20000750 = 0x2008940);
    NONFAILING(*(uint64_t*)0x20000758 = 0x20013900);
    NONFAILING(*(uint64_t*)0x20000760 = 0);
    NONFAILING(*(uint64_t*)0x20000768 = 0x2400000);
    NONFAILING(*(uint64_t*)0x20000770 = 0x20013a00);
    NONFAILING(*(uint64_t*)0x20000778 = 0);
    NONFAILING(*(uint64_t*)0x20000780 = 0x2400160);
    NONFAILING(*(uint64_t*)0x20000788 = 0x20013b00);
    NONFAILING(*(uint64_t*)0x20000790 = 0);
    NONFAILING(*(uint64_t*)0x20000798 = 0x2400fe0);
    NONFAILING(*(uint64_t*)0x200007a0 = 0x20013c00);
    NONFAILING(*(uint64_t*)0x200007a8 = 0);
    NONFAILING(*(uint64_t*)0x200007b0 = 0x2401160);
    NONFAILING(*(uint64_t*)0x200007b8 = 0x20013d00);
    NONFAILING(*(uint64_t*)0x200007c0 = 0);
    NONFAILING(*(uint64_t*)0x200007c8 = 0x2401fe0);
    NONFAILING(*(uint64_t*)0x200007d0 = 0x20013e00);
    NONFAILING(*(uint64_t*)0x200007d8 = 0);
    NONFAILING(*(uint64_t*)0x200007e0 = 0x2402160);
    NONFAILING(*(uint64_t*)0x200007e8 = 0x20013f00);
    NONFAILING(*(uint64_t*)0x200007f0 = 0);
    NONFAILING(*(uint64_t*)0x200007f8 = 0x2402fe0);
    NONFAILING(*(uint64_t*)0x20000800 = 0x20014000);
    NONFAILING(*(uint64_t*)0x20000808 = 0);
    NONFAILING(*(uint64_t*)0x20000810 = 0x2403160);
    NONFAILING(*(uint64_t*)0x20000818 = 0x20014100);
    NONFAILING(*(uint64_t*)0x20000820 = 0);
    NONFAILING(*(uint64_t*)0x20000828 = 0x2403fe0);
    NONFAILING(*(uint64_t*)0x20000830 = 0x20014200);
    NONFAILING(*(uint64_t*)0x20000838 = 0);
    NONFAILING(*(uint64_t*)0x20000840 = 0x2404fe0);
    NONFAILING(*(uint64_t*)0x20000848 = 0x20014300);
    NONFAILING(*(uint64_t*)0x20000850 = 0);
    NONFAILING(*(uint64_t*)0x20000858 = 0x2405fe0);
    NONFAILING(*(uint64_t*)0x20000860 = 0x20014400);
    NONFAILING(*(uint64_t*)0x20000868 = 0);
    NONFAILING(*(uint64_t*)0x20000870 = 0x2406fe0);
    NONFAILING(*(uint64_t*)0x20000878 = 0x20014500);
    NONFAILING(*(uint64_t*)0x20000880 = 0);
    NONFAILING(*(uint64_t*)0x20000888 = 0x2407fe0);
    NONFAILING(*(uint64_t*)0x20000890 = 0x20014600);
    NONFAILING(*(uint64_t*)0x20000898 = 0);
    NONFAILING(*(uint64_t*)0x200008a0 = 0x2408fe0);
    NONFAILING(*(uint64_t*)0x200008a8 = 0x20014700);
    NONFAILING(*(uint64_t*)0x200008b0 = 0);
    NONFAILING(*(uint64_t*)0x200008b8 = 0x2409160);
    NONFAILING(*(uint64_t*)0x200008c0 = 0x20014800);
    NONFAILING(*(uint64_t*)0x200008c8 = 0);
    NONFAILING(*(uint64_t*)0x200008d0 = 0x2409fe0);
    NONFAILING(*(uint64_t*)0x200008d8 = 0x20014900);
    NONFAILING(*(uint64_t*)0x200008e0 = 0);
    NONFAILING(*(uint64_t*)0x200008e8 = 0x240a160);
    NONFAILING(*(uint64_t*)0x200008f0 = 0x20014a00);
    NONFAILING(*(uint64_t*)0x200008f8 = 0);
    NONFAILING(*(uint64_t*)0x20000900 = 0x240afe0);
    NONFAILING(*(uint64_t*)0x20000908 = 0x20014b00);
    NONFAILING(*(uint64_t*)0x20000910 = 0);
    NONFAILING(*(uint64_t*)0x20000918 = 0x2600000);
    NONFAILING(*(uint64_t*)0x20000920 = 0x20014c00);
    NONFAILING(*(uint64_t*)0x20000928 = 0);
    NONFAILING(*(uint64_t*)0x20000930 = 0x2600160);
    NONFAILING(*(uint64_t*)0x20000938 = 0x20014d00);
    NONFAILING(*(uint64_t*)0x20000940 = 0);
    NONFAILING(*(uint64_t*)0x20000948 = 0x2600fe0);
    NONFAILING(*(uint64_t*)0x20000950 = 0x20014e00);
    NONFAILING(*(uint64_t*)0x20000958 = 0);
    NONFAILING(*(uint64_t*)0x20000960 = 0x2601160);
    NONFAILING(*(uint64_t*)0x20000968 = 0x20014f00);
    NONFAILING(*(uint64_t*)0x20000970 = 0);
    NONFAILING(*(uint64_t*)0x20000978 = 0x2601fe0);
    NONFAILING(*(uint64_t*)0x20000980 = 0x20015000);
    NONFAILING(*(uint64_t*)0x20000988 = 0);
    NONFAILING(*(uint64_t*)0x20000990 = 0x2602160);
    NONFAILING(*(uint64_t*)0x20000998 = 0x20015100);
    NONFAILING(*(uint64_t*)0x200009a0 = 0);
    NONFAILING(*(uint64_t*)0x200009a8 = 0x2602fe0);
    NONFAILING(*(uint64_t*)0x200009b0 = 0x20015200);
    NONFAILING(*(uint64_t*)0x200009b8 = 0);
    NONFAILING(*(uint64_t*)0x200009c0 = 0x2603160);
    NONFAILING(*(uint64_t*)0x200009c8 = 0x20015300);
    NONFAILING(*(uint64_t*)0x200009d0 = 0);
    NONFAILING(*(uint64_t*)0x200009d8 = 0x2603940);
    NONFAILING(*(uint64_t*)0x200009e0 = 0x20015400);
    NONFAILING(*(uint64_t*)0x200009e8 = 0);
    NONFAILING(*(uint64_t*)0x200009f0 = 0x2603fe0);
    NONFAILING(*(uint64_t*)0x200009f8 = 0x20015500);
    NONFAILING(*(uint64_t*)0x20000a00 = 0);
    NONFAILING(*(uint64_t*)0x20000a08 = 0x2604160);
    NONFAILING(*(uint64_t*)0x20000a10 = 0x20015600);
    NONFAILING(*(uint64_t*)0x20000a18 = 0);
    NONFAILING(*(uint64_t*)0x20000a20 = 0x2604940);
    NONFAILING(*(uint64_t*)0x20000a28 = 0x20015700);
    NONFAILING(*(uint64_t*)0x20000a30 = 0);
    NONFAILING(*(uint64_t*)0x20000a38 = 0x2604fe0);
    NONFAILING(*(uint64_t*)0x20000a40 = 0x20015800);
    NONFAILING(*(uint64_t*)0x20000a48 = 0);
    NONFAILING(*(uint64_t*)0x20000a50 = 0x2605160);
    NONFAILING(*(uint64_t*)0x20000a58 = 0x20015900);
    NONFAILING(*(uint64_t*)0x20000a60 = 0);
    NONFAILING(*(uint64_t*)0x20000a68 = 0x2605fe0);
    NONFAILING(*(uint64_t*)0x20000a70 = 0x20015a00);
    NONFAILING(*(uint64_t*)0x20000a78 = 0);
    NONFAILING(*(uint64_t*)0x20000a80 = 0x2606160);
    NONFAILING(*(uint64_t*)0x20000a88 = 0x20015b00);
    NONFAILING(*(uint64_t*)0x20000a90 = 0);
    NONFAILING(*(uint64_t*)0x20000a98 = 0x2606fe0);
    NONFAILING(*(uint64_t*)0x20000aa0 = 0x20015c00);
    NONFAILING(*(uint64_t*)0x20000aa8 = 0);
    NONFAILING(*(uint64_t*)0x20000ab0 = 0x2607160);
    NONFAILING(*(uint64_t*)0x20000ab8 = 0x20015d00);
    NONFAILING(*(uint64_t*)0x20000ac0 = 0);
    NONFAILING(*(uint64_t*)0x20000ac8 = 0x2607fe0);
    NONFAILING(*(uint64_t*)0x20000ad0 = 0x20015e00);
    NONFAILING(*(uint64_t*)0x20000ad8 = 0);
    NONFAILING(*(uint64_t*)0x20000ae0 = 0x2608160);
    NONFAILING(*(uint64_t*)0x20000ae8 = 0x20015f00);
    NONFAILING(*(uint64_t*)0x20000af0 = 0);
    NONFAILING(*(uint64_t*)0x20000af8 = 0x2608fe0);
    NONFAILING(*(uint64_t*)0x20000b00 = 0x20016000);
    NONFAILING(*(uint64_t*)0x20000b08 = 0);
    NONFAILING(*(uint64_t*)0x20000b10 = 0x2609160);
    NONFAILING(*(uint64_t*)0x20000b18 = 0x20016100);
    NONFAILING(*(uint64_t*)0x20000b20 = 0);
    NONFAILING(*(uint64_t*)0x20000b28 = 0x2609fe0);
    NONFAILING(*(uint64_t*)0x20000b30 = 0x20016200);
    NONFAILING(*(uint64_t*)0x20000b38 = 0);
    NONFAILING(*(uint64_t*)0x20000b40 = 0x260a160);
    NONFAILING(*(uint64_t*)0x20000b48 = 0x20016300);
    NONFAILING(*(uint64_t*)0x20000b50 = 0);
    NONFAILING(*(uint64_t*)0x20000b58 = 0x260a940);
    NONFAILING(*(uint64_t*)0x20000b60 = 0x20016400);
    NONFAILING(*(uint64_t*)0x20000b68 = 0);
    NONFAILING(*(uint64_t*)0x20000b70 = 0x260afe0);
    NONFAILING(*(uint64_t*)0x20000b78 = 0x20016500);
    NONFAILING(*(uint64_t*)0x20000b80 = 0);
    NONFAILING(*(uint64_t*)0x20000b88 = 0x260b160);
    NONFAILING(*(uint64_t*)0x20000b90 = 0x20016600);
    NONFAILING(*(uint64_t*)0x20000b98 = 0);
    NONFAILING(*(uint64_t*)0x20000ba0 = 0x260b940);
    NONFAILING(*(uint64_t*)0x20000ba8 = 0x20016700);
    NONFAILING(memcpy(
        (void*)0x20016700,
        "\x00\x00\x00\x00\x00\x00\x00\x00\x0b\x00\x00\x00\x0b\x00\x00\x00\x00"
        "\x00\x00\x00\x09\x00\x00\x00\x29\xdb\xda\x7f\x0c\x26\x00\x00\xed\x41"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x04\x00\x00\x00\x00\x10\x00"
        "\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x78\xa0\xd8\x5a"
        "\x00\x00\x00\x00\x78\xa0\xd8\x5a\x00\x00\x00\x00\x78\xa0\xd8\x5a\x00"
        "\x00\x00\x00\x69\x6c\xcd\x30\x9b\x56\x00\x37\x9b\x56\x00\x37\x00\x00"
        "\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00",
        128));
    NONFAILING(*(uint64_t*)0x20000bb0 = 0x80);
    NONFAILING(*(uint64_t*)0x20000bb8 = 0x260bfe0);
    NONFAILING(*(uint64_t*)0x20000bc0 = 0x20016800);
    NONFAILING(*(uint64_t*)0x20000bc8 = 0);
    NONFAILING(*(uint64_t*)0x20000bd0 = 0x260c160);
    NONFAILING(*(uint64_t*)0x20000bd8 = 0x20016900);
    NONFAILING(memcpy(
        (void*)0x20016900,
        "\x00\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00\x03\x00\x00\x00\x00"
        "\x00\x00\x00\x0a\x00\x00\x00\x5a\xab\xd9\x50\x0d\x26\x00\x00",
        32));
    NONFAILING(*(uint64_t*)0x20000be0 = 0x20);
    NONFAILING(*(uint64_t*)0x20000be8 = 0x260cfe0);
    NONFAILING(*(uint8_t*)0x20016a00 = 0);
    syz_mount_image(0x20000000, 0x20000100, 0x2800000, 0x6a, 0x20000200, 0,
                    0x20016a00);
    break;
  }
}

void execute_one()
{
  execute(2);
  collide = 1;
  execute(2);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  install_segv_handler();
  char* cwd = get_current_dir_name();
  for (;;) {
    if (chdir(cwd))
      fail("failed to chdir");
    use_temporary_dir();
    int pid = do_sandbox_none();
    int status = 0;
    while (waitpid(pid, &status, __WALL) != pid) {
    }
  }
}