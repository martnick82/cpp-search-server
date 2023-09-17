#pragma once
#include <vector>
#include <map>
#include <set>
#include <deque>
#include <string>
#include <string_view>
#include <algorithm>
#include <cmath>
#include <execution>
#include <functional>
#include <cassert>
#include <thread>

#include "string_processing.h"
#include "document.h"
#include "concurrent_map.h"
#include "log_duration.h"


const int MAX_RESULT_DOCUMENT_COUNT = 5;
const double DOUBLE_ACCURACY = 1e-6;

class SearchServer
{
public:
    template <typename StopWord>
    explicit SearchServer(const std::set<StopWord>& stop_words)
        : SearchServer(std::vector(stop_words.begin(), stop_words.end())) {}

    template <typename StopWord>
    explicit SearchServer(const std::vector<StopWord>& stop_words);

    explicit SearchServer(const  std::string_view raw_stop_words)
        : SearchServer(SplitIntoWords(raw_stop_words)) {}

    void AddDocument(int document_id, const  std::string_view document, DocumentStatus status,
        const  std::vector<int>& ratings);

    static bool IsValidWord(const  std::string_view text);
   
    static bool IsValidChar(const char& c);    
    //����� ����������
    std::vector<Document> FindTopDocuments(const  std::string_view raw_query) const;  
    std::vector<Document> FindTopDocuments(const  std::string_view raw_query,
        DocumentStatus status) const;    
    template <typename Filtr>
    std::vector<Document> FindTopDocuments(const  std::string_view raw_query,
        Filtr filtr) const;
    //����� ���������� � ���������� ������������ ����������
    template <typename ExecutionPolicy>
    std::vector<Document> FindTopDocuments(ExecutionPolicy&& policy, const  std::string_view raw_query) const;
    template <typename ExecutionPolicy>
    std::vector<Document> FindTopDocuments(ExecutionPolicy&& policy, const  std::string_view raw_query,
        DocumentStatus status) const;
    template <typename Filtr, typename ExecutionPolicy>
    std::vector<Document> FindTopDocuments(ExecutionPolicy&& policy, const  std::string_view raw_query,
        Filtr filtr) const;
  //������:  ������ ��� ��������� �������� - ��� �������!
  //���������: � ������ ������ �� �������� �� cppreference, ��-������� � �������� �������������. 
  // ������ ��� ����������� ������ �������� ����������� ������ �������������� �������� ��� ���������������� ��������,
  // ����� ��������� � ����� ��� ��������.  
  // 
    //���������� ���������� �� �������
    int GetDocumentCount() const;

    //������������: ������� ���������� ������������� ����, ����� ������������� ����� �����
    // ���������: ��������. �������, ��� ��������� �����.
    using matched_document = std::tuple< std::vector< std::string_view>, DocumentStatus>;

    //������� ����������
    matched_document MatchDocument(const  std::string_view raw_query,
        int document_id) const;
    //������� ���������� � ���������� ������������ ����������
    //������������: ����� ����� ���� �� ��� ������ ��������, ��� ��� �� ���� ������ ������ 
    //������������������ �� ������ + ���� ���������� ������������ ������
    // ���������: �� ����� ��� ��� ������, � ���� MatchDocument ��� ����.
    // ��� ���� � ���, ��� ����� ��� execution::seq ������� ���� ������� ������������ � ��������� � ����� ��� ��������?
    // 
    // � ����� ��������, ��� � ����, � ��������� ������� ��� ��� ������� � ������, ����� �������������. 
    // ����� ����� execution::par  �� ��� ���������� ��������������, ��� ����� ��������� ����� �� ������
    //
    template <typename ExecutionPolicy>
    matched_document MatchDocument(ExecutionPolicy&& policy, const  std::string_view raw_query, int document_id) const;

    using Iterator = std::set<int>::iterator;
    std::set<int>::iterator begin();
    std::set<int>::iterator end();

    //����� ��������� ������� ���� �� id ���������
    const std::map<std::string_view, double>& GetWordFrequencies(int document_id) const;

    //�������� ��������� �� ID
    void RemoveDocument(int document_id);
    //�������� ��������� �� ID � ���������� ������������ ����������
    template <typename ExecutionPolicy>
    void RemoveDocument(ExecutionPolicy&& policy, int document_id);

    //��������� ����������  ���� � ���������
    int GetWordsCount(int document_id) const;
    
private:
    struct DocumentData
    {
        
        DocumentData() : rating(0), status(DocumentStatus::ACTUAL), data() {}
        DocumentData(int rate, DocumentStatus status)
            : rating(rate), status(status) {}
        DocumentData(int rate, DocumentStatus status = DocumentStatus::ACTUAL, std:: string text = ""s)
            : rating(rate), status(status), data(text) {}
        int rating;        
        DocumentStatus status;
        std::string data;
    };

    // ��������� ����������
   // std::deque<std::string> storage_;

    //�������� ID ����������   
    std::set<int> document_ids_; 

    //���� ����-����
    std::set<std::string, std::less<>> stop_words_;

    //�������, ��� ��� ������� ����� �� ������� ��������� ID ����������, 
    //� ������� ����� ����������� � IDF ����� ��� ���������
    std::map<std::string_view, std::map<int, double>> word_to_document_freqs_;  

    //������� ID ���������� � ����������� � �������� � �������
    std::map<int, DocumentData> documents_; 

    //����� ���� � ������ ����������
    //std::map<std::vector<std::string_view>, std::vector<int>> word_sets_to_documents_;
    //���������: ����� ����� ������� - �������� ��� ���� ���� � ��������� DocumentData, 
    // ��� ������� ��� ������ �������� ��������� � ������� ������
    // ���������: ��������. ����� � storage_ ����� ���� �������, ��� � � ������. 
    // ��������, ��� ����� ��������� ������ ����� �������. storage_ ������ ��� � ��������� ����������� � �����, 
    // ��� ��� ����� ����� ���������� � ���� �������� ������ ����� ����. 
    // 
    // ID ���������� - ����� ����
    std::map<int, std::vector<std::string_view>> documents_to_word_sets_; 

    //������������: �� ������������� ��������� ����������� � ����� ������(��� ������� �������� ����� ������).
    //����� - ��������� ����� ������� � �����, � ��� ����� ������� � ������������ ��������� ������ ������
    // ���������: ��������. ��������� ����������� �������, � ��� ����, ��� � ���������� ����� ������, 
    // �� ����� ��������� ���, ����� ������ ������ �� ����
    //
    bool IsStopWord(const  std::string_view word) const;

    bool IsDocumentID(const int document_id) const;

    std::vector<std::string_view> SplitIntoWordsNoStop(const std::string_view text) const;

    static int ComputeAverageRating(const  std::vector<int>& ratings);

    struct QueryWord
    {      
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(std::string_view text) const;

    struct Query
    {
        std::vector<std::string_view> plus_words;
        std::vector<std::string_view> minus_words;
    };

    //������� ��������
    //Query ParseQuery(const std::string_view text) const;
    //������� �������� � ���������� ������������ ����������
    //template <typename ExecutionPolicy>
    Query ParseQuery(const std::string_view text, bool sequence = true) const;
    //���������: ���������� ������ ������ �������� �������, ��� ������������� ��������� 
    // �������� ������ ���������� bool - ����������.��� ���, ���� �� ��������� 
    // ���� ���������� �������� �� ���������, �� ����� ����������� ������� ���� ����� 
    // ��� � �����, �� � � ����� �����������
    // ���������: ��������. ��� �������� ���������� ������ ��� � ������ ������ ����� ���� ������.
    //  ��� �� �������� ����� ������������ ��� ������ ������� ������ �������? 
    // ���������� ��������� policy ��� ������� ������� � ��������
    // 
    //���������� IDF �����
    double ComputeWordInverseDocumentFreq(const std::string_view word) const;

    //����� ���������� �� �������
    template <typename Filtr>
    std::vector<Document> FindAllDocuments(const Query& query, Filtr filtr) const;
    // ����� ���������� �� ������� � ���������� ������������ ����������
    template <typename Filtr, typename ExecutionPolicy>
    std::vector<Document> FindAllDocuments(ExecutionPolicy&& policy, const Query& query, Filtr filtr) const;

    
};

template <typename ExecutionPolicy>
std::vector<Document>  SearchServer::FindTopDocuments(ExecutionPolicy&& policy, const  std::string_view raw_query) const
{
    return  FindTopDocuments(policy, raw_query, 
        []([[maybe_unused]] int document_id, DocumentStatus status, [[maybe_unused]] int rating)
        {
            return status == DocumentStatus::ACTUAL;
        });
}

template <typename ExecutionPolicy>
std::vector<Document>  SearchServer::FindTopDocuments(ExecutionPolicy&& policy, const  std::string_view raw_query,
    DocumentStatus status) const
{
    return  FindTopDocuments(policy, raw_query, [status]([[maybe_unused]] int document_id,
        DocumentStatus doc_status, [[maybe_unused]] int rating)
        {
            return status == doc_status;
        });
}

template <typename Filtr>
std::vector<Document> SearchServer::FindTopDocuments(const  std::string_view raw_query,
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
            else
            {
                return lhs.relevance > rhs.relevance;
            }
        });
    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT)
    {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }
    return matched_documents;
}

template <typename Filtr, typename ExecutionPolicy>
std::vector<Document> SearchServer::FindTopDocuments(ExecutionPolicy&& policy, const  std::string_view raw_query,
    Filtr filtr) const
{
    Query query = ParseQuery(raw_query, 0);
    auto matched_documents = FindAllDocuments(policy, query, filtr);
    sort(policy, matched_documents.begin(), matched_documents.end(),
        [](const Document& lhs, const Document& rhs)
        {
            if (std::abs(lhs.relevance - rhs.relevance) < DOUBLE_ACCURACY)
            {
                return lhs.rating > rhs.rating;
            }
            else 
            {
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
std::vector<Document> SearchServer::FindAllDocuments(const Query& query, Filtr filtr) const
{
    std::map<int, double> document_to_relevance;
    for (const std::string_view word : query.plus_words)
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
    for (const std::string_view word : query.minus_words)
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

template <typename Filtr, typename ExecutionPolicy>
std::vector<Document> SearchServer::FindAllDocuments(ExecutionPolicy&& policy, const Query& query, Filtr filtr) const 
{
    ConcurrentMap<int, double> document_to_relevance(std::thread::hardware_concurrency());
    std::for_each(policy,
        query.plus_words.begin(), query.plus_words.end(),
        [this, &document_to_relevance, &filtr](const std::string_view word)
        {            
            if (word_to_document_freqs_.count(word) != 0)
            {
                const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
                const std::map<int, double>& docs_to_freqs = word_to_document_freqs_.at(word);
                std::for_each(docs_to_freqs.begin(), docs_to_freqs.end(),
                    [this, &inverse_document_freq, &document_to_relevance, &filtr](std::pair<int, double> doc_freqs)
                    {
                        const DocumentData& doc = documents_.at(doc_freqs.first);
                        if (filtr(doc_freqs.first, doc.status, doc.rating))
                        {
                            document_to_relevance[doc_freqs.first].ref_to_value += doc_freqs.second * inverse_document_freq;
                        }
                    });
            }
        });
    std::map<int, double> doc_to_rel = document_to_relevance.BuildOrdinaryMap();
    std::for_each(policy,
        query.minus_words.begin(), query.minus_words.end(),
        [this, &doc_to_rel](const std::string_view word)
        {
            if(word_to_document_freqs_.count(word) != 0)
            {
                const std::map<int, double>& docs_to_freqs = word_to_document_freqs_.at(word);
                std::for_each(docs_to_freqs.begin(), docs_to_freqs.end(),
                    [&doc_to_rel](std::pair<int, double> doc_freqs)
                    {
                        doc_to_rel.erase(doc_freqs.first);
                    }
                );
            }
        });    
    std::vector<Document> matched_documents;
    for (const auto [document_id, relevance] : doc_to_rel) {
        matched_documents.push_back(
            { document_id, relevance, documents_.at(document_id).rating });
    }
    return matched_documents;
}

template <typename StopWords>
SearchServer::SearchServer(const std::vector<StopWords>& stop_words)
{
    using namespace std::literals;
    std::for_each(stop_words.begin(), stop_words.end(),
        [this](const std::string_view word)
        {
            if (word.empty() || !IsValidWord(word))
            {
                throw std::invalid_argument("Invalid stop-word <"s + word.data() + ">"s);
            }
            std::string stop_word = word.data();
            stop_word.resize(word.size());
            stop_words_.insert(stop_word);
        });    
}

template <typename ExecutionPolicy>
void SearchServer::RemoveDocument(ExecutionPolicy&& policy, int document_id)
{
    const std::vector<std::string_view>& document_words = documents_to_word_sets_.at(document_id);
    std::for_each(policy, document_words.begin(), document_words.end(),
        [this, document_id](const std::string_view word)
        {
            word_to_document_freqs_.at(word).erase(document_id);
        });
    document_ids_.erase(document_id);
    documents_.erase(document_id);
 /*   std::vector<int>& docs = word_sets_to_documents_.at(document_words);
    auto it = lower_bound(docs.begin(), docs.end(), document_id);
    if (it != docs.end())
    {
        docs.erase(it);
    }
    if (docs.empty())
    {
        word_sets_to_documents_.erase(document_words);
    }*/
    documents_to_word_sets_.erase(document_id);
}

template <typename ExecutionPolicy>
std::tuple< std::vector<std::string_view>, DocumentStatus>
SearchServer::MatchDocument(ExecutionPolicy&& policy, const  std::string_view raw_query, int document_id) const
{
    using namespace std::literals;

    Query query = ParseQuery(raw_query, false);
    const std::vector<std::string_view> document_words = documents_to_word_sets_.at(document_id);
    std::vector<std::string_view> matched_words;
    bool is_minus_word = false;

    is_minus_word = std::any_of(policy, query.minus_words.begin(), query.minus_words.end(),
        [&document_words](const std::string_view query_word)
        {
            return
                std::any_of(document_words.begin(), document_words.end(),
                    [&query_word](const std::string_view word)
                    {
                        return word == query_word;
                    });
        });       
    if (is_minus_word)
    {
        matched_words.clear();
    }
    else
    {
        matched_words.resize(document_words.size() < query.plus_words.size() ?
            document_words.size() : query.plus_words.size());
        std::copy_if(policy, document_words.begin(), document_words.end(),
            matched_words.begin(),
            [&query](const std::string_view word)
            {
                return any_of(query.plus_words.begin(), query.plus_words.end(),
                [&word](const std::string_view query_word)
                {
                    return word == query_word;
                });
            });
        matched_words.erase(std::unique(policy, matched_words.begin(), matched_words.end()), matched_words.end());
        if (matched_words.back() == ""s)
        {
            matched_words.pop_back();
        }
    }
    return tie(matched_words, documents_.at(document_id).status);
}