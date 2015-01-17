#include <iostream>
#include <string>
#include <vector>
using namespace std;

int main() {
    string line;
    cout << "$ ";
    getline(cin, line);

    while (line != "exit") {
        const char * cLine = line.c_str();
        
        cout << "$ ";
        getline(cin, line);
    }
    return 0;
}
