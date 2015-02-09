#include <iostream>
#include <stdio.h>
#include <istream>
#include <unistd.h>
#include <sstream>
#include <string>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include "Timer.h"
using namespace std;

bool file_exists(char *file) {
    struct stat info;
    int num = stat(file, &info);
    if (num == -1) {
        return false;
    }
    else {
        return true;
    }
    cerr << "This shouldn't output" << endl;
    return false;
}

void getput(char *input, char *output) {
    ifstream fin(input);
    ofstream fout(output);
    char c;
    while (fin.get(c)) {
        fout.put(c);
        //put in output
    }
}

void readwrite(char *input, char *output) {
    int fd;
    if (-1 == (fd = open(input, O_RDONLY))) {
        perror("error with opening input");
        exit(EXIT_FAILURE);
    }
    int fd2;
    if (-1 == (fd2 = open(output, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR))) {
        perror("error with opening output");
        exit(EXIT_FAILURE);
    }
    int size;
    char c[1];
    if (-1 == (size = read(fd, c, sizeof(c)))) {
        perror("error with reading");
        exit(EXIT_FAILURE);
    }
    while (size != 0) {
        if (-1 == write(fd2, c, size)) {
            perror("error with writing");
            exit(EXIT_FAILURE);
        }
        if (-1 == (size = read(fd, c, sizeof(c)))) {
            perror("error with reading");
            exit(EXIT_FAILURE);
        }
    }
    if (-1 == close(fd)) {
        perror("error closing input file");
        exit(EXIT_FAILURE);
    }
    if (-1 == close(fd2)) {
        perror("error closing output file");
        exit(EXIT_FAILURE);
    }
}

void readwritebuf(char *input, char *output) {
    int fd;
    if (-1 == (fd = open(input, O_RDONLY))) {
        perror("error with opening input");
        exit(EXIT_FAILURE);
    }
    int fd2;
    if (-1 == (fd2 = open(output, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR))) {
        perror("error with opening output");
        exit(EXIT_FAILURE);
    }
    int size;
    char c[BUFSIZ];
    if (-1 == (size = read(fd, c, sizeof(c)))) {
        perror("error with reading");
        exit(EXIT_FAILURE);
    }
    while (size != 0) {
        if (-1 == write(fd2, c, size)) {
            perror("error with writing");
            exit(EXIT_FAILURE);
        }
        if (-1 == (size = read(fd, c, sizeof(c)))) {
            perror("error with reading");
            exit(EXIT_FAILURE);
        }
    }
    if (-1 == close(fd)) {
        perror("error closing input file");
        exit(EXIT_FAILURE);
    }
    if (-1 == close(fd2)) {
        perror("error closing output file");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv) {
    if (argc == 3) {
        if (!file_exists(argv[1])) {
            perror("error with stat");
            return 1;
        }
        else if (file_exists(argv[2])) {
            cerr << "Output file does exist" << endl;
            return 1;
        }
        readwritebuf(argv[1], argv[2]);
    }
    else if (argc == 4) {
        if (!file_exists(argv[1])) {
            perror("error with stat");
            return 1;
        }
        else if (file_exists(argv[2])) {
            cerr << "Output file does exist" << endl;
            return 1;
        }
        Timer t1, t2, t3;
        double eTime1, eTime2, eTime3;
        t1.start();
        getput(argv[1], argv[2]);
        t1.elapsedUserTime(eTime1);
        if (remove(argv[2]) != 0) {
            perror("error deleting file");
            exit(EXIT_FAILURE);
        }
        t2.start();
        readwrite(argv[1], argv[2]);
        t2.elapsedUserTime(eTime2);
        if (remove(argv[2]) != 0) {
            perror("error deleting file");
            exit(EXIT_FAILURE);
        }
        t3.start();
        readwritebuf(argv[1], argv[2]);
        t3.elapsedUserTime(eTime3);

        cout << "Time for getput(): " << eTime1 << endl;
        cout << "Time for readwrite(): " << eTime2 << endl;
        cout << "Time for readwritebuf(): " << eTime3 << endl;
    }
    else {
    cout << "wrong number of args" << endl;
    return 1;
    }
    return 0;
}
