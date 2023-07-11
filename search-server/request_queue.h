#pragma once
#include <vector>
#include <deque>
#include "search_server.h"
#include "document.h"




class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server) {
        search_server_ = &search_server;
    }

    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate);
    
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);
    std::vector<Document> AddFindRequest(const std::string& raw_query);
    int GetNoResultRequests() const;
private:
    struct QueryResult {
        int results_num=0;
        std::string query;
        
    };
    std::deque<QueryResult> requests_;
    const static int min_in_day_ = 1440;
    const SearchServer *search_server_;
    
};


template <typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {

    std::vector<Document> d;

    d = search_server_->FindTopDocuments(raw_query,
        document_predicate);
    if (requests_.size() == min_in_day_) { requests_.pop_front(); }
    QueryResult qr;
    qr.results_num = d.size();
    qr.query = raw_query;
    requests_.push_back(qr);

    return d;

}
