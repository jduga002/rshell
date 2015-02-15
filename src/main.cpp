#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
using namespace std;

#define CONN_IOPIP_ERROR "rshell: error: rshell currently does not support the use of connectors and IO Redirection/Piping together"

const int MAX_LINE_LENGTH = 2048;
const char ERROR[] = "rshell: syntax error on connector: use ';', '&&', or '||'";
const char LENGTH_ERROR[] = "rshell: error on input: too many chars; exiting rshell";

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

int exec_command(char **command, int num_words) { 
    if (strcmp(command[0],"exit") == 0 && num_words == 1)
        exit(0);
    
    int x = 1;
    int pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(1);
    }
    else if (pid == 0) { // child process
        //execute command
        if (-1 == execvp(command[0], command)) {
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

char **words(char* command, int &size) {
    vector<char *> v;
    char *str_token = strtok(command, " \t");
    while (str_token != NULL) {
        v.push_back(str_token);
        str_token = strtok(NULL, " \t");
    }
    size = v.size();
    unsigned max_char_size = 0;
    for (unsigned i = 0; i < v.size(); i++) {
        if (strlen(v.at(i)) > max_char_size)
            max_char_size = strlen(v.at(i));
    }
    char **args = (char **)malloc(sizeof(char *) * size + 1);
    for (int i = 0; i < size; i++) {
        args[i] = (char*)malloc(sizeof(char) * max_char_size);
        strcpy(args[i],v.at(i));
    }
    args[size] = NULL;
    return args;
}

void exec_commands(const vector<char *> &v_commands) {
    int x = 0;
    char connector[] = "&&";
    for (unsigned i = 0; i < v_commands.size(); i+=2) {
        if ( (x == 0 && strcmp(connector,"&&") == 0) || (x != 0 && strcmp(connector,"||") == 0) ) {
            int size = 0;
            char **command = words(v_commands.at(i), size);
            x = exec_command(command, size);
            for (int j = 0; j < size; j++) {
                free(command[j]);
            }
            free(command);
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

bool isError(char* line) {
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
            bool has_conn = false, has_iopip = false;
            conn_iopip(line, has_conn, has_iopip);
            if (has_conn && has_iopip) cout << CONN_IOPIP_ERROR << endl;
            else if (has_iopip) { }
            else {
                strcpy(line, strtok(line, "#"));
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
