#pragma once
#include "string_processing.h"
#include "document.h"
#include <vector>
#include <map>
#include <set>
#include <string>
#include <algorithm>
#include <cmath>

const int MAX_RESULT_DOCUMENT_COUNT = 5;
const double DOUBLE_ACCURACY = 1e-6;

class SearchServer
{
public:
    template <typename StopWords>
    explicit SearchServer(const StopWords& stop_words);

    explicit SearchServer(const  std::string& raw_stop_words)
        : SearchServer(SplitIntoWords(raw_stop_words)) {}

    void AddDocument(int document_id, const  std::string& document, DocumentStatus status,
        const  std::vector<int>& ratings);

    static bool IsValidWord(const  std::string& text);
   
    static bool IsValidChar(const char& c);    

    std::vector<Document> FindTopDocuments(const  std::string& raw_query) const;
  
    std::vector<Document> FindTopDocuments(const  std::string& raw_query,
        DocumentStatus status) const;

    template <typename Filtr>
    std::vector<Document> FindTopDocuments(const  std::string& raw_query,
        Filtr filtr) const;

    int GetDocumentCount() const;

    std::tuple< std::vector< std::string>, DocumentStatus> MatchDocument(const  std::string& raw_query,
        int document_id) const;

    std::set<int>::iterator begin();

    std::set<int>::iterator end();

    //ìåòîä ïîëó÷åíèÿ ÷àñòîòû ñëîâ ïî id äîêóìåíòà
    const std::map<std::string, double>& GetWordFrequencies(int document_id) const;

    //óäàëåíèå äîêóìåíòà ïî ID
    void RemoveDocument(int document_id);

private:
    struct DocumentData
    {
        DocumentData() : rating(0), status(DocumentStatus::ACTUAL) {};
        DocumentData(int rate, DocumentStatus status)
            : rating(rate), status(status) {};
        int rating;
        DocumentStatus status;
    };

    std::set<int> document_ids_; //ñîäåðæèò ID äîêóìåíòîâ   
    std::set<std::string> stop_words_; //Áàçà ñòîï-ñëîâ
    std::map<std::string, std::map<int, double>> word_to_document_freqs_;  //ñëîâàðü, ãäå äëÿ êàæäîãî ñëîâà íà ñåðâåðå õðàíèòüñÿ ID äîêóìåíòîâ, â êîòîðûõ ñëîâî âñòðå÷àåòñÿ è IDF ñëîâà äëÿ äîêóìåíòà
    std::map<int, DocumentData> documents_; //ñëîâàðü ID äîêóìåíòîâ ñ èíôîðìàöèåé î ðåéòèíãå è ñòàòóñå
    std::map<std::set<std::string>, std::set<int>> word_sets_to_documents_;
    
    bool IsStopWord(const  std::string& word) const;

    bool IsDocumentID(const int doc_id) const;

    std::vector<std::string> SplitIntoWordsNoStop(const std::string& text) const;

    static int ComputeAverageRating(const  std::vector<int>& ratings);

    struct QueryWord
    {
        std::string data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(std::string text) const;

    struct Query
    {
        std::set<std::string> plus_words;
        std::set<std::string> minus_words;
    };

    Query ParseQuery(const std::string& text) const;

    double ComputeWordInverseDocumentFreq(const std::string& word) const;

    template <typename Filtr>
    std::vector<Document> FindAllDocuments(const Query& query, Filtr filtr) const;
};

template <typename Filtr>
std::vector<Document> SearchServer::FindTopDocuments(const  std::string& raw_query,
    Filtr filtr) const
{
    Query query = ParseQuery(raw_query);
    auto matched_documents = FindAllDocuments(query, filtr);
    sort(matched_documents.begin(), matched_documents.end(),
        [](const Document& lhs, const Document& rhs)
        {
            if (std::abs(lhs.relevance - rhs.relevance) < DOUBLE_ACCURACY)
            {
                return lhs.rating > rhs.rating;
            }
            else {
                return lhs.relevance > rhs.relevance;
            }
        });
    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT)
    {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }
    return matched_documents;
}

template <typename Filtr>
std::vector<Document> SearchServer::FindAllDocuments(const Query& query, Filtr filtr) const {
    std::map<int, double> document_to_relevance;
    for (const std::string& word : query.plus_words)
    {
        if (word_to_document_freqs_.count(word) == 0)
        {
            continue;
        }
        const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
        for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word))
        {
            const DocumentData& doc = documents_.at(document_id);
            if (filtr(document_id, doc.status, doc.rating))
            {
                document_to_relevance[document_id] += term_freq * inverse_document_freq;
            }
        }
    }

    for (const std::string& word : query.minus_words)
    {
        if (word_to_document_freqs_.count(word) == 0)
        {
            continue;
        }
        for (const auto [document_id, _] : word_to_document_freqs_.at(word))
        {
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

template <typename StopWords>
SearchServer::SearchServer(const StopWords& stop_words)
{
    using namespace std;
    for (const std::string& word : stop_words)
    {
        if (word.empty() || !IsValidWord(word))
        {
            throw invalid_argument("Invalid stop-word"s);
        }
        stop_words_.insert(word);
    }
}
