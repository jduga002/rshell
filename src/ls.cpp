#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <vector>
#include <string.h>
#include <string>
using namespace std;

#define DIRECTORY "\033[34;1m"
#define EXECUTABLE "\033[32;1m"
#define HIDDEN "\033[40;1m"
#define RESET_COLOR "\033[m"

int PROGRAM_SUCCESS = 0;
const char * const FORMAT = "%b %d %R";

bool string_cmp(string s, string t) {
    for (unsigned k = 0; k < s.length(); k ++) s.at(k) = toupper(s.at(k));
    for (unsigned k = 0; k < t.length(); k ++) t.at(k) = toupper(t.at(k));
    return (s < t);
}

bool dirent_cmp(const dirent * const i, const dirent * const j) {
    string s = i->d_name; string t = j->d_name;
    return string_cmp(s,t);
}

void ls_long(const vector<struct stat> &v_stats, const vector<string> &v_names) {
    

    vector<string> v_uids;
    vector<string> v_gids;
    unsigned max_nlink_len = 0;
    unsigned max_size_len = 0;
    unsigned max_uid_len = 0;
    unsigned max_gid_len = 0;
    unsigned block_cnt = 0;
    for (unsigned i = 0; i < v_stats.size(); i++) {
        //cout << v_dirents.at(i)->d_name << ": " << v_stats.at(i).st_blocks << endl;
        block_cnt += v_stats.at(i).st_blocks;

        ostringstream sin;
        sin << v_stats.at(i).st_nlink;
        string s = sin.str();
        if (s.length() > max_nlink_len) {
            max_nlink_len = s.length();
        }

        sin.str("");
        sin << v_stats.at(i).st_size;
        s = sin.str();
        if (s.length() > max_size_len) {
            max_size_len = s.length();
        }

        struct passwd *p_uid = getpwuid(v_stats.at(i).st_uid);
        if (p_uid == NULL) {
            perror("getpwuid");
            v_uids.push_back(" ??");
        }
        else {
            v_uids.push_back(p_uid->pw_name);
        }
        if (v_uids.at(v_uids.size()-1).length() > max_uid_len) {
            max_uid_len = v_uids.at(v_uids.size()-1).length();
        }

        struct group * g_gid = getgrgid(v_stats.at(i).st_gid);
        if (g_gid == NULL) {
            perror("getgrgid");
            v_gids.push_back(" ??");
        }
        else {
            v_gids.push_back(g_gid->gr_name);
        }
        if (v_gids.at(v_gids.size()-1).length() > max_gid_len) {
            max_gid_len = v_gids.at(v_gids.size()-1).length();
        }
    }
    //cout << "total " << block_cnt << endl;

    for (unsigned i = 0; i < v_stats.size(); i++) {
        if (S_ISDIR(v_stats.at(i).st_mode)) cout << "d";
        else if (S_ISLNK(v_stats.at(i).st_mode)) cout << "l";
        else cout << "-";
        cout << ((v_stats.at(i).st_mode & S_IRUSR)?"r":"-");
        cout << ((v_stats.at(i).st_mode & S_IWUSR)?"w":"-");
        cout << ((v_stats.at(i).st_mode & S_IXUSR)?"x":"-");
        cout << ((v_stats.at(i).st_mode & S_IRGRP)?"r":"-");
        cout << ((v_stats.at(i).st_mode & S_IWGRP)?"w":"-");
        cout << ((v_stats.at(i).st_mode & S_IXGRP)?"x":"-");
        cout << ((v_stats.at(i).st_mode & S_IROTH)?"r":"-");
        cout << ((v_stats.at(i).st_mode & S_IWOTH)?"w":"-");
        cout << ((v_stats.at(i).st_mode & S_IXOTH)?"x":"-");
        cout << setw(max_nlink_len + 1) << v_stats.at(i).st_nlink;
        cout << setw(max_uid_len+1) << v_uids.at(i);
        cout << setw(max_gid_len+1) << v_gids.at(i);
        cout << setw(max_size_len + 1) << v_stats.at(i).st_size;
        struct tm result;
        if (NULL == localtime_r(&(v_stats.at(i).st_mtime), &result)) {
            perror("localtime_r");
            cout << " ????????????";
        }
        else {
            char time[256];
            if (strftime(time, 256, FORMAT, &result)) {
                cout << " " << time;
            }
            else cout << " ????????????";
        }
        cout << " ";
        if (v_stats.at(i).st_mode & S_IFDIR)
            cout << DIRECTORY;
        else if (v_stats.at(i).st_mode & S_IXUSR)
            cout << EXECUTABLE;
        if (v_names.at(i).at(0) == '.')
            cout << HIDDEN;

        cout << v_names.at(i) << RESET_COLOR << endl;
        if (S_ISLNK(v_stats.at(i).st_mode)) {
            cout << " <- " << flush;
        }
    }
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
    if (errno == EBADF) {  // checks if readdir exited loop (returned NULL) because of error
        perror("readdir failed");
        PROGRAM_SUCCESS = 1;
    }

    sort(v_dirents.begin(), v_dirents.end(), dirent_cmp);
    sort(v_dirs.begin(), v_dirs.end(), string_cmp);

    vector<struct stat> v_stats;
    for (unsigned i = 0; i < v_dirents.size(); i++) {
        struct stat statbuf;
        if (-1 == stat((dir + "/" + v_dirents.at(i)->d_name).c_str(), &statbuf)) {
            perror("stat");
            PROGRAM_SUCCESS = 1;
            return;
        }
        v_stats.push_back(statbuf);
    }

    if (is_long) {
        vector<string> v_names;
        for (unsigned i = 0; i < v_dirents.size(); i++) {
            v_names.push_back(v_dirents.at(i)->d_name);
        }
        ls_long(v_stats, v_names);
    }
    else {
        for (unsigned i = 0; i < v_dirents.size(); i++) {
            //if (S_ISDIR(v_stats.at(i).st_mode))
            if (v_stats.at(i).st_mode & S_IFDIR)
                cout << DIRECTORY;
            else if (v_stats.at(i).st_mode & S_IXUSR)
                cout << EXECUTABLE;
            if (v_dirents.at(i)->d_name[0] == '.')
                cout << HIDDEN;
            cout << v_dirents.at(i)->d_name << RESET_COLOR << endl;
        }
    }

    if (-1 ==  closedir(dirp)) {
        perror("closedir failed");
    }
    for (unsigned i = 0; i < v_dirs.size(); i++)
        ls(v_dirs.at(i), mult_args, show_all, not_first, is_recursive, is_long);
}

int main(int argc, char **argv) {
    vector<string> v_dirs;
    vector<string> v_files;
    vector<string> v_flags;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') v_flags.push_back(argv[i]);
        else {
            struct stat statbuf;
            if (-1 == stat(argv[i], &statbuf)) {
                string errorstr = argv[i];
                errorstr += ": error in stat";
                perror(errorstr.c_str());
            }
            else if (S_ISDIR(statbuf.st_mode)) {
                v_dirs.push_back(argv[i]);
            }
            else {
                v_files.push_back(argv[i]);
            }
        }
    }
    if (v_dirs.empty() && v_files.empty()) v_dirs.push_back(".");

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

    sort(v_files.begin(), v_files.end(), string_cmp);
    sort(v_dirs.begin(), v_dirs.end(), string_cmp);
    
    vector<struct stat> v_filestats;
    for (unsigned i = 0; i < v_files.size(); i++) {
        struct stat statbuf;
        if (-1 == stat((v_files.at(i)).c_str(), &statbuf)) {
            perror("stat");
            return 1;
        }
        v_filestats.push_back(statbuf);
    } 

    if (is_long && !v_files.empty()) {
        ls_long(v_filestats, v_files);
    }
    else {
        for (unsigned i = 0; i < v_files.size(); i++) {
            //don't need to check if directory because all these are files
            if (v_filestats.at(i).st_mode & S_IXUSR) cout << EXECUTABLE;
            if (v_files.at(i).at(0) == '.') cout << HIDDEN;
            cout << v_files.at(i) << RESET_COLOR << endl;
        }
    }

    if (!v_files.empty() && !v_dirs.empty()) {
        cout << endl;
        mult_args = true;
    }

    for (unsigned i = 0; i < v_dirs.size(); i++) {
        ls(v_dirs.at(i), mult_args, show_all, not_first, is_recursive, is_long);
    }
    return PROGRAM_SUCCESS;
}
