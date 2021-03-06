#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
using namespace std;

const int MAX_LINE_LENGTH = 2048;
const char ERROR[] = "rshell: syntax error on connector: use ';', '&&', or '||'";
const char ERROR_IOPIP[] = "rshell: syntax error on IO redirection: use '<', '<<<', '>', '>>'. or '|'";
const char LENGTH_ERROR[] = "rshell: error on input: too many chars; exiting rshell";
const char CONN_IOPIP_ERROR[] = "rshell: error: rshell currently does not support the use of connectors and IO Redirection/Piping together";
const char ERROR_MULT_INPUT[] = "rshell: error on input redirection: please use only one of < or <<< per command";
const char ERROR_MULT_OUTPUT[] = "rshell: error on output redirection: please use only one of > or >> per command";
const char CD_ERR[] = "cd: usage: cd directory";

const int P_READ = 0;
const int P_WRITE = 1;

void handler(int i) {
    if (i == SIGINT) { }
}

// error in function; will go unused
void getUsername(char *username) {
    if (NULL == (username = getlogin())) {
        perror("getlogin() error");
        strcpy(username,"???");
    }
}

// probably error in this function, so will go unused
void getHost(char *hostName) {
    if (-1 == gethostname(hostName, MAX_LINE_LENGTH - 1) ) { 
        perror("gethostname() error");
    }
}

int change_dir(vector<char *> &command) {
    if (command.size() == 2) { // no args passed to cd
        cerr << CD_ERR << endl;
        return 1;
    }
    int ret;
    if (-1 == (ret = chdir(command.at(1))))
        perror("chdir");
    else {
        char wrk_dir[PATH_MAX+1];
        if (NULL == getcwd(wrk_dir,PATH_MAX+1)) {
            perror("getcwd");
            exit(1);
        }
        if (-1 == setenv("PWD",wrk_dir,1)) {
            perror("setenv");
            exit(1);
        }
    }
    return ret;
}


bool find_path(string &command) {
    char *environ_path;
    if (NULL == (environ_path = getenv("PATH"))) {
        perror("getenv: error in $PATH");
        exit(1);
    }
    string env_path_str = environ_path;
    char *path_cpy = &env_path_str[0];
    char * str_tok = strtok(path_cpy, ":");
    while (str_tok != NULL) {
        struct stat buf;
        string path= str_tok;
        path += '/';
        path += command;
        if (0 == stat(path.c_str(), &buf)) {
            command = path;
            return true;
        }
        str_tok = strtok(NULL, ":");
    }
    perror(command.c_str());
    return false;
}

bool has_slash(const string &str) {
    for (unsigned i = 0; i < str.size(); i++) {
        if (str.at(i) == '/') return true;
    }
    return false;
}

int exec_command(vector<char *> &command) { 
    if (strcmp(command[0],"exit") == 0)
        exit(0);
    else if (strcmp(command[0],"cd") == 0)
        return change_dir(command);

    string cmd = command.at(0);
    if (!has_slash(cmd)) {
        if (!find_path(cmd)) {
            return 1;
        }
        command.at(0) = &cmd[0];
    }
    
    int x = 1;
    int pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(1);
    }
    else if (pid == 0) { // child process
        signal(SIGINT, SIG_DFL);
        //execute command
        char **command_arr = &command[0];
        if (-1 == execv(command_arr[0], command_arr)) {
            perror("execv");
            exit(1);
        }
        exit(0);
    } 
    else { // parent
        if (-1 == wait(&x)) { // parent function waits for child
            perror("wait");  // program exits if there is an error
            exit(1);
        }
    }
    return x;
}

bool hasString(const vector<char *> &v, const char * string) {
    for (unsigned i = 0; i < v.size(); i++) {
        if (v.at(i) != NULL) {
            if (strcmp(v.at(i), string) == 0) 
                return true;
        }
    }
    return false;
}

bool hasMultStrings(const vector<char *> &v, const char * string1, const char *string2) {
    unsigned cnt = 0;
    for (unsigned i = 0; i < v.size(); i++) {
        if (v.at(i) != NULL) {
            if (strcmp(v.at(i), string1) == 0) 
                cnt++;
            else if (strcmp(v.at(i), string2) == 0) 
                cnt++;

            if (cnt > 1) 
                return true;
        }
    }
    return false;
}

bool last_is_iopip(const vector<char *> &v) {
    unsigned last = v.size()-1;
    if (v.at(last) == NULL) return false;
    if (strcmp(v.at(last), "<") == 0)
        return true;
    if (strcmp(v.at(last), ">") == 0)
        return true;
    if (strcmp(v.at(last), ">>") == 0)
        return true;
    if (strcmp(v.at(last), "<<<") == 0)
        return true;
    return false;
}

int exec_commands_iopip(vector<vector<char *> > &v_commands) {
    int lfds[2], rfds[2];
    for (unsigned i = 0; i < v_commands.size(); i++) {
        if (strcmp(v_commands.at(i).at(0),"exit") == 0)
            exit(0);

        string cmd = v_commands.at(i).at(0);
        if (strcmp(v_commands.at(i).at(0),"cd") != 0 && !has_slash(cmd)) {
            if (!find_path(cmd)) {
                return 1;
            }
            v_commands.at(i).at(0) = &cmd[0];
        }
        if (i != v_commands.size()-1) {
            if (-1 == pipe(rfds)) {
                perror("Error with pipe");
                return 1;
            }
        }
        int pid = fork();
        if (pid == -1) { //error with fork
            perror("error with fork");
            return 1;
        }
        else if (pid == 0) { //child process
            signal(SIGINT, SIG_DFL);
            if (hasString(v_commands.at(i), ">")) {
                char * file;
                for (unsigned j = 0; j < v_commands.at(i).size(); j++) {
                    if (v_commands.at(i).at(j) != NULL) {
                        if (strcmp(v_commands.at(i).at(j), ">") == 0) {
                            file = v_commands.at(i).at(j+1);
                            break;
                        }
                    }
                }
                if (-1 == (rfds[P_WRITE] = open(file, O_WRONLY|O_CREAT|O_TRUNC, S_IWUSR|S_IRUSR|S_IRGRP|S_IROTH))) {
                    perror("open failed");
                    exit(1);
                }
                if (-1 == dup2(rfds[P_WRITE],1)) {
                    perror("Error with dup2");
                    exit(1);
                }
                if (i < v_commands.size()-1 && -1 == close(rfds[P_READ])) {
                    perror("close");
                    exit(1);
                }
                if (-1 == close(rfds[P_WRITE])) {
                    perror("close");
                    exit(1);
                }
            }
            else if (hasString(v_commands.at(i), ">>")) {
                char * file;
                for (unsigned j = 0; j < v_commands.at(i).size(); j++) {
                    if (v_commands.at(i).at(j) != NULL) {
                        if (strcmp(v_commands.at(i).at(j), ">>") == 0) {
                            file = v_commands.at(i).at(j+1);
                            break;
                        }
                    }
                }
                if (-1 == (rfds[P_WRITE] = open(file, O_WRONLY|O_APPEND, S_IWUSR|S_IRUSR|S_IRGRP|S_IROTH))) {
                    perror("open failed");
                    exit(1);
                }
                if (-1 == dup2(rfds[P_WRITE],1)) {
                    perror("Error with dup2");
                    exit(1);
                }
                if (i < v_commands.size()-1 && -1 == close(rfds[P_READ])) {
                    perror("close");
                    exit(1);
                }
                if (-1 == close(rfds[P_WRITE])) {
                    perror("close");
                    exit(1);
                }
            }
            else if (i != v_commands.size()-1) {
                if (-1 == dup2(rfds[P_WRITE],1)) {
                    perror("Error with dup2");
                    exit(1);
                }
                if (-1 == close(rfds[P_READ])) {
                    perror("close");
                    exit(1);
                }
                if (-1 == close(rfds[P_WRITE])) {
                    perror("close");
                    exit(1);
                }
            }
            if (hasString(v_commands.at(i), "<")) {
                char * file;
                for (unsigned j = 0; j < v_commands.at(i).size(); j++) {
                    if (v_commands.at(i).at(j) != NULL) {
                        if (strcmp(v_commands.at(i).at(j), "<") == 0) {
                            file = v_commands.at(i).at(j+1);
                            break;
                        }
                    }
                }
                if (-1 == (lfds[P_READ] = open(file, O_RDONLY))) {
                    perror("open failed");
                    exit(1);
                } 
                if (-1 == dup2(lfds[P_READ],0)) {
                    perror("Error with close");
                    exit(1);
                }
                if (-1 == close(lfds[P_READ])) {
                    perror("close");
                    exit(1);
                }
                if (i != 0 && -1 == close(lfds[P_WRITE])) {
                    perror("close");
                    exit(1);
                }
            }
            else if (i != 0) {
                if (-1 == dup2(lfds[P_READ],0)) {
                    perror("Error with close");
                    exit(1);
                }
                if (-1 == close(lfds[P_READ])) {
                    perror("close");
                    exit(1);
                }
                if (-1 == close(lfds[P_WRITE])) {
                    perror("close");
                    exit(1);
                }
            }
            
            if (strcmp(v_commands.at(i).at(0),"cd") == 0) {
                int ret = change_dir(v_commands.at(i));
                exit(ret);
            }
            char **command_arr = &v_commands.at(i)[0];
            if (-1 == execv(command_arr[0], command_arr)) {
                perror("execv");
                exit(1);
            }
        }
        else { //parent process
            if (i != 0) {
                if (-1 == close(lfds[P_READ])) {
                    perror("close");
                    exit(1);
                }
                if (-1 == close(lfds[P_WRITE])) {
                    perror("close");
                    exit(1);
                }
            }
            if (i < v_commands.size()-1) {
                lfds[P_READ] = rfds[P_READ];
                lfds[P_WRITE] = rfds[P_WRITE];
            }
            //continue loop
        }
    }
    for (unsigned i = 0; i < v_commands.size(); i++) {
        int x;
        if (-1 == wait(&x)) { // parent function waits for child
            perror("wait");  // program exits if there is an error
            exit(1);
        }
    }
    return 0;
}

vector<char *> words(char* command) {
    vector<char *> v;
    char *str_token = strtok(command, " \t");
    while (str_token != NULL) {
        v.push_back(str_token);
        str_token = strtok(NULL, " \t");
    }
    //v.push_back(NULL);
    return v;
}

bool has_ios(const char *word) {
    for (unsigned i = 0; i < strlen(word); i++) {
        if (word[i] == '<')
            return true;
        if (word[i] == '>')
            return true;
    }
    return false;
}

bool allSpaces(char *line) {
    unsigned size = strlen(line);
    for (unsigned i = 0; i < size; i ++) {
        if (!(line[i] == ' ' || line[i] == '\t') ) {
            return false;
        }
    }
    return true;
}

void parse_commands_iopip(const vector<char *> &v_commands) {
    vector<vector<char *> > v_wordscmds;
    char in[] = "<";
    char ininin[] = "<<<";
    char out[] = ">";
    char outout[] = ">>";
    for (unsigned i = 0; i < v_commands.size(); i++) {
        if (allSpaces(v_commands.at(i))) {
            cout << ERROR_IOPIP << endl;
            return;
        }
        if (has_ios(v_commands.at(i))) {
            v_wordscmds.push_back(vector<char *>());
            string orig = v_commands.at(i);
            vector<char *> v;
            char *str_token = strtok(v_commands.at(i), "<>");
            unsigned i = 0;
            while (str_token != NULL) {
                v.push_back(str_token);
                if (i == 0)
                    v.push_back(NULL);
                i += strlen(str_token);
                if (i < orig.length()) {
                    if (orig.at(i) == '<') {
                        if (orig.at(i+1) == '<')  {
                            v.push_back(ininin);
                            i += 3;
                        }
                        else {
                            v.push_back(in);
                            i++;
                        }
                    }
                    else if (orig.at(i) == '>') {
                        if (orig.at(i+1) == '>') {
                            v.push_back(outout);
                            i += 2;
                        }
                        else {
                            v.push_back(out);
                            i++;
                        }
                    }
                }
                str_token = strtok(NULL, "<>");
            }
            for (unsigned j = 0; j < v.size(); j++) {
                if (v.at(j) == NULL) v_wordscmds.at(v_wordscmds.size()-1).push_back(NULL);
                else {
                    str_token = strtok(v.at(j), " \t");
                    while (str_token != NULL) {
                        v_wordscmds.at(v_wordscmds.size()-1).push_back(str_token);
                        str_token = strtok(NULL, " \t");
                    }
                }
            }
        }
        else {
            v_wordscmds.push_back(words(v_commands.at(i)));
            v_wordscmds.at(i).push_back(NULL);
        }
    }
    for (unsigned i = 0; i < v_wordscmds.size(); i++) {
        if (hasMultStrings(v_wordscmds.at(i), "<", "<<<")) {
            cout << ERROR_MULT_INPUT << endl;
            return;
        }
        if (hasMultStrings(v_wordscmds.at(i), ">", ">>")) {
            cout << ERROR_MULT_OUTPUT << endl;
            return;
        }
        if (last_is_iopip(v_wordscmds.at(i))) {
            cout << ERROR_IOPIP << endl;
            return;
        }
    }
    exec_commands_iopip(v_wordscmds);
}

void exec_commands(const vector<char *> &v_commands) {
    int x = 0;
    char connector[] = "&&";
    for (unsigned i = 0; i < v_commands.size(); i+=2) {
        if ( (x == 0 && strcmp(connector,"&&") == 0) || (x != 0 && strcmp(connector,"||") == 0) ) {
            vector<char *> command = words(v_commands.at(i));
            command.push_back(NULL);
            x = exec_command(command);
        }
        if ( i < v_commands.size() - 1 ) {
            strcpy(connector, v_commands.at(i+1));
        }
    }
}

void parse_commands(char *commands) {
    vector<char *> v_commands;
    unsigned size = strlen(commands);
    char andand[] = "&&";
    char oror[] = "||";
    unsigned i = 0;
    char *str_token = strtok(commands, "&|");
    while (str_token != NULL) {
        v_commands.push_back(str_token);
        if (allSpaces(v_commands.at(v_commands.size()-1))) {
            cerr << ERROR << endl;
            return;
        }
        i += strlen(str_token) + 1;
        if (i < size) {
            if (commands[i] == '&') v_commands.push_back(andand);
            else if (commands[i] == '|') v_commands.push_back(oror);
            i ++;
        }
        str_token = strtok(NULL, "&|");
    }
    exec_commands(v_commands);
}

bool isError_iopip(const char* line) {
    for (unsigned j = 0; j < strlen(line) -1; j++) {
        if (line[j] == '<') {
            if (j == 0)
                return true;
            else if (line[j-1] == '>' || line[j-1] == '|')
                return true;
            else if (line[j+1] == '>' || line[j+1] == '|')
                return true;
            else if (line[j-1] != '<')
                continue;
            else { // j must be greater than or equal to 2; line[j-1] == '<'
                if (line[j-2] != '<') return true;
                else if (line[j-3] == '<') return true;
            }
        }
        else if (line[j] == '>') {
            if (j == 0)
                return true;
            else if (line[j-1] == '|' || line[j+1] == '|')
                return true;
            else if (line[j-1] != '>') continue;
            else { // j must be greater than or equal to 2; line[j-1] == '>'
                if (line[j-2] == '>') return true;
            }
        }
        else if (j == 0 && line[j] == '|') 
            return true;
    }
    char last = line[strlen(line)-1];
    if (last == '<' || last == '>' || last == '|')
        return true;
    return false;
}

bool isError(const char* line) {
    for (unsigned j = 0; j < strlen(line) - 1; j++) {
        if (line[j] == ';') {
            if ( line[j+1] == ';')
                return true;
            else if (j == 0)  
                return true;
            else if (line[j-1] == '&' || line[j-1] == '|')
                return true;
            else if (line[j+1] == '&' || line[j+1] == '|')
                return true;
        }
        else if (line[j] == '&' || line[j] == '|') {
            if (j == 0) 
                return true;
            else if (line[j-1] == '&' || line[j-1] == '|') {
                if (line[j-1] != line[j]) {
                    return true;
                }
                else if (line[j+1] == '&' || line[j+1] == '|') {
                    return true;
                }
            }
            else if (line[j+1] != '&' && line[j+1] != '|') {
                return true;
            }
        }
    }
    char last = line[strlen(line)-1];
    if (last == '&' || last == '|')
        return true;
    return false;
}

void conn_iopip(const char * const line, bool &has_conn, bool &has_iopip) {
    for (int i = 0; line[i] != '\0'; i++) {
        if (line[i] == '&' || line[i] == ';') {
            has_conn = true;
        }
        else if (line[i] == '<' || line[i] == '>') {
            has_iopip = true;
        }
        else if (line[i] == '|') {
            if (line[i+1] == '|') { 
                has_conn = true;
                i++;
            }
            else has_iopip = true;
        }
        if (has_conn && has_iopip) break;
    }
}

int main() {

    signal(SIGINT, handler);

    char line[MAX_LINE_LENGTH]; 
    string curr_dir = getenv("PWD");
    if (&curr_dir[0] == NULL) {
        perror("error getting current working directory");
        exit(1);
    }
    cout << curr_dir << " $ " << flush;
    cin.getline(line, MAX_LINE_LENGTH);
    line[MAX_LINE_LENGTH-1] = '\0';
    if (strlen(line) == MAX_LINE_LENGTH - 1) {
        cerr << LENGTH_ERROR << endl;
        exit(1);
    }

    while (strcmp(line, "exit") != 0) {
        if (strlen(line) == 0) { /* do nothing */ }
        else if (line[0] != '#') {
            strcpy(line, strtok(line, "#"));
            bool has_conn = false, has_iopip = false;
            conn_iopip(line, has_conn, has_iopip);
            if (has_conn && has_iopip) cout << CONN_IOPIP_ERROR << endl;
            else if (has_iopip) {
                if (!isError_iopip(line)) {
                    vector<char *> v_commands;
                    char *string_token = strtok(line, "|");
                    while (string_token != NULL) {
                        v_commands.push_back(string_token);
                        string_token = strtok(NULL, "|");
                    }
                    parse_commands_iopip(v_commands);
                }
                else cout << ERROR_IOPIP << endl;
            }
            else {
                if (allSpaces(line)) { /* do nothing */ }
                else if (!isError(line)) {
                    vector<char *> v_commands;
                    char *string_token = strtok(line, ";");

                    while (string_token != NULL) {
                        v_commands.push_back(string_token);
                        string_token = strtok(NULL, ";");
                    }

                    for (unsigned i = 0; i < v_commands.size(); i++) {
                        //run the command between each semicolon
                        parse_commands(v_commands.at(i));
                    }
                }
            else cerr << ERROR << endl;
            }
        }

        curr_dir = getenv("PWD");
        if (&curr_dir[0] == NULL) {
            perror("error getting current working directory");
            exit(1);
        }
        cout << curr_dir << " $ ";
        cin.getline(line, MAX_LINE_LENGTH);

        line[MAX_LINE_LENGTH-1] = '\0';
        if (strlen(line) == MAX_LINE_LENGTH - 1) {
            cerr << LENGTH_ERROR << endl;
            exit(1);
        }
    }

    return 0;

}
