#pragma once
#include <vector>
#include <iostream>
#include "document.h"

template <typename Iterator>
class IteratorRange {
    Iterator first_;
    Iterator last_;

    int size_;
public:
    IteratorRange(Iterator first, Iterator last, int size) {
        first_ = first;
        last_ = last;

        size_ = size;
    }
    auto begin() const {
        return first_;
    }
    auto end() const {
        return last_;
    }
    int  size() const {
        return size_;
    }


};


template <typename Iterator>
class Paginator {
    std::vector<IteratorRange<Iterator>> pages_;


    size_t page_size_;
public:
    Paginator(Iterator first, Iterator last, size_t page_size) {
        int i = 0;
        Iterator begin_page;
        Iterator end_page;
        begin_page = first;
        for (auto it = first; it != last; it++) {
            if (i < page_size) {
                i++;
            }
            else {
                end_page = it - 1;
                pages_.push_back(IteratorRange<Iterator>(begin_page, end_page, i));
                i = 1;
                begin_page = it;
            }
        }
        if (begin_page != last) {
            end_page = last - 1;
            pages_.push_back(IteratorRange<Iterator>(begin_page, end_page, i));
        }
        //it_ = pages_.begin();
        page_size_ = page_size;
    };
    auto begin() const {
        return pages_.begin();
    }
    auto end() const {
        //return pages_.at(pages_.size()-1);
        //return pages_.end();
        return pages_.end();
    }
    /*vector<IteratorRange>::iterator next() {
        return pages_.begin();
    }*/


};



template <typename Container>
auto Paginate(Container& c, size_t page_size) {

    return Paginator(begin(c), end(c), page_size);
}



template <typename Iterator>
std::ostream& operator<<(std::ostream& out, const IteratorRange<Iterator> pages) {
    for (auto page = pages.begin(); page <= pages.end(); page++) {
        out << *page;
    }

    return out;


}


std::ostream& operator<<(std::ostream& out, const Document document) {
    return out << "{ "s
        << "document_id = "s << document.id << ", "s
        << "relevance = "s << document.relevance << ", "s
        << "rating = "s << document.rating << " }"s;
}
