#include "TestingSS.h"

// --------- Начало модульных тестов поисковой системы -----------
using namespace std;
// Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов
void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = { 1, 2, 3 };
    // Сначала убеждаемся, что поиск слова, не входящего в список стоп-слов,
    // находит нужный документ
    {
        SearchServer server(""s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }
    // Затем убеждаемся, что поиск этого же слова, входящего в список стоп-слов,
    // возвращает пустой результат
    {
        SearchServer server("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT(server.FindTopDocuments("in"s).empty());
    }
}

//Добавление документов. Добавленный документ должен находиться по поисковому запросу, который содержит слова из документа.
void TestDocumentSearching()
{
    const int doc_id = 27;
    const string content = "cat in the city"s;
    const vector<int> ratings = { 1, 2, 3 };
    // Сначала убеждаемся, что поиск не находит документ    
    SearchServer search_server("и в на in the"s);
    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });
    ASSERT(search_server.FindTopDocuments("cat"s).empty());

    // Затем убеждаемся,то поиск находит документ         
    search_server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);    
    const auto found_docs = search_server.FindTopDocuments("cat"s);
    ASSERT_EQUAL(found_docs.size(), 1);
    const Document& doc0 = found_docs[0];
    ASSERT_EQUAL(doc0.id, doc_id);
}

//Поддержка минус-слов. Документы, содержащие минус-слова поискового запроса, не должны включаться в результаты поиска.
void TestMinusWords()
{
    // Сначала убеждаемся, что поиск корректно находит документы без минус слов
    SearchServer search_server("и в на in the"s);
    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });
    {
        const auto found_docs = search_server.FindTopDocuments("кот"s);
        ASSERT_EQUAL(found_docs.size(), 2);
        ASSERT_EQUAL(found_docs[0].id, 1);
        ASSERT_EQUAL(found_docs[1].id, 0);
    }

    // Затем убеждаемся,то поиск находит меньше документов с запросом, содержащий минус-слово         

    {
        const auto found_docs = search_server.FindTopDocuments("кот -белый"s);
        ASSERT_EQUAL(found_docs.size(), 1);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, 1);
    }

}
//Матчинг документов. При матчинге документа по поисковому запросу должны быть возвращены все слова из поискового запроса, присутствующие в документе. Если есть соответствие хотя бы по одному минус-слову, должен возвращаться пустой список слов.
void TestDocumentMatching()
{
    SearchServer search_server("и в на in the"s);
    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });

    // проверяем работу проверки соответствия документа поисковому запросу
    {
        const vector <string> query_test = { "евгений"s, "скворец"s };
        const auto& [query, docs_status] = search_server.MatchDocument("скворец     евгений   "s, 3);
        ASSERT_EQUAL(docs_status, DocumentStatus::BANNED);
        ASSERT_EQUAL(query, query_test);
    }

    // проверяем работу проверки соответствия документов поисковому запросу при несоответствии
    {
        const vector <string> query_test = { "пёс"s, "ухоженный"s };
        const auto& [query, docs_status] = search_server.MatchDocument("   ухоженный пёс   "s, 1);
        ASSERT_EQUAL(docs_status, DocumentStatus::ACTUAL);
        ASSERT(query.empty());
    }

}
//Сортировка найденных документов по релевантности. Возвращаемые при поиске документов результаты должны быть отсортированы в порядке убывания релевантности.
void TestSortByRelivance()
{
    SearchServer search_server("и о в по на in the"s);
    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 1 });
    search_server.AddDocument(1, "пушистый кот пушистый хвост чёрный"s, DocumentStatus::ACTUAL, { 1 });
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 1 });
    search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::ACTUAL, { 1 });
    search_server.AddDocument(4, "коварный кот точит когти о ножку стола"s, DocumentStatus::ACTUAL, { 1 });
    search_server.AddDocument(5, "лохматый пёс громко лает на английском языке"s, DocumentStatus::ACTUAL, { 1 });
    search_server.AddDocument(6, "огромный чёрный кот починяет примус насвистывая мелодию"s, DocumentStatus::ACTUAL, { 1 });
    search_server.AddDocument(7, "днем и ночью кот ученый всё ходит по цепи кругом"s, DocumentStatus::ACTUAL, { 1 });
    search_server.AddDocument(8, "куда летит скворец пёс его знает"s, DocumentStatus::ACTUAL, { 1 });

    // Угадайте, кого мы будем искать? :) Ищем котов с разной релевантностью, но строго ранжированных )
    {
        const auto found_docs = search_server.FindTopDocuments("кот"s);
        ASSERT_EQUAL(found_docs.size(), 5);
        double relevance = found_docs[0].relevance;
        for (const Document& doc : found_docs)
        {
            ASSERT(doc.relevance <= relevance);
            relevance = doc.relevance;
        }

    }

}
//Вычисление рейтинга документов. Рейтинг добавленного документа равен среднему арифметическому оценок документа.
void TestDocumentRatingCalculation()
{
    SearchServer search_server("и в на in the"s);
    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });
    {
        const auto found_docs = search_server.FindTopDocuments("кот"s);
        ASSERT_EQUAL(found_docs.size(), 2);
        const Document& doc0 = found_docs[0];
        const Document& doc1 = found_docs[1];
        ASSERT_EQUAL(doc0.id, 1);
        ASSERT_EQUAL(doc1.id, 0);
        ASSERT_EQUAL(doc0.rating, ((7 + 2 + 7) / 3)); // Проверяем расчёт рейтинга, как среднее целочисленное из оценок
        ASSERT_EQUAL(doc1.rating, ((8 - 3) / 2));    // Проверяем расчёт рейтинга, как среднее целочисленное из оценок
    }
}
//Фильтрация результатов поиска с использованием предиката, задаваемого пользователем.
void TestPredicateFunction()
{
    // Create test server
    SearchServer search_server("и в на in the"s);
    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });

    //Проверка фильтрации по ID
    {
        const auto found_docs = search_server.FindTopDocuments("кот"s, []([[maybe_unused]] int doc_id, [[maybe_unused]] DocumentStatus status, [[maybe_unused]] double rating) {
            return doc_id == 1; });
        ASSERT_EQUAL(found_docs.size(), 1);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, 1);
    }
    //Проверка фильтрации по статусу
    {
        const auto found_docs = search_server.FindTopDocuments("ухоженный"s, []([[maybe_unused]] int doc_id, [[maybe_unused]] DocumentStatus status, [[maybe_unused]] double rating) {
            return status == DocumentStatus::BANNED; });
        ASSERT_EQUAL(found_docs.size(), 1);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, 3);
    }
    //Проверка фильтрации по рейтингу
    {
        const auto found_docs = search_server.FindTopDocuments("кот"s, []([[maybe_unused]] int doc_id, [[maybe_unused]] DocumentStatus status, [[maybe_unused]] double rating) {
            return rating > 2; });
        ASSERT_EQUAL(found_docs.size(), 1);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, 1);
    }
}

//Поиск документов, имеющих заданный статус.
void TestSearchingDocumentByStatus()
{
    // Create test server
    SearchServer search_server("и в на in the"s);
    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });
    // проверяем поиск со статусом по умолчанию
    {
        const auto found_docs = search_server.FindTopDocuments("ухоженный"s);
        ASSERT_EQUAL(found_docs.size(), 1);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, 2);
    }
    // проверяем поиск со статусом BANNED
    {
        const auto found_docs = search_server.FindTopDocuments("ухоженный"s, DocumentStatus::BANNED);
        ASSERT_EQUAL(found_docs.size(), 1);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, 3);
    }
}
//Корректное вычисление релевантности найденных документов.
void TestRelevanceSearchihgDocuments()
{
    // Create test server
    SearchServer search_server("и в на in the"s);
    search_server.AddDocument(0, "белый кот и красивый модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });

    // проверяем ранжирование и расчёт по релевантности
    {
        const auto found_docs = search_server.FindTopDocuments("кот"s);
        ASSERT_EQUAL(found_docs.size(), 2);
        const Document& doc0 = found_docs[0];
        const Document& doc1 = found_docs[1];
        ASSERT_EQUAL(doc0.id, 1);
        ASSERT_EQUAL(doc1.id, 0);
        ASSERT(abs(doc0.relevance - log(search_server.GetDocumentCount() * 1.0 / 2) * (1.0 / 4)) < 1e-6);
        ASSERT(abs(doc1.relevance - log(search_server.GetDocumentCount() * 1.0 / 2) * (1.0 / 5)) < 1e-6);    }


}

//Тест работы исключений
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


    SearchServer search_server("и в на in the"s);
    search_server.AddDocument(0, "белый кот и красивый модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(30, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(10, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(20, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });

    // Test negative ID in AddDocument
    try {
        search_server.AddDocument(-7, "белый модный ошейник"s, DocumentStatus::ACTUAL, { 1 });
        cerr << "Testing negative ID in AddDocument " << " --- TEST FAILED ---"s << endl;
    }
    catch (const invalid_argument& error)
    {
        cerr << "Don't worry. Its just testing AddDocument: "s
            << error.what() << " --- TEST OK --- "s << endl;
    }

    // Test existing ID in AddDocument
    try {
        search_server.AddDocument(10, "белый модный ошейник"s, DocumentStatus::ACTUAL, { 1 });
        cerr << "Testing existing ID in AddDocument " << " --- TEST FAILED ---"s << endl;
    }
    catch (const invalid_argument& error)
    {
        cerr << "Don't worry. Its just testing AddDocument: "s << error.what() << " --- TEST OK --- "s << endl;
    }

    // Test invalid text in AddDocument
    try {
        search_server.AddDocument(40, "белый модный \2\0 -ошейник"s, DocumentStatus::ACTUAL, { 1 });
        cerr << "Testing invalid text in AddDocument " << " --- TEST FAILED ---"s << endl;
    }
    catch (const invalid_argument& error)
    {
        cerr << "Don't worry. Its just testing AddDocument: "s << error.what() << " --- TEST OK --- "s << endl;
    }

    // Test invalid words in query FindTopDocuments
    try {
        search_server.FindTopDocuments("белый модный \20 ошейник"s);
        cerr << "Testing invalid symbols in FindTopDocuments " << " --- TEST FAILED ---"s << endl;
    }
    catch (const invalid_argument& error)
    {
        cerr << "Don't worry. Its just testing invalid symbols in FindTopDocuments: "s
            << error.what() << " --- TEST OK --- "s << endl;
    }

    try {
        search_server.FindTopDocuments("белый --ошейник"s);
        cerr << "Testing double minus in FindTopDocuments " << " --- TEST FAILED ---"s << endl;
    }
    catch (const invalid_argument& error)
    {
        cerr << "Don't worry. Its just testing double minus in FindTopDocuments: "s
            << error.what() << " --- TEST OK --- "s << endl;
    }

    try {
        search_server.FindTopDocuments("белый-"s);
        cerr << "Testing end minus in FindTopDocuments " << " --- TEST FAILED ---"s << endl;
    }
    catch (const invalid_argument& error)
    {
        cerr << "Don't worry. Its just testing end minus in FindTopDocuments: "s
            << error.what() << " --- TEST OK --- "s << endl;
    }

    //Test invalid query in MatchDocument
    try {
        const auto result = search_server.MatchDocument("белый --ошейник"s, 0);
        cerr << "Testing double minus in MatchDocument " << " --- TEST FAILED ---"s << endl;
    }
    catch (const invalid_argument& error)
    {
        cerr << "Don't worry. Its just testing double minus in MatchDocument: "s
            << error.what() << " --- TEST OK --- "s << endl;
    }

    try {
        const auto& result = search_server.MatchDocument("белый ошейник -\2"s, 0);
        cerr << "Testing invalid symbols in MatchDocument " << " --- TEST FAILED ---"s << endl;
    }
    catch (const invalid_argument& error)
    {
        cerr << "Don't worry. Its just testing invalid symbols in MatchDocument: "s
            << error.what() << " --- TEST OK --- "s << endl;
    }

    try {
        const auto& result = search_server.MatchDocument("белый ошейник-"s, 0);
        cerr << "Testing end minus in MatchDocument " << " --- TEST FAILED ---"s << endl;
    }
    catch (const invalid_argument& error)
    {
        cerr << "Don't worry. Its just testing end minus in MatchDocument: "s
            << error.what() << " --- TEST OK --- "s << endl;
    }
}
// тестирование методов begin() и end()
void TestIterators()
{
    SearchServer search_server("и в на in the"s);
    ASSERT_EQUAL(search_server.end() - search_server.begin(), 0);
    search_server.AddDocument(0, "белый кот и красивый модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(30, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(10, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(20, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });
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
    SearchServer search_server("и в на in the"s);
    search_server.AddDocument(0, "белый кот и красивый модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(30, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(10, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(20, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });
    const map <string, double> wfd = search_server.GetWordFrequencies(10);
    map<string, double> test;
    test["ухоженный"s] = 1.0 / 4 * log(4 / 2);
    test["пёс"s] = 1.0 / 4 * log(4 / 1);
    test["выразительные"s] = 1.0 / 4 * log(4 / 1);
    test["глаза"s] = 1.0 / 4 * log(4 / 1);
    for (auto& [word, tf_idf] : test)
    {
        ASSERT_EQUAL(wfd.count(word), 1);
        ASSERT(std::abs(wfd.at(word) - tf_idf) < 1e-6);
    }
}

void TestRemoveDocument()
{
    SearchServer search_server("и в на in the"s);
    search_server.AddDocument(0, "белый кот и красивый модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(30, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(10, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(20, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });
    search_server.RemoveDocument(20);
    ASSERT_EQUAL(search_server.GetDocumentCount(), 3);
    vector<Document> res = search_server.FindTopDocuments("евгений");
    ASSERT_EQUAL(res.size(), 0);
}
//Тест возврата слов по ID документа
void TestGetWords()
{
    SearchServer search_server("и в на in the"s);
    search_server.AddDocument(0, "белый кот и красивый модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(30, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(10, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(20, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });
    set<string> test; 
    test.insert("ухоженный"s);
    test.insert("пёс"s);
    test.insert("выразительные"s);
    test.insert("глаза"s);
    set<string> result = search_server.GetWords(10);
    ASSERT_EQUAL(test, result);
}

// Функция TestSearchServer является точкой входа для запуска тестов
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
// --------- Окончание модульных тестов поисковой системы -----------