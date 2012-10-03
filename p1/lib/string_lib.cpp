// Taken from:
// http://stackoverflow.com/questions/1798112/removing-leading-and-trailing-spaces-from-a-string

#include <iostream>
#include <string>

const std::string trim(const std::string& pString,
        const std::string& pWhitespace = " \t")
{
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

const std::string reduce(const std::string& pString,
        const std::string& pFill = " ",
        const std::string& pWhitespace = " \t")
{
    // trim first
    std::string result(trim(pString, pWhitespace));

    // replace sub ranges
    size_t beginSpace = result.find_first_of(pWhitespace);
    while (beginSpace != std::string::npos)
    {
        const size_t endSpace =
            result.find_first_not_of(pWhitespace, beginSpace);
        const size_t range = endSpace - beginSpace;

        result.replace(beginSpace, range, pFill);

        const size_t newStart = beginSpace + pFill.length();
        beginSpace = result.find_first_of(pWhitespace, newStart);
    }

    return result;
}

/*int main(void)
{
    const std::string bleh = "    too much\t   \tspace\t\t\t  ";

    std::cout << "[" << trim(bleh) << "]" << std::endl;
    std::cout << "[" << reduce(bleh) << "]" << std::endl;
    std::cout << "[" << reduce(bleh, "-") << "]" << std::endl;
}*/
