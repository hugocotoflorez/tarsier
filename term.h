#ifndef TERM_H_
#define TERM_H_

typedef struct Term {
        int pid;
        int master_fd;
} Term;

Term term_start();
int term_send(Term t, char c);
char *read_fileno(int fileno, char *buf, int size);

#endif // TERM_H_
