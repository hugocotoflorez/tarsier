#include <assert.h>
#include <fcntl.h>
#include <pty.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "term.h"

#define SHELL "/bin/bash"
#define ARGS "-i", NULL

Term term_start() {
  Term t;
  t.pid = forkpty(&t.master_fd, NULL, NULL, NULL);

  if (t.pid == -1) {
    perror("forkpty");
    exit(1);
  }

  if (t.pid == 0) {
    execl(SHELL, SHELL, ARGS);
    perror(SHELL);
    exit(1);
  }

  return t;
}

int term_send(Term t, char c) { return write(t.master_fd, &c, 1); }

char *read_fileno(int fileno, char *buf, int size) {
  char *ret = NULL;
  int flags = fcntl(fileno, F_GETFL, 0);
  fcntl(fileno, F_SETFL, flags | O_NONBLOCK);
  ssize_t n = read(fileno, buf, size - 1);
  if (n > 0) {
    buf[n] = 0;
    ret = buf;
  }
  fcntl(fileno, F_SETFL, flags);
  return ret;
}

// void async_read(Term t) {
//   char buf[1024];
//   setvbuf(stdout, NULL, _IONBF, 0);
//   // TODO: never closed
//   switch (fork()) {
//   case 0:
//     while (1) {
//       while (read_fileno(t.master_fd, buf, sizeof buf)) {
//         printf("%s", buf);
//       }
//     }
//     exit(1);
//   case -1:
//   default:
//     return;
//   }
// }
//
// int main() {
//   Term t = term_start();
//   async_read(t);
//
//   char c;
//   while (1) {
//     if (read(STDIN_FILENO, &c, 1) >= 0)
//       term_send(t, c);
//   }
//
//   return 0;
// }
