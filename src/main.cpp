#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
using namespace std;

void exec_command(char **command, int num_words) { 
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
}

void exec_commands(char *commands) {
    vector<char *> v_words;
    char *string_token;
    string_token = strtok(commands, " ");
    while (string_token != NULL) {
        v_words.push_back(string_token);
        string_token = strtok(NULL, " ");
    }
    
    for (unsigned i = 0; i < v_words.size(); i++) {
        if (strcmp(v_words.at(i), "&&") == 0) { 

        }
    }
}

int main() {
    char line[500]; 
    cout << "$ ";
    cin.getline(line, 500);

    while (strcmp(line, "exit") != 0) {
        vector<char *> v_commands;
        char *string_token = strtok(line, ";");

        while (string_token != NULL) {
            v_commands.push_back(string_token);
            string_token = strtok(NULL, ";");
        }

        for (unsigned i = 0; i < v_commands.size(); i++) {
            //run the command between each semicolon
            exec_commands(v_commands.at(i));
        }

        cout << "$ " << flush;
        cin.getline(line, 500);
    }

    return 0;

}
