#include <xinu.h>

int32 wait_on_child(pid32 child) {
  int32 msg = recvclr();
  //resume(child);
  while (msg != child) {
    //printf("%d\n", msg);
    msg = receive();
  }
  printf("%d\n", msg);
  return OK;
}
