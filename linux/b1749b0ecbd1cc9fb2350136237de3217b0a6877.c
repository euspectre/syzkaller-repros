// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <linux/futex.h>

static void sleep_ms(uint64_t ms)
{
  usleep(ms * 1000);
}

static uint64_t current_time_ms(void)
{
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    exit(1);
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static void thread_start(void* (*fn)(void*), void* arg)
{
  pthread_t th;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, 128 << 10);
  int i;
  for (i = 0; i < 100; i++) {
    if (pthread_create(&th, &attr, fn, arg) == 0) {
      pthread_attr_destroy(&attr);
      return;
    }
    if (errno == EAGAIN) {
      usleep(50);
      continue;
    }
    break;
  }
  exit(1);
}

#define BITMASK(bf_off, bf_len) (((1ull << (bf_len)) - 1) << (bf_off))
#define STORE_BY_BITMASK(type, htobe, addr, val, bf_off, bf_len)               \
  *(type*)(addr) =                                                             \
      htobe((htobe(*(type*)(addr)) & ~BITMASK((bf_off), (bf_len))) |           \
            (((type)(val) << (bf_off)) & BITMASK((bf_off), (bf_len))))

typedef struct {
  int state;
} event_t;

static void event_init(event_t* ev)
{
  ev->state = 0;
}

static void event_reset(event_t* ev)
{
  ev->state = 0;
}

static void event_set(event_t* ev)
{
  if (ev->state)
    exit(1);
  __atomic_store_n(&ev->state, 1, __ATOMIC_RELEASE);
  syscall(SYS_futex, &ev->state, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, 1000000);
}

static void event_wait(event_t* ev)
{
  while (!__atomic_load_n(&ev->state, __ATOMIC_ACQUIRE))
    syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, 0);
}

static int event_isset(event_t* ev)
{
  return __atomic_load_n(&ev->state, __ATOMIC_ACQUIRE);
}

static int event_timedwait(event_t* ev, uint64_t timeout)
{
  uint64_t start = current_time_ms();
  uint64_t now = start;
  for (;;) {
    uint64_t remain = timeout - (now - start);
    struct timespec ts;
    ts.tv_sec = remain / 1000;
    ts.tv_nsec = (remain % 1000) * 1000 * 1000;
    syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, &ts);
    if (__atomic_load_n(&ev->state, __ATOMIC_RELAXED))
      return 1;
    now = current_time_ms();
    if (now - start > timeout)
      return 0;
  }
}

struct thread_t {
  int created, call;
  event_t ready, done;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;

static void* thr(void* arg)
{
  struct thread_t* th = (struct thread_t*)arg;
  for (;;) {
    event_wait(&th->ready);
    event_reset(&th->ready);
    execute_call(th->call);
    __atomic_fetch_sub(&running, 1, __ATOMIC_RELAXED);
    event_set(&th->done);
  }
  return 0;
}

static void loop(void)
{
  int i, call, thread;
  int collide = 0;
again:
  for (call = 0; call < 6; call++) {
    for (thread = 0; thread < (int)(sizeof(threads) / sizeof(threads[0]));
         thread++) {
      struct thread_t* th = &threads[thread];
      if (!th->created) {
        th->created = 1;
        event_init(&th->ready);
        event_init(&th->done);
        event_set(&th->done);
        thread_start(thr, th);
      }
      if (!event_isset(&th->done))
        continue;
      event_reset(&th->done);
      th->call = call;
      __atomic_fetch_add(&running, 1, __ATOMIC_RELAXED);
      event_set(&th->ready);
      if (collide && (call % 2) == 0)
        break;
      event_timedwait(&th->done, 45);
      break;
    }
  }
  for (i = 0; i < 100 && __atomic_load_n(&running, __ATOMIC_RELAXED); i++)
    sleep_ms(1);
  if (!collide) {
    collide = 1;
    goto again;
  }
}

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res;
  switch (call) {
  case 0:
    res = syscall(__NR_socket, 0x10ul, 3ul, 0xcul);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    *(uint64_t*)0x20000280 = 0;
    *(uint32_t*)0x20000288 = 0x2d3;
    *(uint64_t*)0x20000290 = 0x20000240;
    *(uint64_t*)0x20000240 = 0x200002c0;
    memcpy(
        (void*)0x200002c0,
        "\x14\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00"
        "\x00\x00\x0a\x20\x00\x00\x00\x00\x0a\x01\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x09\x00\x01\x00\x73\x79\x7a\x30\x00\x00\x00"
        "\x00\x70\x00\x00\x00\x12\x0a\x01\x00\x00\x1f\x34\x01\x04\x00\x00\x00"
        "\x00\x00\x00\x00\x04\x00\x04\x80\x09\x00\x02\x00\x73\x79\x7a\x30\x00"
        "\x00\x00\x00\x09\x00\x01\x00\x73\x79\x7a\x30\x00\x00\x00\x00\x08\x00"
        "\x03\x40\x00\x00\x00\x00\x22\x00\x02\x00\xb7\x63\x12\xc4\x11\x0d\xd9"
        "\xb1\x73\x79\xe7\x28\x14\x7a\xfe\x6a\xb9\xf3\x2a\xa9\xd1\x00\x04\x80"
        "\x00\x00\x04\x00\x06\x14\x00\x04\x80\x09\x00\x01\x00\x59\x79\x7a\x30"
        "\x00\x00\x00\x00\x00\x00\x14\x00\x00\x00\x11\x00\x6d\x30\xa1\x6e\x90"
        "\x01\x6c\x47\x8a\x50\x48\x3a\x73\x64\x5a\x6d\xc4\xb5\x34\x5d\x2c\x0d"
        "\xb7\x4c\x9f\x89\x67\x8f\xbc\x9c\xc6\xfb\x9e\x8e\x31\xfa\xd7\xe7\x58"
        "\x0e\x6a\xe4\x35\x8d\x8f\x95\xa2\x73\x5d\x57\xdc\x6e\x3a\xf5\xcf\x3e"
        "\x09\xaa\x68\xb1\x91\xfe\xa4\x67\x66\xe9\xa1\xb3\xb4\xc9\xb0\x75\xf7"
        "\x4c\x98\x1f\x80\x44\x53\x89\x8a\x0b\x8f\x9d\x08\x1e\xf6\x43\x7d\x40"
        "\x51\xa3\x1e\xef\xf8\x78\x44\x21\xe4\x78\xa6\xe0\x4e\x0f\xb3\x69\xef"
        "\x95\xf2\x89\xff\x35\xa2\x43\x34\xba\x0d\x5a\xc0\x37\x02\x8b\x66\x59"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
        299);
    *(uint64_t*)0x20000248 = 0xb8;
    *(uint64_t*)0x20000298 = 1;
    *(uint64_t*)0x200002a0 = 0;
    *(uint64_t*)0x200002a8 = 0;
    *(uint32_t*)0x200002b0 = 0;
    syscall(__NR_sendmsg, -1, 0x20000280ul, 0ul);
    break;
  case 2:
    *(uint64_t*)0x20000600 = 0x20000500;
    *(uint16_t*)0x20000500 = 0x10;
    *(uint16_t*)0x20000502 = 0;
    *(uint32_t*)0x20000504 = 0;
    *(uint32_t*)0x20000508 = 0x80001100;
    *(uint32_t*)0x20000608 = 0xc;
    *(uint64_t*)0x20000610 = 0x200005c0;
    *(uint64_t*)0x200005c0 = 0x20000540;
    *(uint32_t*)0x20000540 = 0x60;
    *(uint8_t*)0x20000544 = 2;
    *(uint8_t*)0x20000545 = 7;
    *(uint16_t*)0x20000546 = 0x101;
    *(uint32_t*)0x20000548 = 0;
    *(uint32_t*)0x2000054c = 0;
    *(uint8_t*)0x20000550 = 0xa;
    *(uint8_t*)0x20000551 = 0;
    *(uint16_t*)0x20000552 = htobe16(3);
    *(uint16_t*)0x20000554 = 0xc;
    STORE_BY_BITMASK(uint16_t, , 0x20000556, 2, 0, 14);
    STORE_BY_BITMASK(uint16_t, , 0x20000557, 1, 6, 1);
    STORE_BY_BITMASK(uint16_t, , 0x20000557, 0, 7, 1);
    *(uint64_t*)0x20000558 = htobe64(4);
    *(uint16_t*)0x20000560 = 0xc;
    STORE_BY_BITMASK(uint16_t, , 0x20000562, 6, 0, 14);
    STORE_BY_BITMASK(uint16_t, , 0x20000563, 1, 6, 1);
    STORE_BY_BITMASK(uint16_t, , 0x20000563, 0, 7, 1);
    *(uint64_t*)0x20000564 = htobe64(0);
    *(uint16_t*)0x2000056c = 8;
    STORE_BY_BITMASK(uint16_t, , 0x2000056e, 5, 0, 14);
    STORE_BY_BITMASK(uint16_t, , 0x2000056f, 1, 6, 1);
    STORE_BY_BITMASK(uint16_t, , 0x2000056f, 0, 7, 1);
    *(uint32_t*)0x20000570 = htobe32(3);
    *(uint16_t*)0x20000574 = 0xc;
    STORE_BY_BITMASK(uint16_t, , 0x20000576, 2, 0, 14);
    STORE_BY_BITMASK(uint16_t, , 0x20000577, 1, 6, 1);
    STORE_BY_BITMASK(uint16_t, , 0x20000577, 0, 7, 1);
    *(uint64_t*)0x20000578 = htobe64(2);
    *(uint16_t*)0x20000580 = 8;
    STORE_BY_BITMASK(uint16_t, , 0x20000582, 5, 0, 14);
    STORE_BY_BITMASK(uint16_t, , 0x20000583, 1, 6, 1);
    STORE_BY_BITMASK(uint16_t, , 0x20000583, 0, 7, 1);
    *(uint32_t*)0x20000584 = htobe32(0);
    *(uint16_t*)0x20000588 = 0xc;
    STORE_BY_BITMASK(uint16_t, , 0x2000058a, 2, 0, 14);
    STORE_BY_BITMASK(uint16_t, , 0x2000058b, 1, 6, 1);
    STORE_BY_BITMASK(uint16_t, , 0x2000058b, 0, 7, 1);
    *(uint64_t*)0x2000058c = htobe64(0xffff);
    *(uint16_t*)0x20000594 = 0xc;
    STORE_BY_BITMASK(uint16_t, , 0x20000596, 6, 0, 14);
    STORE_BY_BITMASK(uint16_t, , 0x20000597, 1, 6, 1);
    STORE_BY_BITMASK(uint16_t, , 0x20000597, 0, 7, 1);
    *(uint64_t*)0x20000598 = htobe64(0x39c);
    *(uint64_t*)0x200005c8 = 0x60;
    *(uint64_t*)0x20000618 = 1;
    *(uint64_t*)0x20000620 = 0;
    *(uint64_t*)0x20000628 = 0;
    *(uint32_t*)0x20000630 = 0x10;
    syscall(__NR_sendmsg, -1, 0x20000600ul, 0x40000ul);
    break;
  case 3:
    res = syscall(__NR_socket, 0x10ul, 3ul, 0xcul);
    if (res != -1)
      r[1] = res;
    break;
  case 4:
    *(uint64_t*)0x20000280 = 0;
    *(uint32_t*)0x20000288 = 0x2d3;
    *(uint64_t*)0x20000290 = 0x20000240;
    *(uint64_t*)0x20000240 = 0x200002c0;
    memcpy(
        (void*)0x200002c0,
        "\x14\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00"
        "\x00\x00\x0a\x20\x00\x00\x00\x00\x0a\x01\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x09\x00\x01\x00\x73\x79\x7a\x30\x00\x00\x00"
        "\x00\x70\x00\x00\x00\x12\x0a\x01\x00\x00\x1f\x34\x01\x04\x00\x00\x00"
        "\x00\x00\x00\x00\x04\x00\x04\x80\x09\x00\x02\x00\x73\x79\x7a\x30\x00"
        "\x00\x00\x00\x09\x00\x01\x00\x73\x79\x7a\x30\x00\x00\x00\x00\x08\x00"
        "\x03\x40\x00\x00\x00\x00\x22\x00\x02\x00\xb7\x63\x12\xc4\x11\x0d\xd9"
        "\xb1\x73\x79\xe7\x28\x14\x7a\xfe\x6a\xb9\xf3\x2a\xa9\xd1\x00\x04\x80"
        "\x00\x00\x04\x00\x06\x14\x00\x04\x80\x09\x00\x01\x00\x59\x79\x7a\x30"
        "\x00\x00\x00\x00\x00\x00\x14\x00\x00\x00\x11\x00\x6d\x30\xa1\x6e\x90"
        "\x01\x6c\x47\x8a\x50\x48\x3a\x73\x64\x5a\x6d\xc4\xb5\x34\x5d\x2c\x0d"
        "\xb7\x4c\x9f\x89\x67\x8f\xbc\x9c\xc6\xfb\x9e\x8e\x31\xfa\xd7\xe7\x58"
        "\x0e\x6a\xe4\x35\x8d\x8f\x95\xa2\x73\x5d\x57\xdc\x6e\x3a\xf5\xcf\x3e"
        "\x09\xaa\x68\xb1\x91\xfe\xa4\x67\x66\xe9\xa1\xb3\xb4\xc9\xb0\x75\xf7"
        "\x4c\x98\x1f\x80\x44\x53\x89\x8a\x0b\x8f\x9d\x08\x1e\xf6\x43\x7d\x40"
        "\x51\xa3\x1e\xef\xf8\x78\x44\x21\xe4\x78\xa6\xe0\x4e\x0f\xb3\x69\xef"
        "\x95\xf2\x89\xff\x35\xa2\x43\x34\xba\x0d\x5a\xc0\x37\x02\x8b\x66\x59"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
        299);
    *(uint64_t*)0x20000248 = 0xb8;
    *(uint64_t*)0x20000298 = 1;
    *(uint64_t*)0x200002a0 = 0;
    *(uint64_t*)0x200002a8 = 0;
    *(uint32_t*)0x200002b0 = 0;
    syscall(__NR_sendmsg, r[1], 0x20000280ul, 0ul);
    break;
  case 5:
    *(uint64_t*)0x20000180 = 0;
    *(uint32_t*)0x20000188 = 0;
    *(uint64_t*)0x20000190 = 0x20000c00;
    *(uint64_t*)0x20000c00 = 0x20000000;
    *(uint32_t*)0x20000000 = 0x14;
    *(uint16_t*)0x20000004 = 0x10;
    *(uint16_t*)0x20000006 = 1;
    *(uint32_t*)0x20000008 = 0;
    *(uint32_t*)0x2000000c = 0;
    *(uint8_t*)0x20000010 = 0;
    *(uint8_t*)0x20000011 = 0;
    *(uint16_t*)0x20000012 = htobe16(0xa);
    *(uint32_t*)0x20000014 = 0x44;
    *(uint8_t*)0x20000018 = 9;
    *(uint8_t*)0x20000019 = 0xa;
    *(uint16_t*)0x2000001a = 0xdfc9;
    *(uint32_t*)0x2000001c = 0;
    *(uint32_t*)0x20000020 = 0;
    *(uint8_t*)0x20000024 = 0;
    *(uint8_t*)0x20000025 = 0;
    *(uint16_t*)0x20000026 = htobe16(0);
    *(uint16_t*)0x20000028 = 8;
    STORE_BY_BITMASK(uint16_t, , 0x2000002a, 5, 0, 14);
    STORE_BY_BITMASK(uint16_t, , 0x2000002b, 1, 6, 1);
    STORE_BY_BITMASK(uint16_t, , 0x2000002b, 0, 7, 1);
    *(uint32_t*)0x2000002c = htobe32(0x2b);
    *(uint16_t*)0x20000030 = 9;
    *(uint16_t*)0x20000032 = 1;
    memcpy((void*)0x20000034, "syz0\000", 5);
    *(uint16_t*)0x2000003c = 8;
    STORE_BY_BITMASK(uint16_t, , 0x2000003e, 0xa, 0, 14);
    STORE_BY_BITMASK(uint16_t, , 0x2000003f, 1, 6, 1);
    STORE_BY_BITMASK(uint16_t, , 0x2000003f, 0, 7, 1);
    *(uint32_t*)0x20000040 = htobe32(0);
    *(uint16_t*)0x20000044 = 9;
    *(uint16_t*)0x20000046 = 2;
    memcpy((void*)0x20000048, "syz1\000", 5);
    *(uint16_t*)0x20000050 = 8;
    STORE_BY_BITMASK(uint16_t, , 0x20000052, 8, 0, 14);
    STORE_BY_BITMASK(uint16_t, , 0x20000053, 1, 6, 1);
    STORE_BY_BITMASK(uint16_t, , 0x20000053, 0, 7, 1);
    *(uint32_t*)0x20000054 = htobe32(2);
    *(uint32_t*)0x20000058 = 0x14;
    *(uint16_t*)0x2000005c = 0x11;
    *(uint16_t*)0x2000005e = 1;
    *(uint32_t*)0x20000060 = 0;
    *(uint32_t*)0x20000064 = 0;
    *(uint8_t*)0x20000068 = 0;
    *(uint8_t*)0x20000069 = 0;
    *(uint16_t*)0x2000006a = htobe16(0xa);
    *(uint64_t*)0x20000c08 = 0x6c;
    *(uint64_t*)0x20000198 = 1;
    *(uint64_t*)0x200001a0 = 0;
    *(uint64_t*)0x200001a8 = 0;
    *(uint32_t*)0x200001b0 = 0;
    syscall(__NR_sendmsg, r[0], 0x20000180ul, 0ul);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  loop();
  return 0;
}