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

  for(int i = 0; i < 55; i++){
    sbrk(4 * 1024 * 1024);
  }
  for(int i = 0; i < 190; i++){
    printf(1, "iter: %d\n", i);
    sbrk(4 * 1024);
    swapstat(&a, &b);
    printf(1, "%d %d\n", a, b);
  }

  exit();
}
