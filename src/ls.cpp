#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <iostream>
#include <vector>
#include <string>

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

int main(int argc, char **argv) {
    vector<string> v_dirs;
    vector<string> v_flags;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') v_dirs.push_back(argv[i]);
        else v_flags.push_back(argv[i]);
    }
    ls();
    return 0;
}

