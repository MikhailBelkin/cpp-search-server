#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <cmath>
using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

struct Document {
    int id;
    //int count_of_words;
    double relevance;
};
struct ParsedQuery{
    set<string> words;
    set<string> minus_words;
};

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document) {
        int word_doc_count=0;
        map <string, int> temp;
        for (const string word : SplitIntoWordsNoStop(document)){
            word_doc_count++;
            temp[word]++;
            //documents_[word].insert({document_id,0.0});
           //cout << "added id=" <<document_id<<" word added: "<<word<<" size of word="<<documents_[word].size()<<endl;
        }
        //documents_[word].[documents_id]=word_doc_count;
     //   cout<<word_doc_count<<" words in document"<<endl;
        //int i=0;
        for (const string word : SplitIntoWordsNoStop(document)){
            documents_[word].insert({document_id,(double)temp[word]/(double)word_doc_count});
            
      //     cout << "111 added id=" <<document_id<<" word added: "<<word<<" TF="<<(double)temp[word]/(double)word_doc_count<<endl;
        }
        
        doc_count_++;
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        ParsedQuery query_words = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query_words);

        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                 return lhs.relevance > rhs.relevance;
             });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

private:
    /*struct DocumentContent {
        int id = 0;
        vector<string> words;
    };*/

    map<string, map<int, double>> documents_;

    set<string> stop_words_;
    int doc_count_=0;
    

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    ParsedQuery ParseQuery(const string& text) const {
        ParsedQuery query_words;
        for (const string& word : SplitIntoWordsNoStop(text)) {
             if (word[0]=='-') {
                 query_words.minus_words.insert(word.substr(1));
                 //cout << "Minus word:" << word.substr(1) << endl;
             }
            else{
             query_words.words.insert(word);
            }
        }
        return query_words;
    }

    vector<Document> FindAllDocuments(ParsedQuery& query_words) const {
        vector<Document> matched_documents;
        //map <int, int> relevance;
            
        if (!query_words.words.empty()) {
           
        
            for ( auto i: MatchDocument(documents_, query_words, doc_count_)){
              matched_documents.push_back({i.first, (double)i.second});
            }
            
        }
        return matched_documents;
       
    }

    static map<int, double> MatchDocument(const map<string, map <int, double>> & content, const ParsedQuery& query_words, int doc_count) {
        //if (query_words.words.empty()) {
        //    return {0, 0};
        //}
        
        set<string> matched_words;
        map <int, double> rel_doc;
        double idf=0;
        
        for (const string& word : query_words.words) {
            //cout << "Finding word=" <<word<<endl;
            int found_doc=0;
            if (content.find(word)!=content.end()){
            //    for ( auto[i, j]: content.at(word)){
                    //rel_doc[i]=j;
                  //cout << "Finding id=" <<i<<" word found: "<<word<<" rel="<<rel_doc[i]<<endl;  
             //   found_doc++;    
             //   }
            found_doc=content.at(word).size();
            idf=log((double)doc_count/found_doc);
            for ( auto[i, j]: content.at(word)){
                    rel_doc[i]+=j*idf;
                  //cout << "Finding id=" <<i<<" word found: "<<word<<" TF="<<j<<" IDF="<<idf<< " TF-IDF="<<rel_doc[i]<<endl;  
                    
                }    
           // cout<<"word="<<word<<" count of doc with="<<found_doc<<" IFD="<<idf<<" TF-ID="<< idf<<endl;        
                
            }
        }
        for (const string& word : query_words.minus_words) {
                if (content.find(word)!=content.end()){
                    for ( auto[i, j]: content.at(word)){
                        rel_doc.erase(i);
                    }
                }
        }
        return rel_doc;    
        
    }
};

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());

    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }

    return search_server;
}

int main() {
    const SearchServer search_server = CreateSearchServer();

    const string query = ReadLine();
    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", "
             << "relevance = "s << relevance << " }"s << endl;
    }
}