#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
using namespace std;

const int MAX_LINE_LENGTH = 2048;

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
    for (unsigned i = 0; i < v_commands.size(); i+=2) {
        int size = 0;
        char **command = words(v_commands.at(i), size);
        for (int j = 0; j < size; j++) {
            cerr << command[j] << endl;
        }
        int x = exec_command(command, size);
        for (int j = 0; j < size; j++) {
            cerr << "freeing at entry " << j << endl;
            free(command[j]);
        }
        free(command);
        while ((i+1 < v_commands.size()) && ((x != 0 && strcmp(v_commands.at(i+1), "&&") == 0)
         || (x == 0 && strcmp(v_commands.at(i+1), "||") == 0))) {
            i += 2;
        }
    }
}

void parse_commands(char *commands) {
    vector<char *> v_commands;
    unsigned size = strlen(commands);
    unsigned i = 0;
    char *str_token = strtok(commands, "&|");
    cerr << "commands is now " << commands << endl;
    cerr << "Entering while loop:" << endl;
    while (str_token != NULL) {
        cerr << "Assigning first command to v_commands" << endl;
        v_commands.push_back(str_token);
        cerr << v_commands.at(v_commands.size()-1) << endl;
        i += strlen(str_token) + 1;
        if (i < size) {
            char connector[3];
            
            cerr << "i is " << i << endl;
            connector[0] = commands[i];
            connector[1] = commands[i];
            connector[2] = '\0';
cerr << "connector is " << connector  << endl;
            i ++;
            v_commands.push_back(connector);
            cerr << v_commands.at(v_commands.size()-1) << endl;
        }
        str_token = strtok(NULL, "&|");
    }
    exec_commands(v_commands);
}

int main() {
    char hostName[MAX_LINE_LENGTH];
    getHost(hostName);
    char *username = getUsername();

    char line[MAX_LINE_LENGTH]; 
    cout << username << "@" << hostName << "$ ";
    cin.getline(line, MAX_LINE_LENGTH);
//    if (line[0] == '#') {
 //       line[0] = '\0';
 //   }
    strcpy(line,strtok(line, "#"));

    while (strcmp(line, "exit") != 0) {
        vector<char *> v_commands;
        char *string_token = strtok(line, ";");
        cerr << "string_token: " << string_token << endl;

        while (string_token != NULL) {
            v_commands.push_back(string_token);
            string_token = strtok(NULL, ";");
        }

        for (unsigned i = 0; i < v_commands.size(); i++) {
            //run the command between each semicolon
            cerr << v_commands.at(i) << endl;
            parse_commands(v_commands.at(i));
        }

        cout << username << "@" << hostName << "$ " << flush;
        cin.getline(line, MAX_LINE_LENGTH);
  //      if (line[0] == '#') { 
   //         line[0] = '\0';
    //    }
        strcpy(line, strtok(line, "#"));
    }

    return 0;

}
