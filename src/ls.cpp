#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <iostream>

using namespace std;

void ls() {
DIR *dirp = opendir(".");
    if (dirp == NULL) {
        perror("opendir failed");
        exit(1);
    }
    dirent *direntp;
    while ((direntp = readdir(dirp))) {
        cout << direntp->d_name << endl;  // use stat here to find attributes of file
    }
    if (errno == EBADF) {  // checks if readdir exited loop (returned NULL) because of error
        perror("readdir failed");
        exit(1);
    }

    if (-1 ==  closedir(dirp)) {
        perror("closedir failed");
        exit(0);
    }
}

int main() {
    ls();
    return 0;
}

