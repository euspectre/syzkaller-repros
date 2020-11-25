// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <endian.h>
#include <fcntl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>

#include <linux/genetlink.h>
#include <linux/if_addr.h>
#include <linux/if_link.h>
#include <linux/in6.h>
#include <linux/neighbour.h>
#include <linux/net.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/veth.h>

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

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  intptr_t res = 0;
  res = syscall(__NR_socket, 2ul, 2ul, 0x88ul);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000880,
         "filter\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000",
         32);
  *(uint32_t*)0x200008a0 = 4;
  *(uint32_t*)0x200008a4 = 4;
  *(uint32_t*)0x200008a8 = 0x3c8;
  *(uint32_t*)0x200008ac = 0;
  *(uint32_t*)0x200008b0 = 0;
  *(uint32_t*)0x200008b4 = 0;
  *(uint32_t*)0x200008b8 = 0x2e0;
  *(uint32_t*)0x200008bc = 0x2e0;
  *(uint32_t*)0x200008c0 = 0x2e0;
  *(uint32_t*)0x200008c4 = 4;
  *(uint64_t*)0x200008c8 = 0;
  *(uint8_t*)0x200008d0 = 0;
  *(uint8_t*)0x200008d1 = 0;
  *(uint8_t*)0x200008d2 = 0;
  *(uint8_t*)0x200008d3 = 0;
  *(uint8_t*)0x200008d4 = 0;
  *(uint8_t*)0x200008d5 = 0;
  *(uint8_t*)0x200008d6 = 0;
  *(uint8_t*)0x200008d7 = 0;
  *(uint8_t*)0x200008d8 = 0;
  *(uint8_t*)0x200008d9 = 0;
  *(uint8_t*)0x200008da = 0;
  *(uint8_t*)0x200008db = 0;
  *(uint8_t*)0x200008dc = 0;
  *(uint8_t*)0x200008dd = 0;
  *(uint8_t*)0x200008de = 0;
  *(uint8_t*)0x200008df = 0;
  *(uint8_t*)0x200008e0 = 0;
  *(uint8_t*)0x200008e1 = 0;
  *(uint8_t*)0x200008e2 = 0;
  *(uint8_t*)0x200008e3 = 0;
  *(uint8_t*)0x200008e4 = 0;
  *(uint8_t*)0x200008e5 = 0;
  *(uint8_t*)0x200008e6 = 0;
  *(uint8_t*)0x200008e7 = 0;
  *(uint8_t*)0x200008e8 = 0;
  *(uint8_t*)0x200008e9 = 0;
  *(uint8_t*)0x200008ea = 0;
  *(uint8_t*)0x200008eb = 0;
  *(uint8_t*)0x200008ec = 0;
  *(uint8_t*)0x200008ed = 0;
  *(uint8_t*)0x200008ee = 0;
  *(uint8_t*)0x200008ef = 0;
  *(uint8_t*)0x200008f0 = 0;
  *(uint8_t*)0x200008f1 = 0;
  *(uint8_t*)0x200008f2 = 0;
  *(uint8_t*)0x200008f3 = 0;
  *(uint8_t*)0x200008f4 = 0;
  *(uint8_t*)0x200008f5 = 0;
  *(uint8_t*)0x200008f6 = 0;
  *(uint8_t*)0x200008f7 = 0;
  *(uint8_t*)0x200008f8 = 0;
  *(uint8_t*)0x200008f9 = 0;
  *(uint8_t*)0x200008fa = 0;
  *(uint8_t*)0x200008fb = 0;
  *(uint8_t*)0x200008fc = 0;
  *(uint8_t*)0x200008fd = 0;
  *(uint8_t*)0x200008fe = 0;
  *(uint8_t*)0x200008ff = 0;
  *(uint8_t*)0x20000900 = 0;
  *(uint8_t*)0x20000901 = 0;
  *(uint8_t*)0x20000902 = 0;
  *(uint8_t*)0x20000903 = 0;
  *(uint8_t*)0x20000904 = 0;
  *(uint8_t*)0x20000905 = 0;
  *(uint8_t*)0x20000906 = 0;
  *(uint8_t*)0x20000907 = 0;
  *(uint8_t*)0x20000908 = 0;
  *(uint8_t*)0x20000909 = 0;
  *(uint8_t*)0x2000090a = 0;
  *(uint8_t*)0x2000090b = 0;
  *(uint8_t*)0x2000090c = 0;
  *(uint8_t*)0x2000090d = 0;
  *(uint8_t*)0x2000090e = 0;
  *(uint8_t*)0x2000090f = 0;
  *(uint8_t*)0x20000910 = 0;
  *(uint8_t*)0x20000911 = 0;
  *(uint8_t*)0x20000912 = 0;
  *(uint8_t*)0x20000913 = 0;
  *(uint8_t*)0x20000914 = 0;
  *(uint8_t*)0x20000915 = 0;
  *(uint8_t*)0x20000916 = 0;
  *(uint8_t*)0x20000917 = 0;
  *(uint8_t*)0x20000918 = 0;
  *(uint8_t*)0x20000919 = 0;
  *(uint8_t*)0x2000091a = 0;
  *(uint8_t*)0x2000091b = 0;
  *(uint8_t*)0x2000091c = 0;
  *(uint8_t*)0x2000091d = 0;
  *(uint8_t*)0x2000091e = 0;
  *(uint8_t*)0x2000091f = 0;
  *(uint8_t*)0x20000920 = 0;
  *(uint8_t*)0x20000921 = 0;
  *(uint8_t*)0x20000922 = 0;
  *(uint8_t*)0x20000923 = 0;
  *(uint8_t*)0x20000924 = 0;
  *(uint8_t*)0x20000925 = 0;
  *(uint8_t*)0x20000926 = 0;
  *(uint8_t*)0x20000927 = 0;
  *(uint8_t*)0x20000928 = 0;
  *(uint8_t*)0x20000929 = 0;
  *(uint8_t*)0x2000092a = 0;
  *(uint8_t*)0x2000092b = 0;
  *(uint8_t*)0x2000092c = 0;
  *(uint8_t*)0x2000092d = 0;
  *(uint8_t*)0x2000092e = 0;
  *(uint8_t*)0x2000092f = 0;
  *(uint8_t*)0x20000930 = 0;
  *(uint8_t*)0x20000931 = 0;
  *(uint8_t*)0x20000932 = 0;
  *(uint8_t*)0x20000933 = 0;
  *(uint8_t*)0x20000934 = 0;
  *(uint8_t*)0x20000935 = 0;
  *(uint8_t*)0x20000936 = 0;
  *(uint8_t*)0x20000937 = 0;
  *(uint8_t*)0x20000938 = 0;
  *(uint8_t*)0x20000939 = 0;
  *(uint8_t*)0x2000093a = 0;
  *(uint8_t*)0x2000093b = 0;
  *(uint8_t*)0x2000093c = 0;
  *(uint8_t*)0x2000093d = 0;
  *(uint8_t*)0x2000093e = 0;
  *(uint8_t*)0x2000093f = 0;
  *(uint8_t*)0x20000940 = 0;
  *(uint8_t*)0x20000941 = 0;
  *(uint8_t*)0x20000942 = 0;
  *(uint8_t*)0x20000943 = 0;
  *(uint8_t*)0x20000944 = 0;
  *(uint8_t*)0x20000945 = 0;
  *(uint8_t*)0x20000946 = 0;
  *(uint8_t*)0x20000947 = 0;
  *(uint8_t*)0x20000948 = 0;
  *(uint8_t*)0x20000949 = 0;
  *(uint8_t*)0x2000094a = 0;
  *(uint8_t*)0x2000094b = 0;
  *(uint8_t*)0x2000094c = 0;
  *(uint8_t*)0x2000094d = 0;
  *(uint8_t*)0x2000094e = 0;
  *(uint8_t*)0x2000094f = 0;
  *(uint8_t*)0x20000950 = 0;
  *(uint8_t*)0x20000951 = 0;
  *(uint8_t*)0x20000952 = 0;
  *(uint8_t*)0x20000953 = 0;
  *(uint8_t*)0x20000954 = 0;
  *(uint8_t*)0x20000955 = 0;
  *(uint8_t*)0x20000956 = 0;
  *(uint8_t*)0x20000957 = 0;
  *(uint8_t*)0x20000958 = 0;
  *(uint8_t*)0x20000959 = 0;
  *(uint8_t*)0x2000095a = 0;
  *(uint8_t*)0x2000095b = 0;
  *(uint8_t*)0x2000095c = 0;
  *(uint8_t*)0x2000095d = 0;
  *(uint8_t*)0x2000095e = 0;
  *(uint8_t*)0x2000095f = 0;
  *(uint8_t*)0x20000960 = 0;
  *(uint8_t*)0x20000961 = 0;
  *(uint8_t*)0x20000962 = 0;
  *(uint8_t*)0x20000963 = 0;
  *(uint8_t*)0x20000964 = 0;
  *(uint8_t*)0x20000965 = 0;
  *(uint8_t*)0x20000966 = 0;
  *(uint8_t*)0x20000967 = 0;
  *(uint8_t*)0x20000968 = 0;
  *(uint8_t*)0x20000969 = 0;
  *(uint8_t*)0x2000096a = 0;
  *(uint8_t*)0x2000096b = 0;
  *(uint8_t*)0x2000096c = 0;
  *(uint8_t*)0x2000096d = 0;
  *(uint8_t*)0x2000096e = 0;
  *(uint8_t*)0x2000096f = 0;
  *(uint8_t*)0x20000970 = 0;
  *(uint8_t*)0x20000971 = 0;
  *(uint8_t*)0x20000972 = 0;
  *(uint8_t*)0x20000973 = 0;
  *(uint16_t*)0x20000974 = 0xc0;
  *(uint16_t*)0x20000976 = 0xf0;
  *(uint32_t*)0x20000978 = 0;
  *(uint64_t*)0x20000980 = 0;
  *(uint64_t*)0x20000988 = 0;
  *(uint16_t*)0x20000990 = 0x30;
  memcpy((void*)0x20000992,
         "CONNMARK\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000",
         29);
  *(uint8_t*)0x200009af = 1;
  *(uint32_t*)0x200009b0 = 0x80000000;
  *(uint32_t*)0x200009b4 = 0x20;
  *(uint32_t*)0x200009b8 = 0x80004;
  *(uint8_t*)0x200009bc = 0;
  *(uint32_t*)0x200009c0 = htobe32(0x7f000001);
  *(uint8_t*)0x200009c4 = 0xac;
  *(uint8_t*)0x200009c5 = 0x1e;
  *(uint8_t*)0x200009c6 = 0;
  *(uint8_t*)0x200009c7 = 1;
  *(uint32_t*)0x200009c8 = htobe32(0);
  *(uint32_t*)0x200009cc = htobe32(0);
  *(uint8_t*)0x200009d0 = 0;
  *(uint8_t*)0x200009d1 = 7;
  *(uint8_t*)0x200009d2 = 0;
  *(uint8_t*)0x200009d3 = 0;
  *(uint8_t*)0x200009d4 = 0;
  *(uint8_t*)0x200009d5 = 0;
  *(uint8_t*)0x200009d6 = 0;
  *(uint8_t*)0x200009d7 = 0;
  *(uint8_t*)0x200009d8 = 0;
  *(uint8_t*)0x200009d9 = 0;
  *(uint8_t*)0x200009da = 0;
  *(uint8_t*)0x200009db = 0;
  *(uint8_t*)0x200009dc = 0;
  *(uint8_t*)0x200009dd = 0;
  *(uint8_t*)0x200009de = 0;
  *(uint8_t*)0x200009df = 0;
  *(uint8_t*)0x200009e0 = 0;
  *(uint8_t*)0x200009e1 = 0;
  *(uint8_t*)0x200009e2 = 0;
  *(uint8_t*)0x200009e3 = 0;
  *(uint8_t*)0x200009e4 = 0;
  *(uint8_t*)0x200009e5 = 0;
  *(uint8_t*)0x200009e6 = 0;
  *(uint8_t*)0x200009e7 = 0;
  *(uint8_t*)0x200009f2 = 0;
  *(uint8_t*)0x200009f3 = 0;
  *(uint8_t*)0x200009f4 = 0;
  *(uint8_t*)0x200009f5 = 0;
  *(uint8_t*)0x200009f6 = 0;
  *(uint8_t*)0x200009f7 = 0;
  *(uint8_t*)0x200009f8 = 0;
  *(uint8_t*)0x200009f9 = 0;
  *(uint8_t*)0x200009fa = 0;
  *(uint8_t*)0x200009fb = 0;
  *(uint8_t*)0x200009fc = 0;
  *(uint8_t*)0x200009fd = 0;
  *(uint8_t*)0x200009fe = 0;
  *(uint8_t*)0x200009ff = 0;
  *(uint8_t*)0x20000a00 = 0;
  *(uint8_t*)0x20000a01 = 0;
  *(uint8_t*)0x20000a02 = 0;
  *(uint8_t*)0x20000a03 = 0;
  *(uint8_t*)0x20000a04 = 0;
  *(uint8_t*)0x20000a05 = 0;
  *(uint8_t*)0x20000a06 = 0;
  *(uint8_t*)0x20000a07 = 0;
  *(uint16_t*)0x20000a12 = htobe16(0);
  *(uint16_t*)0x20000a14 = htobe16(0);
  *(uint16_t*)0x20000a16 = htobe16(0);
  *(uint16_t*)0x20000a18 = htobe16(0);
  *(uint16_t*)0x20000a1a = htobe16(0);
  *(uint16_t*)0x20000a1c = htobe16(0);
  memcpy((void*)0x20000a1e,
         "yam\371\377\377\377\000\000\000\000\000\000\000\000\000", 16);
  memcpy((void*)0x20000a2e,
         "lo\000\000\000\000\000\000\000\000\000\000\000\000\000\000", 16);
  *(uint8_t*)0x20000a3e = 0;
  *(uint8_t*)0x20000a4e = 0;
  *(uint8_t*)0x20000a5e = 0;
  *(uint16_t*)0x20000a60 = 0;
  *(uint16_t*)0x20000a64 = 0xc0;
  *(uint16_t*)0x20000a66 = 0xe8;
  *(uint32_t*)0x20000a68 = 0;
  *(uint64_t*)0x20000a70 = 0;
  *(uint64_t*)0x20000a78 = 0;
  *(uint16_t*)0x20000a80 = 0x28;
  memcpy((void*)0x20000a82,
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000",
         29);
  *(uint8_t*)0x20000a9f = 0;
  *(uint32_t*)0x20000aa0 = 0xfffffffe;
  *(uint32_t*)0x20000aa8 = htobe32(0xe0000002);
  *(uint32_t*)0x20000aac = htobe32(0x7f000001);
  *(uint32_t*)0x20000ab0 = htobe32(0);
  *(uint32_t*)0x20000ab4 = htobe32(0);
  *(uint8_t*)0x20000ab8 = 0;
  *(uint8_t*)0x20000ab9 = 0;
  *(uint8_t*)0x20000aba = 0;
  *(uint8_t*)0x20000abb = 0;
  *(uint8_t*)0x20000abc = 0;
  *(uint8_t*)0x20000abd = 0;
  *(uint8_t*)0x20000abe = 0;
  *(uint8_t*)0x20000abf = 0;
  *(uint8_t*)0x20000ac0 = 0;
  *(uint8_t*)0x20000ac1 = 0;
  *(uint8_t*)0x20000ac2 = 0;
  *(uint8_t*)0x20000ac3 = 0;
  *(uint8_t*)0x20000ac4 = 0;
  *(uint8_t*)0x20000ac5 = 0;
  *(uint8_t*)0x20000ac6 = 0;
  *(uint8_t*)0x20000ac7 = 0;
  *(uint8_t*)0x20000ac8 = 0;
  *(uint8_t*)0x20000ac9 = 0;
  *(uint8_t*)0x20000aca = 0;
  *(uint8_t*)0x20000acb = 0;
  *(uint8_t*)0x20000acc = 0;
  *(uint8_t*)0x20000acd = 0;
  *(uint8_t*)0x20000ace = 0;
  *(uint8_t*)0x20000acf = -1;
  *(uint8_t*)0x20000ada = 1;
  *(uint8_t*)0x20000adb = 0x80;
  *(uint8_t*)0x20000adc = 0xc2;
  *(uint8_t*)0x20000add = 0;
  *(uint8_t*)0x20000ade = 0;
  *(uint8_t*)0x20000adf = 0;
  *(uint8_t*)0x20000aea = 0;
  *(uint8_t*)0x20000aeb = 0;
  *(uint8_t*)0x20000aec = 0;
  *(uint8_t*)0x20000aed = 0;
  *(uint8_t*)0x20000aee = 0;
  *(uint8_t*)0x20000aef = 0;
  *(uint16_t*)0x20000afa = htobe16(0);
  *(uint16_t*)0x20000afc = htobe16(0);
  *(uint16_t*)0x20000afe = htobe16(0);
  *(uint16_t*)0x20000b00 = htobe16(0);
  *(uint16_t*)0x20000b02 = htobe16(0);
  *(uint16_t*)0x20000b04 = htobe16(0x94);
  memcpy((void*)0x20000b06, "team0\000\000\000\000\000\000\000\000\000\000\000",
         16);
  memcpy((void*)0x20000b16, "syzkaller0\000\000\000\000\000\000", 16);
  *(uint8_t*)0x20000b26 = 0;
  *(uint8_t*)0x20000b36 = 0;
  *(uint8_t*)0x20000b46 = 0;
  *(uint16_t*)0x20000b48 = 0;
  *(uint16_t*)0x20000b4c = 0xc0;
  *(uint16_t*)0x20000b4e = 0x108;
  *(uint32_t*)0x20000b50 = 0;
  *(uint64_t*)0x20000b58 = 0;
  *(uint64_t*)0x20000b60 = 0;
  *(uint16_t*)0x20000b68 = 0x48;
  memcpy((void*)0x20000b6a,
         "IDLETIMER\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000",
         29);
  *(uint8_t*)0x20000b87 = 0;
  *(uint32_t*)0x20000b88 = 5;
  memcpy((void*)0x20000b8c,
         "syz0\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000",
         28);
  *(uint64_t*)0x20000ba8 = 0;
  *(uint8_t*)0x20000bb0 = 0;
  *(uint8_t*)0x20000bb1 = 0;
  *(uint8_t*)0x20000bb2 = 0;
  *(uint8_t*)0x20000bb3 = 0;
  *(uint8_t*)0x20000bb4 = 0;
  *(uint8_t*)0x20000bb5 = 0;
  *(uint8_t*)0x20000bb6 = 0;
  *(uint8_t*)0x20000bb7 = 0;
  *(uint8_t*)0x20000bb8 = 0;
  *(uint8_t*)0x20000bb9 = 0;
  *(uint8_t*)0x20000bba = 0;
  *(uint8_t*)0x20000bbb = 0;
  *(uint8_t*)0x20000bbc = 0;
  *(uint8_t*)0x20000bbd = 0;
  *(uint8_t*)0x20000bbe = 0;
  *(uint8_t*)0x20000bbf = 0;
  *(uint8_t*)0x20000bc0 = 0;
  *(uint8_t*)0x20000bc1 = 0;
  *(uint8_t*)0x20000bc2 = 0;
  *(uint8_t*)0x20000bc3 = 0;
  *(uint8_t*)0x20000bc4 = 0;
  *(uint8_t*)0x20000bc5 = 0;
  *(uint8_t*)0x20000bc6 = 0;
  *(uint8_t*)0x20000bc7 = 0;
  *(uint8_t*)0x20000bc8 = 0;
  *(uint8_t*)0x20000bc9 = 0;
  *(uint8_t*)0x20000bca = 0;
  *(uint8_t*)0x20000bcb = 0;
  *(uint8_t*)0x20000bcc = 0;
  *(uint8_t*)0x20000bcd = 0;
  *(uint8_t*)0x20000bce = 0;
  *(uint8_t*)0x20000bcf = 0;
  *(uint8_t*)0x20000bd0 = 0;
  *(uint8_t*)0x20000bd1 = 0;
  *(uint8_t*)0x20000bd2 = 0;
  *(uint8_t*)0x20000bd3 = 0;
  *(uint8_t*)0x20000bd4 = 0;
  *(uint8_t*)0x20000bd5 = 0;
  *(uint8_t*)0x20000bd6 = 0;
  *(uint8_t*)0x20000bd7 = 0;
  *(uint8_t*)0x20000bd8 = 0;
  *(uint8_t*)0x20000bd9 = 0;
  *(uint8_t*)0x20000bda = 0;
  *(uint8_t*)0x20000bdb = 0;
  *(uint8_t*)0x20000bdc = 0;
  *(uint8_t*)0x20000bdd = 0;
  *(uint8_t*)0x20000bde = 0;
  *(uint8_t*)0x20000bdf = 0;
  *(uint8_t*)0x20000be0 = 0;
  *(uint8_t*)0x20000be1 = 0;
  *(uint8_t*)0x20000be2 = 0;
  *(uint8_t*)0x20000be3 = 0;
  *(uint8_t*)0x20000be4 = 0;
  *(uint8_t*)0x20000be5 = 0;
  *(uint8_t*)0x20000be6 = 0;
  *(uint8_t*)0x20000be7 = 0;
  *(uint8_t*)0x20000be8 = 0;
  *(uint8_t*)0x20000be9 = 0;
  *(uint8_t*)0x20000bea = 0;
  *(uint8_t*)0x20000beb = 0;
  *(uint8_t*)0x20000bec = 0;
  *(uint8_t*)0x20000bed = 0;
  *(uint8_t*)0x20000bee = 0;
  *(uint8_t*)0x20000bef = 0;
  *(uint8_t*)0x20000bf0 = 0;
  *(uint8_t*)0x20000bf1 = 0;
  *(uint8_t*)0x20000bf2 = 0;
  *(uint8_t*)0x20000bf3 = 0;
  *(uint8_t*)0x20000bf4 = 0;
  *(uint8_t*)0x20000bf5 = 0;
  *(uint8_t*)0x20000bf6 = 0;
  *(uint8_t*)0x20000bf7 = 0;
  *(uint8_t*)0x20000bf8 = 0;
  *(uint8_t*)0x20000bf9 = 0;
  *(uint8_t*)0x20000bfa = 0;
  *(uint8_t*)0x20000bfb = 0;
  *(uint8_t*)0x20000bfc = 0;
  *(uint8_t*)0x20000bfd = 0;
  *(uint8_t*)0x20000bfe = 0;
  *(uint8_t*)0x20000bff = 0;
  *(uint8_t*)0x20000c00 = 0;
  *(uint8_t*)0x20000c01 = 0;
  *(uint8_t*)0x20000c02 = 0;
  *(uint8_t*)0x20000c03 = 0;
  *(uint8_t*)0x20000c04 = 0;
  *(uint8_t*)0x20000c05 = 0;
  *(uint8_t*)0x20000c06 = 0;
  *(uint8_t*)0x20000c07 = 0;
  *(uint8_t*)0x20000c08 = 0;
  *(uint8_t*)0x20000c09 = 0;
  *(uint8_t*)0x20000c0a = 0;
  *(uint8_t*)0x20000c0b = 0;
  *(uint8_t*)0x20000c0c = 0;
  *(uint8_t*)0x20000c0d = 0;
  *(uint8_t*)0x20000c0e = 0;
  *(uint8_t*)0x20000c0f = 0;
  *(uint8_t*)0x20000c10 = 0;
  *(uint8_t*)0x20000c11 = 0;
  *(uint8_t*)0x20000c12 = 0;
  *(uint8_t*)0x20000c13 = 0;
  *(uint8_t*)0x20000c14 = 0;
  *(uint8_t*)0x20000c15 = 0;
  *(uint8_t*)0x20000c16 = 0;
  *(uint8_t*)0x20000c17 = 0;
  *(uint8_t*)0x20000c18 = 0;
  *(uint8_t*)0x20000c19 = 0;
  *(uint8_t*)0x20000c1a = 0;
  *(uint8_t*)0x20000c1b = 0;
  *(uint8_t*)0x20000c1c = 0;
  *(uint8_t*)0x20000c1d = 0;
  *(uint8_t*)0x20000c1e = 0;
  *(uint8_t*)0x20000c1f = 0;
  *(uint8_t*)0x20000c20 = 0;
  *(uint8_t*)0x20000c21 = 0;
  *(uint8_t*)0x20000c22 = 0;
  *(uint8_t*)0x20000c23 = 0;
  *(uint8_t*)0x20000c24 = 0;
  *(uint8_t*)0x20000c25 = 0;
  *(uint8_t*)0x20000c26 = 0;
  *(uint8_t*)0x20000c27 = 0;
  *(uint8_t*)0x20000c28 = 0;
  *(uint8_t*)0x20000c29 = 0;
  *(uint8_t*)0x20000c2a = 0;
  *(uint8_t*)0x20000c2b = 0;
  *(uint8_t*)0x20000c2c = 0;
  *(uint8_t*)0x20000c2d = 0;
  *(uint8_t*)0x20000c2e = 0;
  *(uint8_t*)0x20000c2f = 0;
  *(uint8_t*)0x20000c30 = 0;
  *(uint8_t*)0x20000c31 = 0;
  *(uint8_t*)0x20000c32 = 0;
  *(uint8_t*)0x20000c33 = 0;
  *(uint8_t*)0x20000c34 = 0;
  *(uint8_t*)0x20000c35 = 0;
  *(uint8_t*)0x20000c36 = 0;
  *(uint8_t*)0x20000c37 = 0;
  *(uint8_t*)0x20000c38 = 0;
  *(uint8_t*)0x20000c39 = 0;
  *(uint8_t*)0x20000c3a = 0;
  *(uint8_t*)0x20000c3b = 0;
  *(uint8_t*)0x20000c3c = 0;
  *(uint8_t*)0x20000c3d = 0;
  *(uint8_t*)0x20000c3e = 0;
  *(uint8_t*)0x20000c3f = 0;
  *(uint8_t*)0x20000c40 = 0;
  *(uint8_t*)0x20000c41 = 0;
  *(uint8_t*)0x20000c42 = 0;
  *(uint8_t*)0x20000c43 = 0;
  *(uint8_t*)0x20000c44 = 0;
  *(uint8_t*)0x20000c45 = 0;
  *(uint8_t*)0x20000c46 = 0;
  *(uint8_t*)0x20000c47 = 0;
  *(uint8_t*)0x20000c48 = 0;
  *(uint8_t*)0x20000c49 = 0;
  *(uint8_t*)0x20000c4a = 0;
  *(uint8_t*)0x20000c4b = 0;
  *(uint8_t*)0x20000c4c = 0;
  *(uint8_t*)0x20000c4d = 0;
  *(uint8_t*)0x20000c4e = 0;
  *(uint8_t*)0x20000c4f = 0;
  *(uint8_t*)0x20000c50 = 0;
  *(uint8_t*)0x20000c51 = 0;
  *(uint8_t*)0x20000c52 = 0;
  *(uint8_t*)0x20000c53 = 0;
  *(uint16_t*)0x20000c54 = 0xc0;
  *(uint16_t*)0x20000c56 = 0xe8;
  *(uint32_t*)0x20000c58 = 0;
  *(uint64_t*)0x20000c60 = 0;
  *(uint64_t*)0x20000c68 = 0;
  *(uint16_t*)0x20000c70 = 0x28;
  memcpy((void*)0x20000c72,
         "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
         "\000\000\000\000\000\000\000\000\000\000\000\000",
         29);
  *(uint8_t*)0x20000c8f = 0;
  *(uint32_t*)0x20000c90 = 0xfffffffe;
  syscall(__NR_setsockopt, r[0], 0xa02000000000000ul, 0x60ul, 0x20000880ul,
          0x418ul);
  return 0;
}
