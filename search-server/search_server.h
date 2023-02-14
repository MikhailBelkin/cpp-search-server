#pragma once
#include <string>
#include <vector>
#include <tuple>
#include <set>
#include <map>
#include <algorithm>
#include <stdexcept>

#include "string_processing.h"
#include "document.h"
#include "read_input_functions.h"
#include <execution>
#include <iostream>
#include <list>
#include <deque>
#include "concurrent_map.h"

//#include "paginator.h"

using namespace std::literals::string_literals;

const int MAX_RESULT_DOCUMENT_COUNT = 5;
const double MIN_RELEVANCE = 1e-6;

class SearchServer {
public:

    template <typename StringContainer>
    explicit SearchServer(const StringContainer& stop_words)
        : stop_words_(MakeUniqueNonEmptyStrings(stop_words)) {

        if (!all_of(stop_words_.begin(), stop_words_.end(), IsValidWord)) {
            throw std::invalid_argument("Some of stop words are invalid"s);
        }
    }

    SearchServer(const std::string& stop_words_text)
        : SearchServer(
            SplitIntoWordsString(stop_words_text))
    {
    }

    SearchServer(const std::string_view& stop_words_text)
        : SearchServer(
            SplitIntoWordsString(stop_words_text))
    {
    }

    void AddDocument(int document_id, const std::string_view& document, DocumentStatus status,
        const std::vector<int>& ratings);

    template <typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(const std::string_view& raw_query,
        DocumentPredicate document_predicate) const;

    template <typename DocumentPredicate, class ExecutionPolicy>
    std::vector<Document> FindTopDocuments(ExecutionPolicy&& p, const std::string_view& raw_query,
        DocumentPredicate document_predicate) const;

    std::vector<Document> FindTopDocuments(const std::string_view& raw_query, DocumentStatus status) const;

    template <class ExecutionPolicy>
    std::vector<Document> FindTopDocuments(ExecutionPolicy&& p, const std::string_view& raw_query, DocumentStatus status) const;


    std::vector<Document> FindTopDocuments(const std::string_view& raw_query) const;

    template <class ExecutionPolicy>
    std::vector<Document> FindTopDocuments(ExecutionPolicy&& p, const std::string_view& raw_query) const;

    int GetDocumentCount() const;

    //int GetDocumentId(int index) const;

    std::vector<int>::iterator begin();

    std::vector<int>::iterator end();

    const std::map<std::string_view, double>& GetWordFrequencies(int document_id) const;
    void RemoveDocument(int document_id);

    /*template <class ExecutionPolicy>
    void RemoveDocument(ExecutionPolicy&& p, int document_id);*/
    void RemoveDocument(const std::execution::sequenced_policy& p, int document_id);
    void RemoveDocument(const std::execution::parallel_policy& p, int document_id);
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(const std::string_view& raw_query, int document_id) const;

    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(const std::execution::sequenced_policy& p, const  std::string_view& raw_query,
        int document_id);
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(const std::execution::parallel_policy& p, const  std::string_view& raw_query,
        int document_id);

private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };

    struct QueryWord {
        std::string_view data;
        bool is_minus;
        bool is_stop;
    };

    struct Query {
        std::set < std::string_view, std::less<>> plus_words;
        std::set < std::string_view, std::less<>> minus_words;
    };


    struct Query_p {
        std::vector<std::string_view> plus_words;
        std::vector<std::string_view> minus_words;
    };

    const std::set < std::string, std::less<>> stop_words_;
    std::map<std::string_view, std::map<int, double>> word_to_document_freqs_;
    std::map<int, std::map<std::string_view, double>> doc_to_word_freqs_;
    std::map<int, DocumentData> documents_;
    std::vector<int> document_ids_;
    std::deque<std::string> raw_document;

    bool IsStopWord(const std::string_view& word) const;

    static bool IsValidWord(const std::string_view& word);

    std::vector<std::string_view> SplitIntoWordsNoStop(const std::string_view& text) const;

    static int ComputeAverageRating(const std::vector<int>& ratings);


    QueryWord ParseQueryWord(const std::string_view& text) const;


    Query ParseQuery(const std::string_view& text) const;
    Query_p ParseQuery(const std::execution::parallel_policy& p, const std::string_view& text) const;


    double ComputeWordInverseDocumentFreq(const std::string_view& word) const;

    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(const Query& query,
        DocumentPredicate document_predicate) const;

    template <typename DocumentPredicate, class ExecutionPolicy>
    std::vector<Document> FindAllDocuments(ExecutionPolicy&& p, const SearchServer::Query_p& query,
        DocumentPredicate document_predicate) const;

};






template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(const std::string_view& raw_query,
    DocumentPredicate document_predicate) const {
    const auto query = SearchServer::ParseQuery(raw_query);

    auto matched_documents = SearchServer::FindAllDocuments(query, document_predicate);

    sort(matched_documents.begin(), matched_documents.end(),
        [](const Document& lhs, const Document& rhs) {
            if (std::abs(lhs.relevance - rhs.relevance) < MIN_RELEVANCE) {
                return lhs.rating > rhs.rating;
            }
            else {
                return lhs.relevance > rhs.relevance;
            }
        });
    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }

    return matched_documents;
}




template <typename DocumentPredicate, class ExecutionPolicy>
std::vector<Document> SearchServer::FindTopDocuments(ExecutionPolicy&& p, const std::string_view& raw_query,
    DocumentPredicate document_predicate) const {
    const auto query = SearchServer::ParseQuery(execution::par, raw_query);

    auto matched_documents = SearchServer::FindAllDocuments(p, query, document_predicate);

    sort(p, matched_documents.begin(), matched_documents.end(),
        [](const Document& lhs, const Document& rhs) {
            if (std::abs(lhs.relevance - rhs.relevance) < MIN_RELEVANCE) {
                return lhs.rating > rhs.rating;
            }
            else {
                return lhs.relevance > rhs.relevance;
            }
        });
    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }

    return matched_documents;
}


template <class ExecutionPolicy>
std::vector<Document> SearchServer::FindTopDocuments(ExecutionPolicy&& p, const std::string_view& raw_query, DocumentStatus status) const {
    return FindTopDocuments(p,
        raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
            return document_status == status;
        });
}

template <class ExecutionPolicy>
std::vector<Document> SearchServer::FindTopDocuments(ExecutionPolicy&& p, const std::string_view& raw_query) const {
    return SearchServer::FindTopDocuments(p, raw_query, DocumentStatus::ACTUAL);
}

template <typename DocumentPredicate, class ExecutionPolicy>
std::vector<Document> SearchServer::FindAllDocuments(ExecutionPolicy&& p, const SearchServer::Query_p& query,
    DocumentPredicate document_predicate) const {
    ConcurrentMap<int, double> document_to_relevance(150);
    /*for (auto word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);*/
        /*for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {

                const auto& document_data = documents_.at(document_id);
                if (document_predicate(document_id, document_data.status, document_data.rating)) {
                    document_to_relevance[document_id] += term_freq * inverse_document_freq;

            }
        }*/

    for_each(p, query.plus_words.begin(), query.plus_words.end(), [&](auto word) {
        if (word_to_document_freqs_.count(word) != 0) {
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
            for_each(p, word_to_document_freqs_.at(word).begin(), word_to_document_freqs_.at(word).end(),
                [/*this, document_predicate, &document_to_relevance, inverse_document_freq*/ &](const auto element) {
                    const auto& document_data = documents_.at(element.first);
                    if (document_predicate(element.first, document_data.status, document_data.rating)) {
                        //double temp = document_to_relevance[document_id].ref_to_value;
                        document_to_relevance[element.first].ref_to_value += element.second * inverse_document_freq;

                    }

                }
            );
        }
    });
    /*for (auto word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
            document_to_relevance.erase(document_id);
        }
    }*/

    for_each(p, query.minus_words.begin(), query.minus_words.end(), [&](auto word) {
        if (word_to_document_freqs_.count(word) != 0) {
            for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
                document_to_relevance.erase(document_id);
            }
        }
        });


    std::vector<Document> matched_documents;
    auto doc_to_rel = document_to_relevance.BuildOrdinaryMap();
    for (const auto [document_id, relevance] : doc_to_rel) {
        matched_documents.push_back(
            { document_id, relevance, documents_.at(document_id).rating });
    }

    return matched_documents;
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(const SearchServer::Query& query,
    DocumentPredicate document_predicate) const {
    std::map<int, double> document_to_relevance;
    for (auto word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
        for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
            //if (documents_.at(document_id).status != DocumentStatus::REMOVED) {
            const auto& document_data = documents_.at(document_id);
            if (document_predicate(document_id, document_data.status, document_data.rating)) {
                document_to_relevance[document_id] += term_freq * inverse_document_freq;
                //   }
            }
        }
    }

    for (auto word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
            document_to_relevance.erase(document_id);
        }
    }

    std::vector<Document> matched_documents;
    for (const auto [document_id, relevance] : document_to_relevance) {
        matched_documents.push_back(
            { document_id, relevance, documents_.at(document_id).rating });
    }
    return matched_documents;
}




/*template <class ExecutionPolicy>
void SearchServer::RemoveDocument(ExecutionPolicy&& p, int document_id) {


    if (std::is_execution_policy_v < std::decay_t < ExecutionPolicy >>) {
        std::cout << "Exec is true" << std::endl;

    }

    auto id = find(p, document_ids_.begin(), document_ids_.end(), document_id);

    if (id != document_ids_.end()) {
        //document_ids_[document_id]=static_cast<int>(DocumentStatus::REMOVED);
        //std::map<int, DocumentData> documents_;
        documents_[document_id].status = DocumentStatus::REMOVED;
        document_ids_.erase(id);
       // documents_.erase(document_id);
       //for (const auto [word, freq] : doc_to_word_freqs_.at(document_id)) {
       //     word_to_document_freqs_[word].erase(document_id);
       // }
       // doc_to_word_freqs_.erase(document_id);
       //
    }
}

*/





