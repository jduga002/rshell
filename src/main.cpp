#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdio.h>
using namespace std;

const int MAX_LINE_LENGTH = 2048;
const char ERROR[] = "rshell: syntax error on connector: use ';', '&&', or '||'";
const char ERROR_IOPIP[] = "rshell: syntax error on IO redirection: use '<', '<<<', '>', '>>'. or '|'";
const char LENGTH_ERROR[] = "rshell: error on input: too many chars; exiting rshell";
const char CONN_IOPIP_ERROR[] = "rshell: error: rshell currently does not support the use of connectors and IO Redirection/Piping together";

const int P_READ = 0;
const int P_WRITE = 1;

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

int exec_command(vector<char *> command) { 
    if (strcmp(command[0],"exit") == 0 && command.size() == 1)
        exit(0);
    
    int x = 1;
    int pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(1);
    }
    else if (pid == 0) { // child process
        //execute command
        char **command_arr = &command[0];
        if (-1 == execvp(command_arr[0], command_arr)) {
            perror("execvp");
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

int exec_commands_iopip(vector<vector<char *> > &v_commands, const vector<int *> &v_fds) {
    for (unsigned i = 0; i < v_commands.size(); i++) {
        if (i != v_commands.size()-1) {
            if (-1 == pipe(v_fds.at(i))) {
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
            if (i != v_commands.size()-1) {
                if (-1 == dup2(v_fds.at(i)[P_WRITE],1)) {
                    perror("Error with dup2");
                    exit(1);
                }
                if (-1 == close(v_fds.at(i)[P_READ])) {
                    perror("close");
                    exit(1);
                }
                if (-1 == close(v_fds.at(i)[P_WRITE])) {
                    perror("close");
                    exit(1);
                }

            }
            if (i != 0) {
                if (-1 == dup2(v_fds.at(i-1)[P_READ],0)) {
                    perror("Error with close");
                    exit(1);
                }
                if (-1 == close(v_fds.at(i-1)[P_READ])) {
                    perror("close");
                    exit(1);
                }
                if (-1 == close(v_fds.at(i-1)[P_WRITE])) {
                    perror("close");
                    exit(1);
                }
            }
            char **command_arr = &v_commands.at(i)[0];
            if (-1 == execvp(command_arr[0], command_arr)) {
                perror("execvp");
                exit(1);
            }
            exit(0);
        }
        else { //parent process
            if (i != 0) {
                if (-1 == close(v_fds.at(i-1)[P_READ])) {
                    perror("close");
                    exit(1);
                }
                if (-1 == close(v_fds.at(i-1)[P_WRITE])) {
                    perror("close");
                    exit(1);
                }
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
    /*for (unsigned i = 0; i < v_fds.size(); i++) {
        if (-1 == close(v_fds.at(i)[P_WRITE])) {
            perror("close");
            exit(1);
        }
        if (-1 == close(v_fds.at(i)[P_READ])) {
            perror("close");
            exit(1);
        }
    }*/
    return 0;
}

vector<char *> words(char* command) {
    vector<char *> v;
    char *str_token = strtok(command, " \t");
    while (str_token != NULL) {
        v.push_back(str_token);
        str_token = strtok(NULL, " \t");
    }
    v.push_back(NULL);
    return v;
}

vector<char *> words_iopip(char *command) {
    string orig = command;
    char in[] = "<";
    char ininin[] = "<<<";
    char out[] = ">";
    char outout[] = ">>";
    char *str_token = strtok(command, "<>");
    int i = strlen(str_token);
    vector<char *> v = words(str_token);
    v.push_back(NULL);
    if (orig.at(i) == '<') {
        if (orig.at(i+1) == '<') 
            v.push_back(ininin);
        else v.push_back(in);
    }
    else { //orig.at(i) == '>'
        if (orig.at(i+1) == '<') 
            v.push_back(outout);
        else v.push_back(out);
    }
    str_token = strtok(NULL, "<>");
    vector<char *> v2 = words(str_token);
    v.insert(v.end(), v2.begin(), v2.end());
    return v;

}

int num_ios(char *word) {
    int num = 0;
    for (unsigned i = 0; i < strlen(word); i++) {
        if (word[i] == '<') {
            num++;
            if (word[i+1] == '<')
                i += 2;
        }
        if (word[i] == '>') {
            num++;
            if (word[i+1] == '>')
                i++;
        }
    }
    return num;
}

void parse_commands_iopip(const vector<char *> &v_commands) {
    vector<vector<char *> > v_wordscmds;
    for (unsigned i = 0; i < v_commands.size(); i++) {
        int num = num_ios(v_commands.at(i));
        if (num > 1) {
            cout << ERROR_IOPIP << endl;
            return;
        }
        else if (num == 1)
            v_wordscmds.push_back(words_iopip(v_commands.at(i)));
        else v_wordscmds.push_back(words(v_commands.at(i)));
    }
    vector<int *> v_fds;
    int fd[2] = {0,0};
    for (unsigned i = 0; i < v_commands.size()-1; i++) {
        v_fds.push_back(fd);
    }
    /*for (unsigned i = 0; i < v_wordscmds.size(); i++) {
        for (unsigned j = 0; j < v_wordscmds.at(i).size(); j++) {
            if (v_wordscmds.at(i).at(j) != NULL) cerr << v_wordscmds.at(i).at(j) << endl;
            else cerr << "It is Null" << endl;
        }
        cerr << endl;
    }*/
    exec_commands_iopip(v_wordscmds, v_fds);
}

void exec_commands(const vector<char *> &v_commands) {
    int x = 0;
    char connector[] = "&&";
    for (unsigned i = 0; i < v_commands.size(); i+=2) {
        if ( (x == 0 && strcmp(connector,"&&") == 0) || (x != 0 && strcmp(connector,"||") == 0) ) {
            vector<char *> command = words(v_commands.at(i));
            x = exec_command(command);
        }
        if ( i < v_commands.size() - 1 ) {
            strcpy(connector, v_commands.at(i+1));
        }
    }
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

    char line[MAX_LINE_LENGTH]; 
    cerr << "$ ";
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

        cout << "$ " << flush;
        cin.getline(line, MAX_LINE_LENGTH);

        line[MAX_LINE_LENGTH-1] = '\0';
        if (strlen(line) == MAX_LINE_LENGTH - 1) {
            cerr << LENGTH_ERROR << endl;
            exit(1);
        }
    }

    return 0;

}
