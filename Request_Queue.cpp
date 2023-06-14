#include "Request_Queue.h"
    // ������� "������" ��� ���� ������� ������, ����� ��������� ���������� ��� ����� ����������
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

    // ���� �������� ���� ������ ������ ���������� ������� ��������, 
    // �� ����� ������� ���������� ������� �������� � ��������� ����������, 
    // � ��� ���������� ��� �������� �������� �� ������� requests_ ��������� � ��������.
    // ������� �� ����, ��� ������ ������� ���������������, �� ���� ����� �����,
    // ������� ����� ����� ���������� �� ���� �������.
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
