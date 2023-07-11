#include "remove_duplicates.h"
#include "search_server.h"

#include <map>
#include <set>
#include <iostream>

void RemoveDuplicates(SearchServer& s) {
    std::map<std::set<std::string_view>, std::vector<int>> docs_info;
    std::set<int> duplicate_ids;


    for (auto id : s) {
        std::set<std::string_view> read_words;
        auto info = s.GetWordFrequencies(id);
        for (auto s : info) {
            read_words.insert(s.first);
        }
        docs_info[read_words].push_back(id);
        if (docs_info[read_words].size() > 1) duplicate_ids.insert(id);
    }

    for (auto i : duplicate_ids) {
            std::cout << "Found duplicate document id " << i << std::endl;
            s.RemoveDocument(i);
    }
}



