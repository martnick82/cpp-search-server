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
        QueryResult() : request_result(NotProcessedQuery), status(), is_predicate(false) {}
        QueryResult(int rr, DocumentStatus st, bool ip)
            : request_result(rr), status(st), is_predicate(ip) {}
        int request_result; // Количество найденных документов
  //      std::vector<std::string> query; //Слова запроса
// ЗАМЕЧАНИЕ:    насколько нужны слова запроса ?
//  Пояснение: не знаю)) так же, как и не знаю насколько нужны status и is_predicate
// для решения задачи в тренажёре достаточно request_result, причём даже типа bool.
// Исходил из того, что на данный класс в задании возложена задача сбора статистики,
// но поскольку мне пока неясны цели анализа статистики, решмл, что сами запросы могут быть важны для анализа,
// чтобы понять, к примеру, что, в случае большого количества нулевых результатов поиска, 
// наша база данных документов нерелевантна запросам пользователей. Также учитывал, что запросы хранятся только ограниченное время,
//  значит ресурсы для хранения истории запросов нужны ограниченные.
 // А статус в большинстве случаев вообще можно игнорировать, так как он по умолчанию ACTUAL, 
 // вот в случае is_predicate == true, его значение может быть интересно
        DocumentStatus status; //
        bool is_predicate;     //
    };
    std::deque<QueryResult> requests_;
    const SearchServer& search_server_;
    const static int min_in_day_ = 1440;
    const static int NotProcessedQuery = -1; //Невалидное значение для иниациализации объекта QueryResult

    void ProcessingRequest(QueryResult qr);
};