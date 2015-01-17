#include <iostream>
#include <string>
#include <string.h>
#include <vector>
using namespace std;

void exec_commands(char *commands) {
    
    }

int main() {
    char line[500]; 
    cout << "$ ";
    cin.getline(line, 500);

    while (strcmp(line, "exit") != 0) {
        vector<char *> v_commands;
        v_commands.push_back(strtok(line, ";"));
        //cout << v_commands.at(0) << endl;
        while (v_commands.at(v_commands.size()-1) != NULL) {
            v_commands.push_back(strtok(NULL, ";"));
        }
        cout << "$ ";
        cin.getline(line, 500);
    }
    return 0;
}
