#include "search_server.h"
#include "log_duration.h"
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <utility>
#include <numeric>
#include <execution>




using namespace std;



void SearchServer::AddDocument(int document_id, const std::string_view& document, DocumentStatus status,
    const vector<int>& ratings) {
    if ((document_id < 0) || (documents_.count(document_id) > 0)) {
        throw invalid_argument("Invalid document_id"s);
    }



    raw_document.emplace_back((document));


    const auto words = SplitIntoWordsNoStop(raw_document.back());

    const double inv_word_count = 1.0 / words.size();
    for (const std::string_view& word : words) {
        word_to_document_freqs_[word][document_id] += inv_word_count;
        doc_to_word_freqs_[document_id][word] += inv_word_count;
    }
    documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
    document_ids_.push_back(document_id);
}

vector<Document> SearchServer::FindTopDocuments(const std::string_view& raw_query, DocumentStatus status) const {
    return FindTopDocuments(
        raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
            return document_status == status;
        });
}

vector<Document> SearchServer::FindTopDocuments(const std::string_view& raw_query) const {
    return SearchServer::FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}




int SearchServer::GetDocumentCount() const {
    return document_ids_.size();
}


std::vector<int>::iterator SearchServer::begin() {
    return document_ids_.begin();
}


std::vector<int>::iterator SearchServer::end() {
    return document_ids_.end();
}

static const std::map<std::string_view, double> empty_map = { { ""s, 0.0 } };

const std::map<std::string_view, double>& SearchServer::GetWordFrequencies(int document_id) const {
    if (find(document_ids_.begin(), document_ids_.end(), document_id) != document_ids_.end()) {
        return doc_to_word_freqs_.at(document_id);
    }
    return empty_map;

}

void SearchServer::RemoveDocument(int document_id) {
    auto id = find(document_ids_.begin(), document_ids_.end(), document_id);
    if (id != document_ids_.end()) {
        documents_[document_id].status = DocumentStatus::REMOVED;
        document_ids_.erase(id);
        for (const auto [word, freq] : doc_to_word_freqs_.at(document_id)) {
            word_to_document_freqs_[word].erase(document_id);
        }
        doc_to_word_freqs_.erase(document_id);

    }
}




tuple<vector<string_view>, DocumentStatus> SearchServer::MatchDocument(const std::string_view& raw_query,
    int document_id) const {
    if (std::find(document_ids_.begin(), document_ids_.end(), document_id) == document_ids_.end()) {
        throw out_of_range("Invalid document_id - out of range"s);

    }


    const auto query = ParseQuery(raw_query);

    vector<std::string_view> matched_words;


    for (auto word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            return { matched_words, documents_.at(document_id).status };
        }
    }

    matched_words.reserve(query.plus_words.size());
    for (auto word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.push_back(word);
        }
    }
    return { matched_words, documents_.at(document_id).status };
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(const std::execution::sequenced_policy& p, const std::string_view& raw_query,
    int document_id) {
    return MatchDocument(raw_query, document_id);
}


std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(const std::execution::parallel_policy& p, const std::string_view& raw_query,
    int document_id) {

    if (std::find(document_ids_.begin(), document_ids_.end(), document_id) == document_ids_.end()) {
        throw out_of_range("Invalid document_id - out of range"s);

    }
    Query_parallel q_set = ParseQuery(p, raw_query);
    for (auto word : q_set.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            return {};
        }
    }


    std::vector<std::string_view> matched_words;

    matched_words.resize(q_set.plus_words.size());
    auto it_temp = std::copy_if(
        execution::par,
        q_set.plus_words.begin(),
        q_set.plus_words.end(),
        matched_words.begin(),
        [document_id, this](const std::string_view& word) {
            return (
                (word_to_document_freqs_.count(word) != 0) &&
                (word_to_document_freqs_.at(word).count(document_id))
                );
        }
    );
    matched_words.resize(std::distance(matched_words.begin(), it_temp));

    std::sort(execution::par, matched_words.begin(), matched_words.end());
    auto it = std::unique(execution::par, matched_words.begin(), matched_words.end());
    matched_words.resize(std::distance(matched_words.begin(), it));



    return { matched_words, documents_.at(document_id).status };
}




bool SearchServer::IsStopWord(const std::string_view& word) const {
    return stop_words_.count(word) > 0;
}



vector<string_view> SearchServer::SplitIntoWordsNoStop(const std::string_view& text) const {
    vector<std::string_view> words;
    for (const std::string_view& word : SplitIntoWords(text)) {
        if (!IsValidWord(word)) {
            throw invalid_argument("Word "/*sv + word + */" is invalid"s);
        }
        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    return words;
}




SearchServer::QueryWord SearchServer::ParseQueryWord(const std::string_view& text) const {
    if (text.empty()) {
        throw invalid_argument("Query word is empty"s);
    }
    std::string_view word = text;
    bool is_minus = false;
    if (word[0] == '-') {
        is_minus = true;
        word = word.substr(1);
    }
    if (word.empty() || word[0] == '-' || !IsValidWord(word)) {
        throw invalid_argument("Query word  is invalid");
    }

    return { word, is_minus, IsStopWord(word) };
}



SearchServer::Query SearchServer::ParseQuery(const std::string_view& text) const {
    SearchServer::Query result;
    for (const std::string_view& word : SplitIntoWords(text)) {
        const auto query_word = ParseQueryWord(word);
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                result.minus_words.insert(query_word.data);
            }
            else {
                result.plus_words.insert(query_word.data);
            }
        }
    }
    return result;
}

SearchServer::Query_parallel SearchServer::ParseQuery(const std::execution::parallel_policy& p, const std::string_view& text) const {

    SearchServer::Query_parallel result;
    for (const std::string_view& word : SplitIntoWords(text)) {
        const auto query_word = ParseQueryWord(word);
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                result.minus_words.push_back(query_word.data);
            }
            else {
                result.plus_words.push_back(query_word.data);
            }
        }
    }

    std::sort(execution::par, result.plus_words.begin(), result.plus_words.end());
    auto it = std::unique(execution::par, result.plus_words.begin(), result.plus_words.end());
    result.plus_words.resize(std::distance(result.plus_words.begin(), it));


    return result;


}


double SearchServer::ComputeWordInverseDocumentFreq(const std::string_view& word) const {
    return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}


int SearchServer::ComputeAverageRating(const vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }
    int rating_sum = std::accumulate(ratings.begin(), ratings.end(), 0);

    return rating_sum / static_cast<int>(ratings.size());
}

bool SearchServer::IsValidWord(const std::string_view& word) {
    // A valid word must not contain special characters
    return none_of(word.begin(), word.end(), [](char c) {
        return c >= '\0' && c < ' ';
        });
}


void SearchServer::RemoveDocument(const std::execution::sequenced_policy& p, int document_id) {

    auto id = find(std::execution::seq, document_ids_.begin(), document_ids_.end(), document_id);
    if (id != document_ids_.end()) {
        document_ids_.erase(id);
        documents_[document_id].status = DocumentStatus::REMOVED;
    }
}

void SearchServer::RemoveDocument(const std::execution::parallel_policy& p, int document_id) {
    auto id = find(std::execution::par, document_ids_.begin(), document_ids_.end(), document_id);
    if (id != document_ids_.end()) {
        document_ids_.erase(id);
        documents_[document_id].status = DocumentStatus::REMOVED;
    }
}


