#pragma once
#include "Document.h"
#include "SearchServer.h"
#include "String_processing.h"
#include <deque>
#include <vector>
#include <string>
class RequestQueue
{
public:
    explicit RequestQueue(const SearchServer& search_server)
        : requests_(), search_server_(search_server) {}

    // сделаем "обёртки" для всех методов поиска, чтобы сохранять результаты для нашей статистики
    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate)
    {
        QueryResult qr(NotProcessedQuery, SplitIntoWords(raw_query), DocumentStatus::ACTUAL, true);
        std::vector<Document> result = search_server_.FindTopDocuments(raw_query, document_predicate);
        qr.request_result = result.size();
        ProcessingRequest(qr); // 
        return result;

    }
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);
    std::vector<Document> AddFindRequest(const std::string& raw_query);
    int GetNoResultRequests() const;
private:
    struct QueryResult {
        QueryResult() : request_result(NotProcessedQuery), query({}), status(), is_predicate(false) {}
        QueryResult(int rr, std::vector<std::string> rq, DocumentStatus st, bool ip)
            : request_result(rr), query(rq), status(st), is_predicate(ip) {}
        int request_result; // Количество найденных документов
        std::vector<std::string> query; //Слова запроса
        DocumentStatus status; //
        bool is_predicate;     //
    };
    std::deque<QueryResult> requests_;
    const SearchServer& search_server_;
    const static int min_in_day_ = 1440;
    const static int NotProcessedQuery = -1; //Невалидное значение для иниациализации объекта QueryResult

    void ProcessingRequest(QueryResult qr);
};