#include <iostream>
#include <string>
using namespace std;

int main() {
    string input;
    cout << "Zayden ready (offline mode). Ask me anything.\n";
    while (true) {
        cout << "> ";
        getline(cin, input);
        if (input == "exit") break;
        cout << "Zayden: I am the unified council. You said: " << input << "\n";
    }
    return 0;
}
