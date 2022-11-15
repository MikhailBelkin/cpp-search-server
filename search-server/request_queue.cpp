#include "request_queue.h"
#include <string>



std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus status) {
        std::vector<Document> doc;

        doc = search_server_->FindTopDocuments(raw_query,
            status);
        if (requests_.size() == min_in_day_) { requests_.pop_front(); }
        QueryResult qr;
        qr.results_num = doc.size();
        qr.query = raw_query;
        requests_.push_back(qr);
        return doc;
        
    }
    std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query) {
        std::vector<Document> d;
        d = search_server_->FindTopDocuments(raw_query);
        if (requests_.size() == min_in_day_) { requests_.pop_front(); }
        QueryResult qr;
        qr.results_num = d.size();
        qr.query = raw_query;
        requests_.push_back(qr);
        return d;
    }
    int RequestQueue::GetNoResultRequests() const {
        int count_no_res=0;
        for (auto c : requests_) {
            if (c.results_num == 0) count_no_res++;
        }
        return count_no_res;
    }