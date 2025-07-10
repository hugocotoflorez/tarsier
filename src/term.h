#ifndef TERM_H_
#define TERM_H_

typedef struct Term {
        int pid;
        int master_fd;
} Term;

Term term_start();
int term_send(Term t, char c);
int term_send_str(Term t, char *s);
char *read_fileno(int fileno, char *buf, int size);
void resize_term(Term t, int hc, int wc, int hpx, int wpx);
void term_close(Term t);

#endif // TERM_H_
