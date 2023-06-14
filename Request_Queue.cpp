#include "Request_Queue.h"
    // сделаем "обЄртки" дл€ всех методов поиска, чтобы сохран€ть результаты дл€ нашей статистики
    std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus status)
    {
        QueryResult qr(NotProcessedQuery, SplitIntoWords(raw_query), status, false);
        std::vector<Document> result = search_server_.FindTopDocuments(raw_query, status);
        qr.request_result = result.size();
        ProcessingRequest(qr); //
        return result;
    }
    std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query)
    {
        QueryResult qr(NotProcessedQuery, SplitIntoWords(raw_query), DocumentStatus::ACTUAL, false);
        std::vector<Document> result = search_server_.FindTopDocuments(raw_query);
        qr.request_result = result.size();
        ProcessingRequest(qr);
        return result;
    }

    // ≈сли основна€ цель класса узнать количество нулевых запросов, 
    // то стоит хранить количество нулевых запросов в отдельной переменной, 
    // и при добавлении или удалении запросов из очереди requests_ обновл€ть еЄ значение.
    // »сходил из того, что данна€ функци€ вспомогательна€, то есть нужна редко,
    // поэтому когда нужно пробегаюсь по всей очереди.
    int RequestQueue::GetNoResultRequests() const
    {
        auto it = requests_.begin();
        auto end_deque = requests_.end();
        int result = 0;
        while (it != end_deque)
        {
            it = find_if(it, end_deque, [](QueryResult qr)
                { return qr.request_result == 0; });
            if (it != end_deque)
            {
                ++result;
                ++it;
            }
        }
        return result;
    }
 
    void RequestQueue::ProcessingRequest(QueryResult qr)
    {
        requests_.push_back(qr);
        if (requests_.size() > min_in_day_)
        {
            requests_.pop_front();
        }
    }
