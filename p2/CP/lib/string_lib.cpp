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

inline string int_to_str(int n) {
    stringstream s;
    s << n;
    return s.str();
}

inline string double_to_str(double n) {
    stringstream s;
    s << n;
    return s.str();
}

inline int str_to_int(string s) {
    int n;
    istringstream ss(s);
    ss >> n;
    return n;
}

inline int str_to_double(string s) {
    double n;
    istringstream ss(s);
    ss >> n;
    return n;
}
