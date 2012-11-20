#include "string_lib.h"

const string trim(const string& pString) {
    // Tomado de:
    // http://stackoverflow.com/questions/1798112/removing-leading-and-trailing-spaces-from-a-string
    const string pWhitespace = " \t";
    const size_t beginStr = pString.find_first_not_of(pWhitespace);
    if (beginStr == string::npos)
    {
        // no content
        return "";
    }

    const size_t endStr = pString.find_last_not_of(pWhitespace);
    const size_t range = endStr - beginStr + 1;

    return pString.substr(beginStr, range);
}

string int_to_str(int n) {
    stringstream s;
    s << n;
    return s.str();
}

string double_to_str(double n) {
    stringstream s;
    s << n;
    return s.str();
}

int str_to_int(string s) {
    int n;
    istringstream ss(s);
    ss >> n;
    return n;
}

double str_to_double(string s) {
    double n;
    istringstream ss(s);
    ss >> n;
    return n;
}

// Tomado de
// http://cboard.cprogramming.com/c-programming/88942-split-string-function.html
char  ** split(char string[], int * num, char * sep) {
    char * pch;
    char ** out = 0;
    int i = 0;
    pch = strtok (string, sep );
    while (pch != 0 ) {
        out = (char**) realloc(out, (i + 1) * sizeof(char *));
        out[i] = (char*) malloc(strlen(pch) + 1);
        strcpy(out[i], pch);
        ++i;
        pch = strtok (NULL, sep);
    }
    *num = i;
    return out;
}
