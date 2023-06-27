#include "TestingSS.h"

// --------- ������ ��������� ������ ��������� ������� -----------
using namespace std;
// ���� ���������, ��� ��������� ������� ��������� ����-����� ��� ���������� ����������
void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = { 1, 2, 3 };
    // ������� ����������, ��� ����� �����, �� ��������� � ������ ����-����,
    // ������� ������ ��������
    {
        SearchServer server(""s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }
    // ����� ����������, ��� ����� ����� �� �����, ��������� � ������ ����-����,
    // ���������� ������ ���������
    {
        SearchServer server("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT(server.FindTopDocuments("in"s).empty());
    }
}

//���������� ����������. ����������� �������� ������ ���������� �� ���������� �������, ������� �������� ����� �� ���������.
void TestDocumentSearching()
{
    const int doc_id = 27;
    const string content = "cat in the city"s;
    const vector<int> ratings = { 1, 2, 3 };
    // ������� ����������, ��� ����� �� ������� ��������    
    SearchServer search_server("� � �� in the"s);
    search_server.AddDocument(0, "����� ��� � ������ �������"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "�������� ��� �������� �����"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "��������� �� ������������� �����"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(3, "��������� ������� �������"s, DocumentStatus::BANNED, { 9 });
    ASSERT(search_server.FindTopDocuments("cat"s).empty());

    // ����� ����������,�� ����� ������� ��������         
    search_server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);    
    const auto found_docs = search_server.FindTopDocuments("cat"s);
    ASSERT_EQUAL(found_docs.size(), 1);
    const Document& doc0 = found_docs[0];
    ASSERT_EQUAL(doc0.id, doc_id);
}

//��������� �����-����. ���������, ���������� �����-����� ���������� �������, �� ������ ���������� � ���������� ������.
void TestMinusWords()
{
    // ������� ����������, ��� ����� ��������� ������� ��������� ��� ����� ����
    SearchServer search_server("� � �� in the"s);
    search_server.AddDocument(0, "����� ��� � ������ �������"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "�������� ��� �������� �����"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "��������� �� ������������� �����"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(3, "��������� ������� �������"s, DocumentStatus::BANNED, { 9 });
    {
        const auto found_docs = search_server.FindTopDocuments("���"s);
        ASSERT_EQUAL(found_docs.size(), 2);
        ASSERT_EQUAL(found_docs[0].id, 1);
        ASSERT_EQUAL(found_docs[1].id, 0);
    }

    // ����� ����������,�� ����� ������� ������ ���������� � ��������, ���������� �����-�����         

    {
        const auto found_docs = search_server.FindTopDocuments("��� -�����"s);
        ASSERT_EQUAL(found_docs.size(), 1);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, 1);
    }

}
//������� ����������. ��� �������� ��������� �� ���������� ������� ������ ���� ���������� ��� ����� �� ���������� �������, �������������� � ���������. ���� ���� ������������ ���� �� �� ������ �����-�����, ������ ������������ ������ ������ ����.
void TestDocumentMatching()
{
    SearchServer search_server("� � �� in the"s);
    search_server.AddDocument(0, "����� ��� � ������ �������"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "�������� ��� �������� �����"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "��������� �� ������������� �����"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(3, "��������� ������� �������"s, DocumentStatus::BANNED, { 9 });

    // ��������� ������ �������� ������������ ��������� ���������� �������
    {
        const vector <string> query_test = { "�������"s, "�������"s };
        const auto& [query, docs_status] = search_server.MatchDocument("�������     �������   "s, 3);
        ASSERT_EQUAL(docs_status, DocumentStatus::BANNED);
        ASSERT_EQUAL(query, query_test);
    }

    // ��������� ������ �������� ������������ ���������� ���������� ������� ��� ��������������
    {
        const vector <string> query_test = { "��"s, "���������"s };
        const auto& [query, docs_status] = search_server.MatchDocument("   ��������� ��   "s, 1);
        ASSERT_EQUAL(docs_status, DocumentStatus::ACTUAL);
        ASSERT(query.empty());
    }

}
//���������� ��������� ���������� �� �������������. ������������ ��� ������ ���������� ���������� ������ ���� ������������� � ������� �������� �������������.
void TestSortByRelivance()
{
    SearchServer search_server("� � � �� �� in the"s);
    search_server.AddDocument(0, "����� ��� � ������ �������"s, DocumentStatus::ACTUAL, { 1 });
    search_server.AddDocument(1, "�������� ��� �������� ����� ������"s, DocumentStatus::ACTUAL, { 1 });
    search_server.AddDocument(2, "��������� �� ������������� �����"s, DocumentStatus::ACTUAL, { 1 });
    search_server.AddDocument(3, "��������� ������� �������"s, DocumentStatus::ACTUAL, { 1 });
    search_server.AddDocument(4, "�������� ��� ����� ����� � ����� �����"s, DocumentStatus::ACTUAL, { 1 });
    search_server.AddDocument(5, "�������� �� ������ ���� �� ���������� �����"s, DocumentStatus::ACTUAL, { 1 });
    search_server.AddDocument(6, "�������� ������ ��� �������� ������ ����������� �������"s, DocumentStatus::ACTUAL, { 1 });
    search_server.AddDocument(7, "���� � ����� ��� ������ �� ����� �� ���� ������"s, DocumentStatus::ACTUAL, { 1 });
    search_server.AddDocument(8, "���� ����� ������� �� ��� �����"s, DocumentStatus::ACTUAL, { 1 });

    // ��������, ���� �� ����� ������? :) ���� ����� � ������ ��������������, �� ������ ������������� )
    {
        const auto found_docs = search_server.FindTopDocuments("���"s);
        ASSERT_EQUAL(found_docs.size(), 5);
        double relevance = found_docs[0].relevance;
        for (const Document& doc : found_docs)
        {
            ASSERT(doc.relevance <= relevance);
            relevance = doc.relevance;
        }

    }

}
//���������� �������� ����������. ������� ������������ ��������� ����� �������� ��������������� ������ ���������.
void TestDocumentRatingCalculation()
{
    SearchServer search_server("� � �� in the"s);
    search_server.AddDocument(0, "����� ��� � ������ �������"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "�������� ��� �������� �����"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "��������� �� ������������� �����"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(3, "��������� ������� �������"s, DocumentStatus::BANNED, { 9 });
    {
        const auto found_docs = search_server.FindTopDocuments("���"s);
        ASSERT_EQUAL(found_docs.size(), 2);
        const Document& doc0 = found_docs[0];
        const Document& doc1 = found_docs[1];
        ASSERT_EQUAL(doc0.id, 1);
        ASSERT_EQUAL(doc1.id, 0);
        ASSERT_EQUAL(doc0.rating, ((7 + 2 + 7) / 3)); // ��������� ������ ��������, ��� ������� ������������� �� ������
        ASSERT_EQUAL(doc1.rating, ((8 - 3) / 2));    // ��������� ������ ��������, ��� ������� ������������� �� ������
    }
}
//���������� ����������� ������ � �������������� ���������, ����������� �������������.
void TestPredicateFunction()
{
    // Create test server
    SearchServer search_server("� � �� in the"s);
    search_server.AddDocument(0, "����� ��� � ������ �������"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "�������� ��� �������� �����"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "��������� �� ������������� �����"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(3, "��������� ������� �������"s, DocumentStatus::BANNED, { 9 });

    //�������� ���������� �� ID
    {
        const auto found_docs = search_server.FindTopDocuments("���"s, []([[maybe_unused]] int doc_id, [[maybe_unused]] DocumentStatus status, [[maybe_unused]] double rating) {
            return doc_id == 1; });
        ASSERT_EQUAL(found_docs.size(), 1);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, 1);
    }
    //�������� ���������� �� �������
    {
        const auto found_docs = search_server.FindTopDocuments("���������"s, []([[maybe_unused]] int doc_id, [[maybe_unused]] DocumentStatus status, [[maybe_unused]] double rating) {
            return status == DocumentStatus::BANNED; });
        ASSERT_EQUAL(found_docs.size(), 1);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, 3);
    }
    //�������� ���������� �� ��������
    {
        const auto found_docs = search_server.FindTopDocuments("���"s, []([[maybe_unused]] int doc_id, [[maybe_unused]] DocumentStatus status, [[maybe_unused]] double rating) {
            return rating > 2; });
        ASSERT_EQUAL(found_docs.size(), 1);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, 1);
    }
}

//����� ����������, ������� �������� ������.
void TestSearchingDocumentByStatus()
{
    // Create test server
    SearchServer search_server("� � �� in the"s);
    search_server.AddDocument(0, "����� ��� � ������ �������"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "�������� ��� �������� �����"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "��������� �� ������������� �����"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(3, "��������� ������� �������"s, DocumentStatus::BANNED, { 9 });
    // ��������� ����� �� �������� �� ���������
    {
        const auto found_docs = search_server.FindTopDocuments("���������"s);
        ASSERT_EQUAL(found_docs.size(), 1);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, 2);
    }
    // ��������� ����� �� �������� BANNED
    {
        const auto found_docs = search_server.FindTopDocuments("���������"s, DocumentStatus::BANNED);
        ASSERT_EQUAL(found_docs.size(), 1);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, 3);
    }
}
//���������� ���������� ������������� ��������� ����������.
void TestRelevanceSearchihgDocuments()
{
    // Create test server
    SearchServer search_server("� � �� in the"s);
    search_server.AddDocument(0, "����� ��� � �������� ������ �������"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "�������� ��� �������� �����"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "��������� �� ������������� �����"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(3, "��������� ������� �������"s, DocumentStatus::BANNED, { 9 });

    // ��������� ������������ � ������ �� �������������
    {
        const auto found_docs = search_server.FindTopDocuments("���"s);
        ASSERT_EQUAL(found_docs.size(), 2);
        const Document& doc0 = found_docs[0];
        const Document& doc1 = found_docs[1];
        ASSERT_EQUAL(doc0.id, 1);
        ASSERT_EQUAL(doc1.id, 0);
        ASSERT(abs(doc0.relevance - log(search_server.GetDocumentCount() * 1.0 / 2) * (1.0 / 4)) < 1e-6);
        ASSERT(abs(doc1.relevance - log(search_server.GetDocumentCount() * 1.0 / 2) * (1.0 / 5)) < 1e-6);    }


}

//���� ������ ����������
void TestException()
{
    //Test invalid stop-words string
    try {
        SearchServer search_server("C++\0\2"s);
        cerr << "Testing invalid stop-words string" << " --- TEST FAILED ---"s << endl;
    }
    catch (const invalid_argument& error)
    {
        cerr << "Don't worry. Its just testing stop-words string: "s << error.what() << " --- TEST OK --- "s << endl;
    };

    try {
        vector<string> stop_words = { "-"s, "\0"s, "--"s };
        SearchServer search_server(stop_words);
        cerr << "Testing invalid stop-words vector" << " --- TEST FAILED --- "s << endl;
    }
    catch (const invalid_argument& error)
    {
        cerr << "Don't worry. Its just testing stop-words vector: "s << error.what() << " --- TEST OK ---"s << endl;
    }


    SearchServer search_server("� � �� in the"s);
    search_server.AddDocument(0, "����� ��� � �������� ������ �������"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(30, "�������� ��� �������� �����"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(10, "��������� �� ������������� �����"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(20, "��������� ������� �������"s, DocumentStatus::BANNED, { 9 });

    // Test negative ID in AddDocument
    try {
        search_server.AddDocument(-7, "����� ������ �������"s, DocumentStatus::ACTUAL, { 1 });
        cerr << "Testing negative ID in AddDocument " << " --- TEST FAILED ---"s << endl;
    }
    catch (const invalid_argument& error)
    {
        cerr << "Don't worry. Its just testing AddDocument: "s
            << error.what() << " --- TEST OK --- "s << endl;
    }

    // Test existing ID in AddDocument
    try {
        search_server.AddDocument(10, "����� ������ �������"s, DocumentStatus::ACTUAL, { 1 });
        cerr << "Testing existing ID in AddDocument " << " --- TEST FAILED ---"s << endl;
    }
    catch (const invalid_argument& error)
    {
        cerr << "Don't worry. Its just testing AddDocument: "s << error.what() << " --- TEST OK --- "s << endl;
    }

    // Test invalid text in AddDocument
    try {
        search_server.AddDocument(40, "����� ������ \2\0 -�������"s, DocumentStatus::ACTUAL, { 1 });
        cerr << "Testing invalid text in AddDocument " << " --- TEST FAILED ---"s << endl;
    }
    catch (const invalid_argument& error)
    {
        cerr << "Don't worry. Its just testing AddDocument: "s << error.what() << " --- TEST OK --- "s << endl;
    }

    // Test invalid words in query FindTopDocuments
    try {
        search_server.FindTopDocuments("����� ������ \20 �������"s);
        cerr << "Testing invalid symbols in FindTopDocuments " << " --- TEST FAILED ---"s << endl;
    }
    catch (const invalid_argument& error)
    {
        cerr << "Don't worry. Its just testing invalid symbols in FindTopDocuments: "s
            << error.what() << " --- TEST OK --- "s << endl;
    }

    try {
        search_server.FindTopDocuments("����� --�������"s);
        cerr << "Testing double minus in FindTopDocuments " << " --- TEST FAILED ---"s << endl;
    }
    catch (const invalid_argument& error)
    {
        cerr << "Don't worry. Its just testing double minus in FindTopDocuments: "s
            << error.what() << " --- TEST OK --- "s << endl;
    }

    try {
        search_server.FindTopDocuments("�����-"s);
        cerr << "Testing end minus in FindTopDocuments " << " --- TEST FAILED ---"s << endl;
    }
    catch (const invalid_argument& error)
    {
        cerr << "Don't worry. Its just testing end minus in FindTopDocuments: "s
            << error.what() << " --- TEST OK --- "s << endl;
    }

    //Test invalid query in MatchDocument
    try {
        const auto result = search_server.MatchDocument("����� --�������"s, 0);
        cerr << "Testing double minus in MatchDocument " << " --- TEST FAILED ---"s << endl;
    }
    catch (const invalid_argument& error)
    {
        cerr << "Don't worry. Its just testing double minus in MatchDocument: "s
            << error.what() << " --- TEST OK --- "s << endl;
    }

    try {
        const auto& result = search_server.MatchDocument("����� ������� -\2"s, 0);
        cerr << "Testing invalid symbols in MatchDocument " << " --- TEST FAILED ---"s << endl;
    }
    catch (const invalid_argument& error)
    {
        cerr << "Don't worry. Its just testing invalid symbols in MatchDocument: "s
            << error.what() << " --- TEST OK --- "s << endl;
    }

    try {
        const auto& result = search_server.MatchDocument("����� �������-"s, 0);
        cerr << "Testing end minus in MatchDocument " << " --- TEST FAILED ---"s << endl;
    }
    catch (const invalid_argument& error)
    {
        cerr << "Don't worry. Its just testing end minus in MatchDocument: "s
            << error.what() << " --- TEST OK --- "s << endl;
    }
}
// ������������ ������� begin() � end()
void TestIterators()
{
    SearchServer search_server("� � �� in the"s);
    ASSERT_EQUAL(search_server.end() - search_server.begin(), 0);
    search_server.AddDocument(0, "����� ��� � �������� ������ �������"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(30, "�������� ��� �������� �����"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(10, "��������� �� ������������� �����"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(20, "��������� ������� �������"s, DocumentStatus::BANNED, { 9 });
    int num = 0;
    vector<int> test_vector = { 0, 30, 10, 20 };
    ASSERT_EQUAL(search_server.end() - search_server.begin(), 4);
    for (int id : search_server)
    {
        ASSERT_EQUAL(id, test_vector.at(num));
        ++num;
    }
    ASSERT_EQUAL(num, 4);
}

void TestGetWordFrequencies()
{
    SearchServer search_server("� � �� in the"s);
    search_server.AddDocument(0, "����� ��� � �������� ������ �������"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(30, "�������� ��� �������� �����"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(10, "��������� �� ������������� �����"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(20, "��������� ������� �������"s, DocumentStatus::BANNED, { 9 });
    const map <string, double> wfd = search_server.GetWordFrequencies(10);
    map<string, double> test;
    test["���������"s] = 1.0 / 4 * log(4 / 2);
    test["��"s] = 1.0 / 4 * log(4 / 1);
    test["�������������"s] = 1.0 / 4 * log(4 / 1);
    test["�����"s] = 1.0 / 4 * log(4 / 1);
    for (auto& [word, tf_idf] : test)
    {
        ASSERT_EQUAL(wfd.count(word), 1);
        ASSERT(std::abs(wfd.at(word) - tf_idf) < 1e-6);
    }
}

void TestRemoveDocument()
{
    SearchServer search_server("� � �� in the"s);
    search_server.AddDocument(0, "����� ��� � �������� ������ �������"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(30, "�������� ��� �������� �����"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(10, "��������� �� ������������� �����"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(20, "��������� ������� �������"s, DocumentStatus::BANNED, { 9 });
    search_server.RemoveDocument(20);
    ASSERT_EQUAL(search_server.GetDocumentCount(), 3);
    vector<Document> res = search_server.FindTopDocuments("�������");
    ASSERT_EQUAL(res.size(), 0);
}
//���� �������� ���� �� ID ���������
void TestGetWords()
{
    SearchServer search_server("� � �� in the"s);
    search_server.AddDocument(0, "����� ��� � �������� ������ �������"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(30, "�������� ��� �������� �����"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(10, "��������� �� ������������� �����"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(20, "��������� ������� �������"s, DocumentStatus::BANNED, { 9 });
    set<string> test; 
    test.insert("���������"s);
    test.insert("��"s);
    test.insert("�������������"s);
    test.insert("�����"s);
    set<string> result = search_server.GetWords(10);
    ASSERT_EQUAL(test, result);
}

// ������� TestSearchServer �������� ������ ����� ��� ������� ������
void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestDocumentSearching);
    RUN_TEST(TestMinusWords);
    RUN_TEST(TestDocumentMatching);
    RUN_TEST(TestSortByRelivance);
    RUN_TEST(TestDocumentRatingCalculation);
    RUN_TEST(TestPredicateFunction);
    RUN_TEST(TestSearchingDocumentByStatus);
    RUN_TEST(TestRelevanceSearchihgDocuments);
    RUN_TEST(TestException);
    RUN_TEST(TestIterators);
    RUN_TEST(TestGetWordFrequencies);
    RUN_TEST(TestRemoveDocument);
    RUN_TEST(TestGetWords);
}
// --------- ��������� ��������� ������ ��������� ������� -----------