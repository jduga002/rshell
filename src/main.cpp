#include <iostream>
#include <string>
#include <string.h>
#include <vector>
using namespace std;

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
