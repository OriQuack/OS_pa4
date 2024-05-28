#include "param.h"
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"
#include "syscall.h"
#include "traps.h"
#include "memlayout.h"


int main () {
  printf(1, "Start");
	int a, b;
  swapstat(&a, &b);
  printf(1, "%d %d\n", a, b);

  char* mem = sbrk(4 * 1024 * 1024);
  mem[0] = 33;

  for(int i = 0; i < 54; i++){
    char* m = sbrk(4 * 1024 * 1024);
    m[0] = i;
  }
  for(int i = 0; i < 200; i++){
    printf(1, "iter: %d\n", i);
    sbrk(4 * 1024);
    swapstat(&a, &b);
    printf(1, "%d %d\n", a, b);
  }

  if(fork() == 0){
    printf(1, "CHILD: %d\n", mem[0]);
    printf(1, "CHILD: %d\n", mem[4 * 1024 * 1024 * 4]);
    swapstat(&a, &b);
    printf(1, "%d %d\n", a, b);
    printf(1, "CHILD EXIT\n");
    exit();
  }

  mem[0] = 3;
  swapstat(&a, &b);
  printf(1, "%d %d\n", a, b);
  mem[4096] = 4;
  swapstat(&a, &b);
  printf(1, "%d %d\n", a, b);
  mem[4096 * 2] = 89;
  swapstat(&a, &b);
  printf(1, "%d %d\n", a, b);

  printf(1, "**PROGRAM DONE**\n");
  wait();
  exit();
}
