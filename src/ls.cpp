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
using namespace std;

int PROGRAM_SUCCESS = 0;

void ls_long(const vector<dirent *> &v_dirents, string dir_loc) {
    vector<struct stat> v_stats;
    for (unsigned i = 0; i < v_dirents.size(); i++) {
        struct stat statbuf;
        if (-1 == stat((dir_loc + v_dirents.at(i)->d_name).c_str(), &statbuf)) {
            perror("stat");
            PROGRAM_SUCCESS = 1;
            return;
        }
        v_stats.push_back(statbuf);
    }
    unsigned block_cnt = 0;
    for (unsigned i = 0; i < v_stats.size(); i++) {
        cout << v_dirents.at(i)->d_name << ": " << v_stats.at(i).st_blocks << endl;
        block_cnt += v_stats.at(i).st_blocks;
    }
    cout << "total " << block_cnt << endl;
}

void ls(string dir, bool &mult_args, bool &show_all, bool &not_first, bool &is_recursive, bool &is_long) {
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
        cerr << dir << ": ";
        perror("opendir failed");
        PROGRAM_SUCCESS = 1;
        return;
    }
    vector<dirent *> v_dirents;
    dirent *direntp;
    while ((direntp = readdir(dirp))) {
        if (direntp->d_name[0] != '.' || show_all) {
            //cout << direntp->d_name << endl;  // use stat here to find attributes of file
            v_dirents.push_back(direntp);
        }
        if (is_recursive) {
            string name = dir + "/" + direntp->d_name;
            if (stat(name.c_str(), &statbuf) == -1) {
                perror(("stat: " + name + ":").c_str());
                PROGRAM_SUCCESS = 1;
            }
            else if (S_ISDIR(statbuf.st_mode)) {
                if (direntp->d_name[0] != '.') v_dirs.push_back(name);
                else if (show_all && 
                       !(strcmp(direntp->d_name,".")==0 || strcmp(direntp->d_name,"..")==0)) 
                    v_dirs.push_back(name);
            }
        }
    }
    if (is_long) {
        ls_long(v_dirents, dir + "/");
    }
    else {
        for (unsigned i = 0; i < v_dirents.size(); i++) {
            cout << v_dirents.at(i)->d_name << endl;
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
        ls(v_dirs.at(i), mult_args, show_all, not_first, is_recursive, is_long);
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
    bool is_long = false;
    for (unsigned i = 0; i < v_flags.size(); i++) {
        for (unsigned j = 1; j < v_flags.at(i).length(); j++) {
            if (v_flags.at(i).at(j) == 'a')
                show_all = true;
            else if (v_flags.at(i).at(j) == 'R') {
                is_recursive = true;
                mult_args = true;
            }
            else if (v_flags.at(i).at(j) == 'l')
                is_long = true;
            else { 
                cerr << "Invalid option -- \'" << v_flags.at(i).at(j) << "\'" << endl;
                return 1;
            }
        }
    }
    if (v_dirs.size() >= 2) mult_args = true;
    
    for (unsigned i = 0; i < v_dirs.size(); i++) {
        ls(v_dirs.at(i), mult_args, show_all, not_first, is_recursive, is_long);
    }
    /*string h = "hey";
    string i = "ick";
    bool isLess = h < i;
    cout << isLess << endl;
    cout << true << endl;*/
    return PROGRAM_SUCCESS;
}
