#include "string_processing.h"




std::vector<std::string> SplitIntoWordsString(const std::string_view& text) {
    std::vector<std::string> words;
    std::string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        }
        else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}




std::vector<std::string_view> SplitIntoWords(const std::string_view& str) {
    std::vector<std::string_view> result;
    int64_t pos = str.find_first_not_of(" ");
    const int64_t pos_end = str.npos;
    while (pos != pos_end) {
        int64_t space = str.find(' ', pos);
        result.push_back(space == pos_end ? str.substr(pos) : str.substr(pos, space - pos));
        pos = str.find_first_not_of(" ", space);
    }

    return result;
}

