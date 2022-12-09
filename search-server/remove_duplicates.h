#pragma once
#include "remove_duplicates.h"
#include "search_server.h"
#include <map>
#include <set>
#include <iostream>

void RemoveDuplicates(SearchServer& s) {
    std::map<int, std::set<std::string>> docs_info;
    for (auto id : s) {
        auto info = s.GetWordFrequencies(id);
        for (auto s : info) {
            docs_info[id].insert(s.first);
        }
    }
    for (auto i : docs_info) {
        for (auto j : docs_info) {
            if (i.first != j.first && i.second == j.second && !j.second.empty()) {
                std::cout << "Found duplicate document id " << j.first << std::endl;
                s.RemoveDocument(j.first);
                docs_info[j.first].clear();
                /*for (auto s : docs_info[j.first]) {
                    std::cout << "Word is " << s << std::endl;
                    docs_info[j.first].erase(s);
                }*/

            }
        }
    }

}