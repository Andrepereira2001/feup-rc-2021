#ifndef UTILS_H_
#define UTILS_H_

#define DATA_SIZE 255

struct args {
    char user[DATA_SIZE];
    char password[DATA_SIZE];
    char host[DATA_SIZE];
    char url[DATA_SIZE];
};

int parseArguments(char *commandArgs);

#endif // UTILS_H_