#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <iostream>

using namespace std;

/*
 * This is a BARE BONES example of how to use opendir/readdir/closedir.
 * Notice that there is no error checking on these functions.
 * You MUST add error checking yourself.
 */

int main()
{
    DIR *dirp = opendir(".");
    if (dirp == NULL) {
        perror("opendir failed");
        exit(1);
    }
    dirent *direntp;
    while ((direntp = readdir(dirp))) {
        cout << direntp->d_name << endl;  // use stat here to find attributes of file
    }
    if (errno == EBADF) {  // checks to see if readdir exited loop (returned NULL) because of error
        perror("readdir failed");
        exit(1);
    }

    if (-1 ==  closedir(dirp)) {
        perror("closedir failed");
    }
}

