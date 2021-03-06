#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "config.h"

void display_blocked();
void user_input(char buffer[], size_t buffer_size, char *prompt);
void guessing(int password);
void block();
void unblock();

int main() {
    char decision[255];
    char domain[255];

    srand(time(NULL));

    display_blocked();
    user_input(decision, sizeof(decision), "Do you want to block or unblock a website? [b/u] ");

    switch (*decision) {
        case 'U':
        case 'u':
            guessing(rand() % 100);
            user_input(domain, sizeof(domain), "Type the domain you want to unblock: ");
            unblock(domain);
            break;

        case 'B':
        case 'b':
            user_input(domain, sizeof(domain), "Type the domain you want to block: ");
            block(domain);
            break;

        default:
            main();
            break;
    }

    return 0;
}

void display_blocked() {
    FILE *file;
    char line[255];
    char *token;

    printf("The following domains are currently blocked:\n\n");

    file = fopen(HOSTS, "r");
    while (fgets(line, sizeof(line), file) != NULL) {
        if (line != NULL) {
            line[strcspn(line, "\n")] = 0;
            if (strstr(line, BLOCK_IP) != NULL && line[0] != '#') {
                token = strrchr(line, ' ');
                if (strcmp(token + 1, "localhost") != 0) {
                    printf("    - %s\n", token + 1);
                }
            }
        }
    }

    printf("\n");
}

void user_input(char buffer[], size_t buffer_size, char *prompt) {
    printf("%s", prompt);
    if (fgets(buffer, buffer_size, stdin) != NULL) {
        /* Remove trailing newline */
        buffer[strcspn(buffer, "\n")] = 0;
    } else {
        fprintf(stderr, "Invalid input, exiting...\n");
        exit(1);
    }
}

void guessing(int password) {
    char guess[255];
    
    user_input(guess, sizeof(guess), "Password: ");

    if (atoi(guess) != password) {
        guessing(password);
    }
}

void block(char *domain) {
    FILE *file;
    char line[255];

    file = fopen(HOSTS, "r");
    while (fgets(line, sizeof(line), file) != NULL) {
        if (strstr(line, domain) != NULL && strstr(line, BLOCK_IP) != NULL && line[0] != '#') {
            printf("%s is already blocked!\n", domain);
            return;
        }
    }

    fclose(file);

    file = fopen(HOSTS, "a");
    if (file == NULL) {
        perror("Could not edit hosts file");
    } else {
        fprintf(file, "%s %s\n", BLOCK_IP, domain);
        fclose(file);
    }
}

void unblock(char *domain) {
    FILE *file;
    FILE *file_tmp;
    char line[255];

    /* Use r+ instead of r here to check for write permissions */
    file = fopen(HOSTS, "r+");
    file_tmp = tmpfile();

    if (file == NULL || file_tmp == NULL) {
        perror("Could not edit hosts file");
    } else {
        while (fgets(line, sizeof(line), file) != NULL) {
            if (strstr(line, domain) == NULL || strstr(line, BLOCK_IP) == NULL || line[0] == '#') {
                fprintf(file_tmp, "%s", line);
            }
        }

        rewind(file_tmp);
        freopen(NULL, "w+", file);

        /* Copy temporary file contents to main hosts file */
        while (fgets(line, sizeof(line), file_tmp) != NULL) {
            fprintf(file, "%s", line);
        }

        fclose(file);
        fclose(file_tmp);
    }
}
