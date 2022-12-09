#include "search_server.h"
#include "log_duration.h"
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <utility>
#include <numeric>




using namespace std;



void SearchServer::AddDocument(int document_id, const std::string& document, DocumentStatus status,
        const vector<int>& ratings) {
        if ((document_id < 0) || (documents_.count(document_id) > 0)) {
            throw invalid_argument("Invalid document_id"s);
        }
        const auto words = SplitIntoWordsNoStop(document);

        const double inv_word_count = 1.0 / words.size();
        for (const std::string& word : words) {
            word_to_document_freqs_[word][document_id] += inv_word_count;
            //std::map<int, std::map<std::string, double>> doc_to_word_freqs_;
            doc_to_word_freqs_[document_id][word] += inv_word_count;
        }
        documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
        document_ids_.push_back(document_id);
}

vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query, DocumentStatus status) const {
        return FindTopDocuments(
            raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
                return document_status == status;
            });
}

vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query) const {
        return SearchServer::FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

int SearchServer::GetDocumentCount() const {
        return documents_.size();
}

/*int SearchServer::GetDocumentId(int index) const {
        return document_ids_.at(index);
}*/

std::vector<int>::iterator SearchServer::begin() {
    return document_ids_.begin();
}


std::vector<int>::iterator SearchServer::end() {
    return document_ids_.end();
}

static const std::map<std::string, double> empty_map= { { ""s, 0.0 } };
    
const std::map<std::string, double>& SearchServer::GetWordFrequencies(int document_id) const{
    if (find(document_ids_.begin(), document_ids_.end(), document_id) != document_ids_.end()) {
        return doc_to_word_freqs_.at(document_id);
    }
    //const std::map<std::string, double> m = & new std::map<std::string, double>;
    return empty_map;

}

void SearchServer::RemoveDocument(int document_id) {
    auto id = find(document_ids_.begin(), document_ids_.end(), document_id);
    if (id != document_ids_.end()) {
        document_ids_.erase(id);
        documents_.erase(document_id);
    }
}

tuple<vector<string>, DocumentStatus> SearchServer::MatchDocument(const std::string& raw_query,
        int document_id) const {
        const auto query = ParseQuery(raw_query);

        vector<std::string> matched_words;
        for (const std::string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)) {
                matched_words.push_back(word);
            }
        }
        for (const std::string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)) {
                matched_words.clear();
                break;
            }
        }
        return { matched_words, documents_.at(document_id).status };
}


bool SearchServer::IsStopWord(const std::string& word) const {
        return stop_words_.count(word) > 0;
}



vector<string> SearchServer::SplitIntoWordsNoStop(const std::string& text) const {
        vector<std::string> words;
        for (const std::string& word : SplitIntoWords(text)) {
            if (!IsValidWord(word)) {
                throw invalid_argument("Word "s + word + " is invalid"s);
            }
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
}



    
SearchServer::QueryWord SearchServer::ParseQueryWord(const std::string& text) const {
        if (text.empty()) {
            throw invalid_argument("Query word is empty"s);
        }
        std::string word = text;
        bool is_minus = false;
        if (word[0] == '-') {
            is_minus = true;
            word = word.substr(1);
        }
        if (word.empty() || word[0] == '-' || !IsValidWord(word)) {
            throw invalid_argument("Query word "s + text + " is invalid");
        }

        return { word, is_minus, IsStopWord(word) };
}

    

SearchServer::Query SearchServer::ParseQuery(const std::string& text) const {
        SearchServer::Query result;
        for (const std::string& word : SplitIntoWords(text)) {
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

    // Existence required
double SearchServer::ComputeWordInverseDocumentFreq(const std::string& word) const {
        return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}


int SearchServer::ComputeAverageRating(const vector<int>& ratings) {
        if (ratings.empty()) {
            return 0;
        }
        int rating_sum = std::accumulate(ratings.begin(), ratings.end(), 0);

        return rating_sum / static_cast<int>(ratings.size());
}

bool SearchServer::IsValidWord(const std::string& word) {
        // A valid word must not contain special characters
        return none_of(word.begin(), word.end(), [](char c) {
            return c >= '\0' && c < ' ';
            });
}

