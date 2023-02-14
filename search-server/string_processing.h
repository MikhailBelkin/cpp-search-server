#pragma once
#include <string>
#include <vector>
#include <set>
#include<string_view>



std::vector<std::string_view> SplitIntoWords(const std::string_view& text);
std::vector<std::string> SplitIntoWordsString(const std::string_view& text);


template <typename StringContainer>
std::set < std::string, std::less<>> MakeUniqueNonEmptyStrings(const StringContainer& strings) {
    std::set < std::string, std::less<>> non_empty_strings;

    for (const auto str : strings) {
        if (!str.empty()) {
            non_empty_strings.insert(str);
        }
    }
    return non_empty_strings;
}
