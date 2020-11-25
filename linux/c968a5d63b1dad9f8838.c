// KASAN: use-after-free Write in tcindex_set_parms
// https://syzkaller.appspot.com/bug?id=c968a5d63b1dad9f8838
// status:6
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sched.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <linux/capability.h>
#include <linux/genetlink.h>
#include <linux/if_addr.h>
#include <linux/if_link.h>
#include <linux/in6.h>
#include <linux/neighbour.h>
#include <linux/net.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/veth.h>

static __thread int skip_segv;
static __thread jmp_buf segv_env;

static void segv_handler(int sig, siginfo_t* info, void* ctx)
{
  uintptr_t addr = (uintptr_t)info->si_addr;
  const uintptr_t prog_start = 1 << 20;
  const uintptr_t prog_end = 100 << 20;
  if (__atomic_load_n(&skip_segv, __ATOMIC_RELAXED) &&
      (addr < prog_start || addr > prog_end)) {
    _longjmp(segv_env, 1);
  }
  exit(sig);
}

static void install_segv_handler(void)
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

static bool write_file(const char* file, const char* what, ...)
{
  char buf[1024];
  va_list args;
  va_start(args, what);
  vsnprintf(buf, sizeof(buf), what, args);
  va_end(args);
  buf[sizeof(buf) - 1] = 0;
  int len = strlen(buf);
  int fd = open(file, O_WRONLY | O_CLOEXEC);
  if (fd == -1)
    return false;
  if (write(fd, buf, len) != len) {
    int err = errno;
    close(fd);
    errno = err;
    return false;
  }
  close(fd);
  return true;
}

struct nlmsg {
  char* pos;
  int nesting;
  struct nlattr* nested[8];
  char buf[1024];
};

static struct nlmsg nlmsg;

static void netlink_init(struct nlmsg* nlmsg, int typ, int flags,
                         const void* data, int size)
{
  memset(nlmsg, 0, sizeof(*nlmsg));
  struct nlmsghdr* hdr = (struct nlmsghdr*)nlmsg->buf;
  hdr->nlmsg_type = typ;
  hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | flags;
  memcpy(hdr + 1, data, size);
  nlmsg->pos = (char*)(hdr + 1) + NLMSG_ALIGN(size);
}

static void netlink_attr(struct nlmsg* nlmsg, int typ, const void* data,
                         int size)
{
  struct nlattr* attr = (struct nlattr*)nlmsg->pos;
  attr->nla_len = sizeof(*attr) + size;
  attr->nla_type = typ;
  memcpy(attr + 1, data, size);
  nlmsg->pos += NLMSG_ALIGN(attr->nla_len);
}

static int netlink_send_ext(struct nlmsg* nlmsg, int sock, uint16_t reply_type,
                            int* reply_len)
{
  if (nlmsg->pos > nlmsg->buf + sizeof(nlmsg->buf) || nlmsg->nesting)
    exit(1);
  struct nlmsghdr* hdr = (struct nlmsghdr*)nlmsg->buf;
  hdr->nlmsg_len = nlmsg->pos - nlmsg->buf;
  struct sockaddr_nl addr;
  memset(&addr, 0, sizeof(addr));
  addr.nl_family = AF_NETLINK;
  unsigned n = sendto(sock, nlmsg->buf, hdr->nlmsg_len, 0,
                      (struct sockaddr*)&addr, sizeof(addr));
  if (n != hdr->nlmsg_len)
    exit(1);
  n = recv(sock, nlmsg->buf, sizeof(nlmsg->buf), 0);
  if (hdr->nlmsg_type == NLMSG_DONE) {
    *reply_len = 0;
    return 0;
  }
  if (n < sizeof(struct nlmsghdr))
    exit(1);
  if (reply_len && hdr->nlmsg_type == reply_type) {
    *reply_len = n;
    return 0;
  }
  if (n < sizeof(struct nlmsghdr) + sizeof(struct nlmsgerr))
    exit(1);
  if (hdr->nlmsg_type != NLMSG_ERROR)
    exit(1);
  return -((struct nlmsgerr*)(hdr + 1))->error;
}

static int netlink_send(struct nlmsg* nlmsg, int sock)
{
  return netlink_send_ext(nlmsg, sock, 0, NULL);
}

static int netlink_next_msg(struct nlmsg* nlmsg, unsigned int offset,
                            unsigned int total_len)
{
  struct nlmsghdr* hdr = (struct nlmsghdr*)(nlmsg->buf + offset);
  if (offset == total_len || offset + hdr->nlmsg_len > total_len)
    return -1;
  return hdr->nlmsg_len;
}

static void netlink_device_change(struct nlmsg* nlmsg, int sock,
                                  const char* name, bool up, const char* master,
                                  const void* mac, int macsize,
                                  const char* new_name)
{
  struct ifinfomsg hdr;
  memset(&hdr, 0, sizeof(hdr));
  if (up)
    hdr.ifi_flags = hdr.ifi_change = IFF_UP;
  hdr.ifi_index = if_nametoindex(name);
  netlink_init(nlmsg, RTM_NEWLINK, 0, &hdr, sizeof(hdr));
  if (new_name)
    netlink_attr(nlmsg, IFLA_IFNAME, new_name, strlen(new_name));
  if (master) {
    int ifindex = if_nametoindex(master);
    netlink_attr(nlmsg, IFLA_MASTER, &ifindex, sizeof(ifindex));
  }
  if (macsize)
    netlink_attr(nlmsg, IFLA_ADDRESS, mac, macsize);
  int err = netlink_send(nlmsg, sock);
  (void)err;
}

const int kInitNetNsFd = 239;

#define DEVLINK_FAMILY_NAME "devlink"

#define DEVLINK_CMD_PORT_GET 5
#define DEVLINK_CMD_RELOAD 37
#define DEVLINK_ATTR_BUS_NAME 1
#define DEVLINK_ATTR_DEV_NAME 2
#define DEVLINK_ATTR_NETDEV_NAME 7
#define DEVLINK_ATTR_NETNS_FD 138

static int netlink_devlink_id_get(struct nlmsg* nlmsg, int sock)
{
  struct genlmsghdr genlhdr;
  struct nlattr* attr;
  int err, n;
  uint16_t id = 0;
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = CTRL_CMD_GETFAMILY;
  netlink_init(nlmsg, GENL_ID_CTRL, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(nlmsg, CTRL_ATTR_FAMILY_NAME, DEVLINK_FAMILY_NAME,
               strlen(DEVLINK_FAMILY_NAME) + 1);
  err = netlink_send_ext(nlmsg, sock, GENL_ID_CTRL, &n);
  if (err) {
    return -1;
  }
  attr = (struct nlattr*)(nlmsg->buf + NLMSG_HDRLEN +
                          NLMSG_ALIGN(sizeof(genlhdr)));
  for (; (char*)attr < nlmsg->buf + n;
       attr = (struct nlattr*)((char*)attr + NLMSG_ALIGN(attr->nla_len))) {
    if (attr->nla_type == CTRL_ATTR_FAMILY_ID) {
      id = *(uint16_t*)(attr + 1);
      break;
    }
  }
  if (!id) {
    return -1;
  }
  recv(sock, nlmsg->buf, sizeof(nlmsg->buf), 0); /* recv ack */
  return id;
}

static void netlink_devlink_netns_move(const char* bus_name,
                                       const char* dev_name, int netns_fd)
{
  struct genlmsghdr genlhdr;
  int sock;
  int id, err;
  sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
  if (sock == -1)
    exit(1);
  id = netlink_devlink_id_get(&nlmsg, sock);
  if (id == -1)
    goto error;
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = DEVLINK_CMD_RELOAD;
  netlink_init(&nlmsg, id, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(&nlmsg, DEVLINK_ATTR_BUS_NAME, bus_name, strlen(bus_name) + 1);
  netlink_attr(&nlmsg, DEVLINK_ATTR_DEV_NAME, dev_name, strlen(dev_name) + 1);
  netlink_attr(&nlmsg, DEVLINK_ATTR_NETNS_FD, &netns_fd, sizeof(netns_fd));
  err = netlink_send(&nlmsg, sock);
  if (err) {
  }
error:
  close(sock);
}

static struct nlmsg nlmsg2;

static void initialize_devlink_ports(const char* bus_name, const char* dev_name,
                                     const char* netdev_prefix)
{
  struct genlmsghdr genlhdr;
  int len, total_len, id, err, offset;
  uint16_t netdev_index;
  int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
  if (sock == -1)
    exit(1);
  int rtsock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if (rtsock == -1)
    exit(1);
  id = netlink_devlink_id_get(&nlmsg, sock);
  if (id == -1)
    goto error;
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = DEVLINK_CMD_PORT_GET;
  netlink_init(&nlmsg, id, NLM_F_DUMP, &genlhdr, sizeof(genlhdr));
  netlink_attr(&nlmsg, DEVLINK_ATTR_BUS_NAME, bus_name, strlen(bus_name) + 1);
  netlink_attr(&nlmsg, DEVLINK_ATTR_DEV_NAME, dev_name, strlen(dev_name) + 1);
  err = netlink_send_ext(&nlmsg, sock, id, &total_len);
  if (err) {
    goto error;
  }
  offset = 0;
  netdev_index = 0;
  while ((len = netlink_next_msg(&nlmsg, offset, total_len)) != -1) {
    struct nlattr* attr = (struct nlattr*)(nlmsg.buf + offset + NLMSG_HDRLEN +
                                           NLMSG_ALIGN(sizeof(genlhdr)));
    for (; (char*)attr < nlmsg.buf + offset + len;
         attr = (struct nlattr*)((char*)attr + NLMSG_ALIGN(attr->nla_len))) {
      if (attr->nla_type == DEVLINK_ATTR_NETDEV_NAME) {
        char* port_name;
        char netdev_name[IFNAMSIZ];
        port_name = (char*)(attr + 1);
        snprintf(netdev_name, sizeof(netdev_name), "%s%d", netdev_prefix,
                 netdev_index);
        netlink_device_change(&nlmsg2, rtsock, port_name, true, 0, 0, 0,
                              netdev_name);
        break;
      }
    }
    offset += len;
    netdev_index++;
  }
error:
  close(rtsock);
  close(sock);
}

static void initialize_devlink_pci(void)
{
  int netns = open("/proc/self/ns/net", O_RDONLY);
  if (netns == -1)
    exit(1);
  int ret = setns(kInitNetNsFd, 0);
  if (ret == -1)
    exit(1);
  netlink_devlink_netns_move("pci", "0000:00:10.0", netns);
  ret = setns(netns, 0);
  if (ret == -1)
    exit(1);
  close(netns);
  initialize_devlink_ports("pci", "0000:00:10.0", "netpci");
}

static void setup_common()
{
  if (mount(0, "/sys/fs/fuse/connections", "fusectl", 0, 0)) {
  }
}

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  setsid();
  int netns = open("/proc/self/ns/net", O_RDONLY);
  if (netns == -1)
    exit(1);
  if (dup2(netns, kInitNetNsFd) < 0)
    exit(1);
  close(netns);
  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = (200 << 20);
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 32 << 20;
  setrlimit(RLIMIT_MEMLOCK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 136 << 20;
  setrlimit(RLIMIT_FSIZE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_STACK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 256;
  setrlimit(RLIMIT_NOFILE, &rlim);
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
  typedef struct {
    const char* name;
    const char* value;
  } sysctl_t;
  static const sysctl_t sysctls[] = {
      {"/proc/sys/kernel/shmmax", "16777216"},
      {"/proc/sys/kernel/shmall", "536870912"},
      {"/proc/sys/kernel/shmmni", "1024"},
      {"/proc/sys/kernel/msgmax", "8192"},
      {"/proc/sys/kernel/msgmni", "1024"},
      {"/proc/sys/kernel/msgmnb", "1024"},
      {"/proc/sys/kernel/sem", "1024 1048576 500 1024"},
  };
  unsigned i;
  for (i = 0; i < sizeof(sysctls) / sizeof(sysctls[0]); i++)
    write_file(sysctls[i].name, sysctls[i].value);
}

static int wait_for_loop(int pid)
{
  if (pid < 0)
    exit(1);
  int status = 0;
  while (waitpid(-1, &status, __WALL) != pid) {
  }
  return WEXITSTATUS(status);
}

static void drop_caps(void)
{
  struct __user_cap_header_struct cap_hdr = {};
  struct __user_cap_data_struct cap_data[2] = {};
  cap_hdr.version = _LINUX_CAPABILITY_VERSION_3;
  cap_hdr.pid = getpid();
  if (syscall(SYS_capget, &cap_hdr, &cap_data))
    exit(1);
  const int drop = (1 << CAP_SYS_PTRACE) | (1 << CAP_SYS_NICE);
  cap_data[0].effective &= ~drop;
  cap_data[0].permitted &= ~drop;
  cap_data[0].inheritable &= ~drop;
  if (syscall(SYS_capset, &cap_hdr, &cap_data))
    exit(1);
}

static int do_sandbox_none(void)
{
  if (unshare(CLONE_NEWPID)) {
  }
  int pid = fork();
  if (pid != 0)
    return wait_for_loop(pid);
  setup_common();
  sandbox_common();
  drop_caps();
  if (unshare(CLONE_NEWNET)) {
  }
  initialize_devlink_pci();
  loop();
  exit(1);
}

uint64_t r[5] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff, 0x0};

void loop(void)
{
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0ul);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0ul);
  if (res != -1)
    r[1] = res;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0ul);
  if (res != -1)
    r[2] = res;
  res = syscall(__NR_socket, 0x10ul, 0x803ul, 0);
  if (res != -1)
    r[3] = res;
  NONFAILING(*(uint64_t*)0x20000280 = 0);
  NONFAILING(*(uint32_t*)0x20000288 = 0);
  NONFAILING(*(uint64_t*)0x20000290 = 0x20000180);
  NONFAILING(*(uint64_t*)0x20000180 = 0);
  NONFAILING(*(uint64_t*)0x20000188 = 0);
  NONFAILING(*(uint64_t*)0x20000298 = 1);
  NONFAILING(*(uint64_t*)0x200002a0 = 0);
  NONFAILING(*(uint64_t*)0x200002a8 = 0);
  NONFAILING(*(uint32_t*)0x200002b0 = 0);
  syscall(__NR_sendmsg, r[3], 0x20000280ul, 0ul);
  NONFAILING(*(uint32_t*)0x200002c0 = 0x14);
  res = syscall(__NR_getsockname, r[3], 0x20000100ul, 0x200002c0ul);
  if (res != -1)
    NONFAILING(r[4] = *(uint32_t*)0x20000104);
  NONFAILING(*(uint64_t*)0x20000240 = 0);
  NONFAILING(*(uint32_t*)0x20000248 = 0);
  NONFAILING(*(uint64_t*)0x20000250 = 0x20000140);
  NONFAILING(*(uint64_t*)0x20000140 = 0x200003c0);
  NONFAILING(memcpy((void*)0x200003c0, "\x38\x00\x00\x00\x24\x00\x07\x05\x00"
                                       "\x00\x00\x40\x07\xa2\xa3\x00\x05\x00"
                                       "\x00\x00",
                    20));
  NONFAILING(*(uint32_t*)0x200003d4 = r[4]);
  NONFAILING(memcpy((void*)0x200003d8, "\x00\x00\x00\x00\xff\xff\xff\xff\x00"
                                       "\x00\x00\x00\x09\x00\x01\x00\x68\x66"
                                       "\x73\x63\x00\x00\x00\x00\x08\x00\x02"
                                       "\x00\x00\x00\x00\x00",
                    32));
  NONFAILING(*(uint64_t*)0x20000148 = 0x38);
  NONFAILING(*(uint64_t*)0x20000258 = 1);
  NONFAILING(*(uint64_t*)0x20000260 = 0);
  NONFAILING(*(uint64_t*)0x20000268 = 0);
  NONFAILING(*(uint32_t*)0x20000270 = 0);
  syscall(__NR_sendmsg, r[2], 0x20000240ul, 0ul);
  NONFAILING(*(uint64_t*)0x200001c0 = 0);
  NONFAILING(*(uint32_t*)0x200001c8 = 0);
  NONFAILING(*(uint64_t*)0x200001d0 = 0x20000180);
  NONFAILING(*(uint64_t*)0x20000180 = 0x20000580);
  NONFAILING(*(uint32_t*)0x20000580 = 0x3c);
  NONFAILING(*(uint16_t*)0x20000584 = 0x2c);
  NONFAILING(*(uint16_t*)0x20000586 = 0xd27);
  NONFAILING(*(uint32_t*)0x20000588 = 0);
  NONFAILING(*(uint32_t*)0x2000058c = 0);
  NONFAILING(*(uint8_t*)0x20000590 = 0);
  NONFAILING(*(uint8_t*)0x20000591 = 0);
  NONFAILING(*(uint16_t*)0x20000592 = 0);
  NONFAILING(*(uint32_t*)0x20000594 = r[4]);
  NONFAILING(*(uint16_t*)0x20000598 = 0);
  NONFAILING(*(uint16_t*)0x2000059a = 0);
  NONFAILING(*(uint16_t*)0x2000059c = 0);
  NONFAILING(*(uint16_t*)0x2000059e = 0);
  NONFAILING(*(uint16_t*)0x200005a0 = 4);
  NONFAILING(*(uint16_t*)0x200005a2 = 0xfff1);
  NONFAILING(*(uint16_t*)0x200005a4 = 0xc);
  NONFAILING(*(uint16_t*)0x200005a6 = 1);
  NONFAILING(memcpy((void*)0x200005a8, "tcindex\000", 8));
  NONFAILING(*(uint16_t*)0x200005b0 = 0xc);
  NONFAILING(*(uint16_t*)0x200005b2 = 2);
  NONFAILING(*(uint16_t*)0x200005b4 = 6);
  NONFAILING(*(uint16_t*)0x200005b6 = 2);
  NONFAILING(*(uint16_t*)0x200005b8 = 0);
  NONFAILING(*(uint64_t*)0x20000188 = 0x3c);
  NONFAILING(*(uint64_t*)0x200001d8 = 1);
  NONFAILING(*(uint64_t*)0x200001e0 = 0);
  NONFAILING(*(uint64_t*)0x200001e8 = 0);
  NONFAILING(*(uint32_t*)0x200001f0 = 0);
  syscall(__NR_sendmsg, r[1], 0x200001c0ul, 0ul);
  NONFAILING(*(uint64_t*)0x200001c0 = 0);
  NONFAILING(*(uint32_t*)0x200001c8 = 0);
  NONFAILING(*(uint64_t*)0x200001d0 = 0x20000180);
  NONFAILING(*(uint64_t*)0x20000180 = 0x20000400);
  NONFAILING(*(uint32_t*)0x20000400 = 0x3c);
  NONFAILING(*(uint16_t*)0x20000404 = 0x2c);
  NONFAILING(*(uint16_t*)0x20000406 = 0xd27);
  NONFAILING(*(uint32_t*)0x20000408 = 0);
  NONFAILING(*(uint32_t*)0x2000040c = 0);
  NONFAILING(*(uint8_t*)0x20000410 = 0);
  NONFAILING(*(uint8_t*)0x20000411 = 0);
  NONFAILING(*(uint16_t*)0x20000412 = 0xf0);
  NONFAILING(*(uint32_t*)0x20000414 = r[4]);
  NONFAILING(*(uint16_t*)0x20000418 = 2);
  NONFAILING(*(uint16_t*)0x2000041a = 0);
  NONFAILING(*(uint16_t*)0x2000041c = 0);
  NONFAILING(*(uint16_t*)0x2000041e = 0);
  NONFAILING(*(uint16_t*)0x20000420 = 0);
  NONFAILING(*(uint16_t*)0x20000422 = 0xfff1);
  NONFAILING(*(uint16_t*)0x20000424 = 0xc);
  NONFAILING(*(uint16_t*)0x20000426 = 1);
  NONFAILING(memcpy((void*)0x20000428, "tcindex\000", 8));
  NONFAILING(*(uint16_t*)0x20000430 = 0xc);
  NONFAILING(*(uint16_t*)0x20000432 = 2);
  NONFAILING(*(uint16_t*)0x20000434 = 8);
  NONFAILING(*(uint16_t*)0x20000436 = 1);
  NONFAILING(*(uint32_t*)0x20000438 = 0);
  NONFAILING(*(uint64_t*)0x20000188 = 0x3c);
  NONFAILING(*(uint64_t*)0x200001d8 = 1);
  NONFAILING(*(uint64_t*)0x200001e0 = 0);
  NONFAILING(*(uint64_t*)0x200001e8 = 0);
  NONFAILING(*(uint32_t*)0x200001f0 = 0);
  syscall(__NR_sendmsg, r[0], 0x200001c0ul, 0ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  install_segv_handler();
  do_sandbox_none();
  return 0;
}