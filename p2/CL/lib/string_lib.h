#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>

using namespace std;

const string trim(const string& pString);

string int_to_str(int n);
string double_to_str(double n);
int str_to_int(string s);
double str_to_double(string s);
char ** split(char string[], int * num, char * sep);
