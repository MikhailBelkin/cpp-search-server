#include "process_queries.h"
#include "search_server.h"
#include "document.h"
#include <vector>
#include <list>
#include <string>
#include <algorithm>
#include <execution>

using namespace std;

std::vector<std::vector<Document>> ProcessQueries(
    const SearchServer& search_server,
    const std::vector<std::string>& queries) {

    std::vector<std::vector<Document>> result(queries.size());

    std::transform( execution::par,
                    queries.begin(), 
                    queries.end(), 
                    result.begin(),
                    [&search_server](std::string str) {
                        return search_server.FindTopDocuments(str);
                    }
                    );

    return result;

}

std::vector<Document> ProcessQueriesJoined(
    const SearchServer& search_server,
    const std::vector<std::string>& queries) {

    std::vector<std::vector<Document>> result(queries.size());

    std::transform(execution::par,
        queries.begin(),
        queries.end(),
        result.begin(),
        [&search_server](std::string str) {
            return search_server.FindTopDocuments(str);
        }
    );

   std::vector<Document> joined_doc;
    for (auto i : result) {
        for (auto j : i) {
            joined_doc.push_back(j);
        }
    }

   // std::transform(execution::par, result.begin(), result.end(), joined_doc.begin(), )
    return joined_doc;

}