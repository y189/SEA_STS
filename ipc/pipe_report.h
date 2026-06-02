#ifndef PIPE_REPORT_H
#define PIPE_REPORT_H

void init_report_pipe(int fd[2]);
void write_report(int fd_write, const char* log_msg);
void read_report(int fd_read, char* buffer, int max_len);

#endif
