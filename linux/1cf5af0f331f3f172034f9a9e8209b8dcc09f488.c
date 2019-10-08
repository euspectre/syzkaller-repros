// KASAN: slab-out-of-bounds Write in tcp_v6_syn_recv_sock (2)
// https://syzkaller.appspot.com/bug?id=1cf5af0f331f3f172034f9a9e8209b8dcc09f488
// status:invalid
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <linux/futex.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

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

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

uint64_t r[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff};
unsigned long long procid;
void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    res = syscall(__NR_socketpair, 0, 0, 0, 0x20000140);
    if (res != -1)
      r[0] = *(uint32_t*)0x20000144;
    break;
  case 1:
    res = syscall(__NR_socket, 0xa, 1, 0);
    if (res != -1)
      r[1] = res;
    break;
  case 2:
    *(uint16_t*)0x20000080 = 0xa;
    *(uint16_t*)0x20000082 = htobe16(0x4e22);
    *(uint32_t*)0x20000084 = 0;
    *(uint8_t*)0x20000088 = 0;
    *(uint8_t*)0x20000089 = 0;
    *(uint8_t*)0x2000008a = 0;
    *(uint8_t*)0x2000008b = 0;
    *(uint8_t*)0x2000008c = 0;
    *(uint8_t*)0x2000008d = 0;
    *(uint8_t*)0x2000008e = 0;
    *(uint8_t*)0x2000008f = 0;
    *(uint8_t*)0x20000090 = 0;
    *(uint8_t*)0x20000091 = 0;
    *(uint8_t*)0x20000092 = 0;
    *(uint8_t*)0x20000093 = 0;
    *(uint8_t*)0x20000094 = 0;
    *(uint8_t*)0x20000095 = 0;
    *(uint8_t*)0x20000096 = 0;
    *(uint8_t*)0x20000097 = 0;
    *(uint32_t*)0x20000098 = 0;
    syscall(__NR_bind, r[1], 0x20000080, 0x1c);
    break;
  case 3:
    syscall(__NR_listen, r[1], 0);
    break;
  case 4:
    res = syscall(__NR_socket, 0xa, 1, 0);
    if (res != -1)
      r[2] = res;
    break;
  case 5:
    *(uint32_t*)0x20000040 = 0x80;
    syscall(__NR_accept, r[1], 0x200001c0, 0x20000040);
    break;
  case 6:
    *(uint16_t*)0x20000000 = 0xa;
    *(uint16_t*)0x20000002 = htobe16(0x4e22);
    *(uint32_t*)0x20000004 = 0;
    *(uint8_t*)0x20000008 = 0;
    *(uint8_t*)0x20000009 = 0;
    *(uint8_t*)0x2000000a = 0;
    *(uint8_t*)0x2000000b = 0;
    *(uint8_t*)0x2000000c = 0;
    *(uint8_t*)0x2000000d = 0;
    *(uint8_t*)0x2000000e = 0;
    *(uint8_t*)0x2000000f = 0;
    *(uint8_t*)0x20000010 = 0;
    *(uint8_t*)0x20000011 = 0;
    *(uint8_t*)0x20000012 = 0;
    *(uint8_t*)0x20000013 = 0;
    *(uint8_t*)0x20000014 = 0;
    *(uint8_t*)0x20000015 = 0;
    *(uint8_t*)0x20000016 = 0;
    *(uint8_t*)0x20000017 = 0;
    *(uint32_t*)0x20000018 = 0;
    syscall(__NR_sendto, r[2], 0x20000280, 0xfffffdf7, 0x20000003, 0x20000000,
            0x1c);
    break;
  case 7:
    *(uint32_t*)0x20000000 = 0;
    syscall(__NR_setsockopt, r[0], 0x112, 7, 0x20000000, 4);
    break;
  case 8:
    *(uint32_t*)0x20000280 = 0xf;
    *(uint32_t*)0x20000284 = 4;
    *(uint32_t*)0x20000288 = 4;
    *(uint32_t*)0x2000028c = 0x70;
    *(uint32_t*)0x20000290 = 0;
    *(uint32_t*)0x20000294 = -1;
    *(uint32_t*)0x20000298 = 0;
    *(uint8_t*)0x2000029c = 0;
    *(uint8_t*)0x2000029d = 0;
    *(uint8_t*)0x2000029e = 0;
    *(uint8_t*)0x2000029f = 0;
    *(uint8_t*)0x200002a0 = 0;
    *(uint8_t*)0x200002a1 = 0;
    *(uint8_t*)0x200002a2 = 0;
    *(uint8_t*)0x200002a3 = 0;
    *(uint8_t*)0x200002a4 = 0;
    *(uint8_t*)0x200002a5 = 0;
    *(uint8_t*)0x200002a6 = 0;
    *(uint8_t*)0x200002a7 = 0;
    *(uint8_t*)0x200002a8 = 0;
    *(uint8_t*)0x200002a9 = 0;
    *(uint8_t*)0x200002aa = 0;
    *(uint8_t*)0x200002ab = 0;
    res = syscall(__NR_bpf, 0, 0x20000280, 0x2c);
    if (res != -1)
      r[3] = res;
    break;
  case 9:
    *(uint32_t*)0x20000180 = r[3];
    *(uint64_t*)0x20000188 = 0x20000000;
    *(uint64_t*)0x20000190 = 0x20000140;
    *(uint64_t*)0x20000198 = 0;
    syscall(__NR_bpf, 2, 0x20000180, 0x20);
    break;
  }
}

void execute_one()
{
  execute(10);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (procid = 0; procid < 8; procid++) {
    if (fork() == 0) {
      for (;;) {
        loop();
      }
    }
  }
  sleep(1000000);
  return 0;
}