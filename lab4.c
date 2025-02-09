#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUF_SIZE 128
#define EXTRA_SIZE 256

struct header {
  uint64_t size;
  struct header *next;
};

void handle_error(const char *msg) {
  write(STDERR_FILENO, msg, strlen(msg));
  exit(EXIT_FAILURE);
}

void print_out(char *format, void *data, size_t data_size) {
  char buf[BUF_SIZE];
  ssize_t len;

  if (data_size == sizeof(uint64_t)) {
    len = snprintf(buf, BUF_SIZE, format, *(uint64_t *)data);
  } else {
    len = snprintf(buf, BUF_SIZE, format, *(void **)data);
  }

  if (len < 0) {
    handle_error("snprintf failed\n");
  }
  write(STDOUT_FILENO, buf, len);
}

int main(void) {
  void *heap_start = sbrk(EXTRA_SIZE);
  if (heap_start == (void *)-1) {
    handle_error("sbrk failed\n");
  }

  struct header *h1 = (struct header *)heap_start;
  struct header *h2 = (struct header *)((uint8_t *)heap_start + BUF_SIZE);

  h1->size = BUF_SIZE;
  h1->next = NULL;

  h2->size = BUF_SIZE;
  h2->next = h1;

  print_out("First block: %p\n", h1, sizeof(void *));
  print_out("Second block: %p\n", h2, sizeof(void *));
  print_out("First block size: %lu\n", &h1->size, sizeof(uint64_t));
  print_out("First block next: %p\n", &h1->next, sizeof(void *));
  print_out("Second block size: %lu\n", &h2->size, sizeof(uint64_t));
  print_out("Second block next: %p\n", &h2->next, sizeof(void *));

  memset((uint8_t *)h1 + sizeof(struct header), 0,
         BUF_SIZE - sizeof(struct header));
  memset((uint8_t *)h2 + sizeof(struct header), 1,
         BUF_SIZE - sizeof(struct header));

  uint8_t *data1 = (uint8_t *)h1 + sizeof(struct header);
  uint8_t *data2 = (uint8_t *)h2 + sizeof(struct header);

  for (int i = 0; i < BUF_SIZE - sizeof(struct header); i++) {
    print_out("%u\n", &data1[i], sizeof(uint8_t));
  }
  print_out("--------\n", NULL, 0);
  for (int i = 0; i < BUF_SIZE - sizeof(struct header); i++) {
    print_out("%u\n", &data2[i], sizeof(uint8_t));
  }

  return 0;
}
