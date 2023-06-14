#pragma once
#include "String_processing.h"
#include "Document.h"
#include <vector>
#include <map>
#include <set>
#include <map>
#include <algorithm>
const int MAX_RESULT_DOCUMENT_COUNT = 5;
const double DOUBLE_ACCURACY = 1e-6;

class SearchServer
{
public:
    template <typename StopWords>
    explicit SearchServer(const StopWords& stop_words)
    {
        using namespace std;
        for (const  std::string& word : stop_words)
        {
            if (word.empty() || !IsValidWord(word))
            {
                throw invalid_argument("Invalid stop-word"s);
            }
            stop_words_.insert(word);
        }
    }

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

    int GetDocumentId(const int index) const;
    
    int GetDocumentCount() const;

    std::tuple< std::vector< std::string>, DocumentStatus> MatchDocument(const  std::string& raw_query,
        int document_id) const;

private:
    struct DocumentData
    {
        DocumentData() : rating(0), status(DocumentStatus::ACTUAL) {};
        DocumentData(int rate, DocumentStatus status)
            : rating(rate), status(status) {};
        int rating;
        DocumentStatus status;
    };

    std::vector<int> document_id_chain_; //содержит ID документов в последовательности их добавления на сервер
    std::set<std::string> stop_words_; //База стоп-слов
    std::map<std::string, std::map<int, double>> word_to_document_freqs_;  //словарь, где для каждого слова на сервере храниться ID документов, в которых слово встречается и IDF слова для документа
    std::map<int, DocumentData> documents_; //словарь ID документов с информацией о рейтинге и статусе
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
    std::vector<Document> FindAllDocuments(const Query& query, Filtr filtr) const {
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
};
