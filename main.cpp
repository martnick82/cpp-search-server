#include "search_server.h"
#include "document.h"
#include "paginator.h"
#include "request_queue.h"
#include "testingss.h"
#include "string_processing.h"
#include "read_input_functions.h"
#include "log_duration.h"
#include "remove_duplicates.h"

using namespace std;

ostream& operator<< (ostream& out, const Document& document)
{
    out << "{ "s
        << "document_id = "s << document.id << ", "s
        << "relevance = "s << document.relevance << ", "s
        << "rating = "s << document.rating << " }"s;
    return out;
}


template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}
int main() {
    {
        { 
            LOG_DURATION_STREAM("Testing time", cerr);
            TestSearchServer(); 
        }
        {
            LOG_DURATION_STREAM("Checking request queue", cerr);
            SearchServer search_server("and in at"s);
            RequestQueue request_queue(search_server);
            search_server.AddDocument(1, "curly cat curly tail"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
            search_server.AddDocument(2, "curly dog and fancy collar"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
            search_server.AddDocument(3, "big cat fancy collar "s, DocumentStatus::ACTUAL, { 1, 2, 8 });
            search_server.AddDocument(4, "big dog sparrow Eugene"s, DocumentStatus::ACTUAL, { 1, 3, 2 });
            search_server.AddDocument(5, "big dog sparrow Vasiliy"s, DocumentStatus::ACTUAL, { 1, 1, 1 });
            // 1439 �������� � ������� �����������
            for (int i = 0; i < 1439; ++i) {
                request_queue.AddFindRequest("empty request"s);
            }
            // ��� ��� 1439 �������� � ������� �����������
            {LOG_DURATION_STREAM("Operation time", cerr);
            request_queue.AddFindRequest("curly dog"s); }
            // ����� �����, ������ ������ ������, 1438 �������� � ������� �����������
            {LOG_DURATION_STREAM("Operation time", cerr);
            request_queue.AddFindRequest("big collar"s); }
            // ������ ������ ������, 1437 �������� � ������� �����������
            {LOG_DURATION_STREAM("Operation time", cerr);
            request_queue.AddFindRequest("sparrow"s); }
            cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << endl;
        }
        {
            LOG_DURATION_STREAM("Checking RemoveDuplicates", cerr);
            SearchServer search_server("and with"s);

            search_server.AddDocument(1, "funny pet and nasty rat"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
            search_server.AddDocument(2, "funny pet with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });

            // �������� ��������� 2, ����� �����
            search_server.AddDocument(3, "funny pet with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });

            // ������� ������ � ����-������, ������� ����������
            search_server.AddDocument(4, "funny pet and curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });

            // ��������� ���� ����� ��, ������� ���������� ��������� 1
            search_server.AddDocument(5, "funny funny pet and nasty nasty rat"s, DocumentStatus::ACTUAL, { 1, 2 });

            // ���������� ����� �����, ���������� �� ��������
            search_server.AddDocument(6, "funny pet and not very nasty rat"s, DocumentStatus::ACTUAL, { 1, 2 });

            // ��������� ���� ����� ��, ��� � id 6, �������� �� ������ �������, ������� ����������
            search_server.AddDocument(7, "very nasty rat and not very funny pet"s, DocumentStatus::ACTUAL, { 1, 2 });

            // ���� �� ��� �����, �� �������� ����������
            search_server.AddDocument(8, "pet with rat and rat and rat"s, DocumentStatus::ACTUAL, { 1, 2 });

            // ����� �� ������ ����������, �� �������� ����������
            search_server.AddDocument(9, "nasty rat with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });

            cout << "Before duplicates removed: "s << search_server.GetDocumentCount() << endl;
            {
                LOG_DURATION_STREAM("Remove Duplicates time", cerr);
                RemoveDuplicates(search_server);
            }
            cout << "After duplicates removed: "s << search_server.GetDocumentCount() << endl;
            return 0;
        }
    }
}