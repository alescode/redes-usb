#include "string_lib.h"

const std::string trim(const std::string& pString) {
    // Tomado de:
    // http://stackoverflow.com/questions/1798112/removing-leading-and-trailing-spaces-from-a-string
    const std::string pWhitespace = " \t";
    const size_t beginStr = pString.find_first_not_of(pWhitespace);
    if (beginStr == std::string::npos)
    {
        // no content
        return "";
    }

    const size_t endStr = pString.find_last_not_of(pWhitespace);
    const size_t range = endStr - beginStr + 1;

    return pString.substr(beginStr, range);
}
