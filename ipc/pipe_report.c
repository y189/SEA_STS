#include "pipe_report.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>

void init_report_pipe(int fd[2]) {
    if (pipe(fd) < 0) {
        perror("Pipe creation failed");
    }
}

void write_report(int fd_write, const char* log_msg) {
    // On écrit exactement la longueur de la chaîne + le caractère nul d'un seul bloc (atomique)
    write(fd_write, log_msg, strlen(log_msg) + 1);
}

void read_report(int fd_read, char* buffer, int max_len) {
    // On lit octet par octet jusqu'à trouver le délimiteur de fin de chaîne '\0'
    int i = 0;
    char c;
    while (i < max_len - 1) {
        int n = read(fd_read, &c, 1);
        if (n <= 0) break; // Plus rien à lire ou erreur
        buffer[i++] = c;
        if (c == '\0') break; // Fin du message
    }
    buffer[i] = '\0'; // Sécurité
}
