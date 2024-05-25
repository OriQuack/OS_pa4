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

  for(int i = 0; i < 56; i++){
    sbrk(4 * 1024 * 1024);
    swapstat(&a, &b);
    printf(1, "%d %d\n", a, b);
    printf("iter: %d\n", i);
  }

  exit();
}
