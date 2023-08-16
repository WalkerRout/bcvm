
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "vm.h"
#include "repl.h"

void sighandler(int signum) {
  printf("Caught signal %d, exiting...\n", signum);
  vm_free();
  exit(0);
}

int main(int argc, const char *argv[]) {
  (void) signal(SIGINT, sighandler); // Ctrl + C

  vm_init();

  if (argc == 1) {
    repl_run();
  } else if (argc == 2) {
    repl_run_file(argv[1]);
  } else {
    fprintf(stderr, "Usage: interpreter [path]\n");
    exit(64);
  }

  vm_free();

  return 0;
}
