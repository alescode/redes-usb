#include <map>
#include <string>
#include <iostream>

using namespace std;

void print_map(const map<string, int> m) {
    map<string, int>::const_iterator pos;
    cout << "{";
    for (pos = m.begin(); pos != m.end(); ++pos) {
        cout << "\"" << pos->first << "\": ";
        cout << pos->second << ", ";
    }
    cout << "}" << endl;
}
