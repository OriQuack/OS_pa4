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
	int a, b;
  swapstat(&a, &b);
  printf(1, "%d %d\n", a, b);

  for(int i = 0; i < 1000; i++){
    sbrk(4096 * 4096);
  }

  swapstat(&a, &b);
  printf(1, "%d %d\n", a, b);

  for(int i = 0; i < 100; i++){
    sbrk(4096 * 4096);
  }

  swapstat(&a, &b);
  printf(1, "%d %d\n", a, b);

  exit();
}
