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

    // ������� "������" ��� ���� ������� ������, ����� ��������� ���������� ��� ����� ����������
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
        int request_result; // ���������� ��������� ����������
  //      std::vector<std::string> query; //����� �������
// ���������:    ��������� ����� ����� ������� ?
//  ���������: �� ����)) ��� ��, ��� � �� ���� ��������� ����� status � is_predicate
// ��� ������� ������ � �������� ���������� request_result, ������ ���� ���� bool.
// ������� �� ����, ��� �� ������ ����� � ������� ��������� ������ ����� ����������,
// �� ��������� ��� ���� ������ ���� ������� ����������, �����, ��� ���� ������� ����� ���� ����� ��� �������,
// ����� ������, � �������, ���, � ������ �������� ���������� ������� ����������� ������, 
// ���� ���� ������ ���������� ������������ �������� �������������. ����� ��������, ��� ������� �������� ������ ������������ �����,
//  ������ ������� ��� �������� ������� �������� ����� ������������.
 // � ������ � ����������� ������� ������ ����� ������������, ��� ��� �� �� ��������� ACTUAL, 
 // ��� � ������ is_predicate == true, ��� �������� ����� ���� ���������
        DocumentStatus status; //
        bool is_predicate;     //
    };
    std::deque<QueryResult> requests_;
    const SearchServer& search_server_;
    const static int min_in_day_ = 1440;
    const static int NotProcessedQuery = -1; //���������� �������� ��� �������������� ������� QueryResult

    void ProcessingRequest(QueryResult qr);
};