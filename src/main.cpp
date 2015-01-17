#include <iostream>
#include <string>
#include <string.h>
#include <vector>
using namespace std;

int main() {
    char line[500]; 
    cout << "$ ";
    cin.getline(line, 500);

    while (strcmp(line, "exit") != 0) {
        vector<char *> commands;
        cout << "$ ";
        cin.getline(line, 500);
    }
    return 0;
}
