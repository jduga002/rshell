#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <iostream>
#include <vector>
#include <string.h>
#include <string>

int PROGRAM_SUCCESS = 0;

using namespace std;

void ls(string dir, bool &mult_args, bool &show_all, bool &not_first, bool &is_recursive) {
    struct stat statbuf;
    vector<string> v_dirs;
    if (not_first) {
        cout << endl;
    }
    else {
        not_first = true;
    }
    if (mult_args) cout << dir << ":" << endl;

    DIR *dirp = opendir(dir.c_str());
    if (dirp == NULL) {
        perror("opendir failed");
        PROGRAM_SUCCESS = 1;
        return;
    }
    dirent *direntp;
    while ((direntp = readdir(dirp))) {
        if (direntp->d_name[0] != '.' || show_all) {
            cout << direntp->d_name << endl;  // use stat here to find attributes of file
        }
        if (is_recursive) {
            if (stat(direntp->d_name, &statbuf) == -1) {
                perror("stat");
                exit(1);
            }
            if (S_ISDIR(statbuf.st_mode)) {
                if (direntp->d_name[0] != '.') v_dirs.push_back(direntp->d_name);
                else if (show_all && 
                       !(strcmp(direntp->d_name,".")==0 || strcmp(direntp->d_name,"..")==0)) 
                    v_dirs.push_back(direntp->d_name);
            }
        }
    }
    if (errno == EBADF) {  // checks if readdir exited loop (returned NULL) because of error
        perror("readdir failed");
        PROGRAM_SUCCESS = 1;
    }

    if (-1 ==  closedir(dirp)) {
        perror("closedir failed");
    }
    for (unsigned i = 0; i < v_dirs.size(); i++)
        ls(v_dirs.at(i), mult_args, show_all, not_first, is_recursive);
}

int main(int argc, char **argv) {
    vector<string> v_dirs;
    vector<string> v_flags;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') v_dirs.push_back(argv[i]);
        else v_flags.push_back(argv[i]);
    }
    if (v_dirs.empty()) v_dirs.push_back(".");

    bool mult_args = false;
    bool show_all = false;
    bool not_first = false;
    bool is_recursive = false;
    for (unsigned i = 0; i < v_flags.size(); i++) {
        for (unsigned j = 1; j < v_flags.at(i).length(); j++) {
            if (v_flags.at(i).at(j) == 'a')
                show_all = true;
            if (v_flags.at(i).at(j) == 'R') {
                is_recursive = true;
                mult_args = true;
            }
        }
    }
    if (v_dirs.size() >= 2) mult_args = true;
    
    for (unsigned i = 0; i < v_dirs.size(); i++) {
        ls(v_dirs.at(i), mult_args, show_all, not_first, is_recursive);
    }
    /*string h = "hey";
    string i = "ick";
    bool isLess = h < i;
    cout << isLess << endl;
    cout << true << endl;*/
    return PROGRAM_SUCCESS;
}
