#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
using namespace std;

const int MAX_LINE_LENGTH = 500;

char * getUsername() {
    char *username;
    username = getlogin();
    if (!username) {
        perror("getlogin() error");
        strcpy(username, "user");
    }
    return username;
}

void getHost(char *hostName) {
    if (-1 == gethostname(hostName, MAX_LINE_LENGTH - 1) ) { 
        perror("gethostname() error");
        strcpy(hostName, "hostName");
    }
}

bool exec_command(char **command, int num_words) { 
    if (strcmp(command[0],"exit") == 0 && num_words == 1)
        exit(0);
    
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
        if (-1 == wait(0)) { // parent function waits for child
            perror("wait");  // program exits if there is an error
            exit(1);
        }
    }
    return true;
}

void exec_commands(char *commands) {
    vector<char *> v_commands;
    char *ptr = commands;
    unsigned size = strlen(commands);
    unsigned i = 0;
    unsigned j = 0;
    cerr << "Entering while loop:" << endl;
    while (i + j < size) {
        i = strcspn(ptr, "&|");
        cerr << "  i is " << i << endl;
        if (i + j == size || i + j == size - 1) {
            cerr << "    Reached end of line." << endl;
            v_commands.push_back(ptr);
            break;
        }
        else if (ptr[i+j] == ptr[i+j+1]) {
            cerr << "    " << ptr[i+j] << ptr[i+j] << " found" << endl;
            char *string_copy;
            cerr << "Char * string_copy, made; will copy ptr to it." << endl;
            strncpy(string_copy, ptr, i+j);
            cerr << "    Going to append null char." << endl;
            string_copy[i + j + 1] = '\0';
            v_commands.push_back(string_copy);
            ptr += i + j + 2;
            i = 0; j = 0;
            size = strlen(ptr);
        }
        else {
            ptr += i + j + 1;
            j = i;
            i = 0;
        }
    for (unsigned k = 0; k < v_commands.size(); k ++)
        cout << v_commands.at(k) << endl;
    }
/*
    char *string_token;
    string_token = strtok(commands, " ");
    while (string_token != NULL) {
        v_words.push_back(string_token);
        string_token = strtok(NULL, " ");
    }
    
    int size = 0;
    char **command = NULL;
    for (unsigned i = 0; i < v_words.size(); i++) {
        if (strcmp(v_words.at(i), "&&") == 0) { 
            bool success = exec_command(command, size);
            if (!success) {
                
            }
        }
    }*/
}

int main() {
    char hostName[MAX_LINE_LENGTH];
    getHost(hostName);
    char *username = getUsername();

    char line[MAX_LINE_LENGTH]; 
    cout << username << "@" << hostName << "$ ";
    cin.getline(line, MAX_LINE_LENGTH);
    if (line[0] == '#') {
        line[0] = '\0';
    }
    strcpy(line,strtok(line, "#"));

    while (strcmp(line, "exit") != 0) {
        vector<char *> v_commands;
        char *string_token = strtok(line, ";");

        while (string_token != NULL) {
            v_commands.push_back(string_token);
            string_token = strtok(NULL, ";");
        }

        for (unsigned i = 0; i < v_commands.size(); i++) {
            //run the command between each semicolon
            cerr << v_commands.at(i) << endl;
            exec_commands(v_commands.at(i));
        }

        cout << username << "@" << hostName << "$ " << flush;
        cin.getline(line, MAX_LINE_LENGTH);
        if (line[0] == '#') { 
            line[0] = '\0';
        }
        strcpy(line, strtok(line, "#"));
    }

    return 0;

}
