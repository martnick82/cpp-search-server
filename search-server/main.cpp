#include <iostream>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace std;
// Вывод значения пары
template <typename KeyElement, typename SubjElement>
ostream& operator<<(ostream& os, pair<KeyElement, SubjElement> map_element)
{
    os << map_element.first << ": " << map_element.second;
    return os;
}

// Служебная функция для вывода значений контейнеров
template <typename Obj>
ostream& Print(ostream& os, const Obj& object)
{
    bool is_first = true;
    for (const auto& part : object)
    {
        if (!is_first)
            cout << ", ";
        cout << part;
        is_first = false;
    }
    return os;
}

// Вывод значения вектора
template <typename Element>
ostream& operator<<(ostream& os, vector<Element> vect)
{
    os << "[";
    Print(os, vect) << "]";
    return os;

}

// Вывод значения набора
template <typename Element>
ostream& operator<<(ostream& os, set<Element> setobj)
{
    os << "{";
    Print(os, setobj);
    os << "}";
    return os;
}

// Вывод значения словаря
template <typename KeyElement, typename SubjElement>
ostream& operator<<(ostream& os, map<KeyElement, SubjElement> dict)
{
    os << "{";
    Print(os, dict);
    os << "}";
    return os;
}


template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const string& t_str, const string& u_str, const string& file,
    const string& func, unsigned line, const string& hint) {
    if (t != u) {
        cerr << boolalpha;
        cerr << file << "("s << line << "): "s << func << ": "s;
        cerr << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
        cerr << t << " != "s << u << "."s;
        if (!hint.empty()) {
            cerr << " Hint: "s << hint;
        }
        cerr << endl;
        abort();
    }
}

#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

void AssertImpl(bool value, const string& expr_str, const string& file, const string& func, unsigned line,
    const string& hint) {
    if (!value) {
        cerr << file << "("s << line << "): "s << func << ": "s;
        cerr << "ASSERT("s << expr_str << ") failed."s;
        if (!hint.empty()) {
            cerr << " Hint: "s << hint;
        }
        cerr << endl;
        abort();
    }
}

#define ASSERT(expr) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_HINT(expr, hint) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

/* Подставьте вашу реализацию класса SearchServer сюда */
const int MAX_RESULT_DOCUMENT_COUNT = 5;
const double DOUBLE_ACCURACY = 1e-6;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        }
        else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

struct Document {
    int id;
    double relevance;
    int rating;
};

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

// Вывод значения DocumentStatus
ostream& operator<<(ostream& os, DocumentStatus status)
{
    switch (status)
    {
    case DocumentStatus::ACTUAL:
        os << "ACTUAL";
        return os;
    case DocumentStatus::BANNED:
        os << "BANNED";
        return os;
    case DocumentStatus::IRRELEVANT:
        os << "IRRELEVANT";
        return os;
    case DocumentStatus::REMOVED:
        os << "REMOVED";
        return os;
    }
    return os;
}


class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document, DocumentStatus status,
        const vector<int>& ratings) {
        const vector<string> words = SplitIntoWordsNoStop(document);
        const double inv_word_count = 1.0 / words.size();
        for (const string& word : words) {
            word_to_document_freqs_[word][document_id] += inv_word_count;
        }
        documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
    }

    vector<Document> FindTopDocuments(const string& raw_query,
        DocumentStatus status = DocumentStatus::ACTUAL) const {
        return  FindTopDocuments(raw_query, [status]([[maybe_unused]] int document_id, DocumentStatus doc_status, [[maybe_unused]] int rating) {
            return status == doc_status; });
    }

    template <typename Filtr>
    vector<Document> FindTopDocuments(const string& raw_query,
        Filtr filtr) const {
        const Query query = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query, filtr);

        sort(matched_documents.begin(), matched_documents.end(),
            [](const Document& lhs, const Document& rhs) {
                if (abs(lhs.relevance - rhs.relevance) < DOUBLE_ACCURACY) {
                    return lhs.rating > rhs.rating;
                }
                else {
                    return lhs.relevance > rhs.relevance;
                }
            });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

    int GetDocumentCount() const {
        return static_cast<int> (documents_.size());
    }

    tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query,
        int document_id) const {
        const Query query = ParseQuery(raw_query);
        vector<string> matched_words;
        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)) {
                matched_words.push_back(word);
            }
        }
        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)) {
                matched_words.clear();
                break;
            }
        }
        return { matched_words, documents_.at(document_id).status };
    }

private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };

    set<string> stop_words_;
    map<string, map<int, double>> word_to_document_freqs_;
    map<int, DocumentData> documents_;

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    static int ComputeAverageRating(const vector<int>& ratings) {
        if (ratings.empty()) {
            return 0;
        }
        int rating_sum = 0;
        for (const int rating : ratings) {
            rating_sum += rating;
        }
        return rating_sum / static_cast<int>(ratings.size());
    }

    struct QueryWord {
        string data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(string text) const {
        bool is_minus = false;
        // Word shouldn't be empty
        if (text[0] == '-') {
            is_minus = true;
            text = text.substr(1);
        }
        return { text, is_minus, IsStopWord(text) };
    }

    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };

    Query ParseQuery(const string& text) const {
        Query query;
        for (const string& word : SplitIntoWords(text)) {
            const QueryWord query_word = ParseQueryWord(word);
            if (!query_word.is_stop) {
                if (query_word.is_minus) {
                    query.minus_words.insert(query_word.data);
                }
                else {
                    query.plus_words.insert(query_word.data);
                }
            }
        }
        return query;
    }

    // Existence required
    double ComputeWordInverseDocumentFreq(const string& word) const {
        return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
    }

    template <typename Filtr>
    vector<Document> FindAllDocuments(const Query& query, Filtr filtr) const {
        map<int, double> document_to_relevance;
        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
            for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
                const DocumentData& doc = documents_.at(document_id);
                if (filtr(document_id, doc.status, doc.rating)) {
                    document_to_relevance[document_id] += term_freq * inverse_document_freq;
                }
            }
        }

        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
                document_to_relevance.erase(document_id);
            }
        }
        vector<Document> matched_documents;
        for (const auto [document_id, relevance] : document_to_relevance) {
            matched_documents.push_back(
                { document_id, relevance, documents_.at(document_id).rating });
        }
        return matched_documents;
    }
};

// -------- Начало модульных тестов поисковой системы ----------

// Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов
void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = { 1, 2, 3 };
    // Сначала убеждаемся, что поиск слова, не входящего в список стоп-слов,
    // находит нужный документ
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }

    // Затем убеждаемся, что поиск этого же слова, входящего в список стоп-слов,
    // возвращает пустой результат
    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT(server.FindTopDocuments("in"s).empty());
    }
}

/*
Разместите код остальных тестов здесь*/
//Добавление документов. Добавленный документ должен находиться по поисковому запросу, который содержит слова из документа.
void TestDocumentSearching()
{
    const int doc_id = 27;
    const string content = "cat in the city"s;
    const vector<int> ratings = { 1, 2, 3 };
    // Сначала убеждаемся, что поиск не находит документ    
    SearchServer search_server;
    search_server.SetStopWords("и в на in the"s);
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
//Поддержка стоп-слов. Стоп-слова исключаются из текста документов.Это уже в примере есть

//Поддержка минус-слов. Документы, содержащие минус-слова поискового запроса, не должны включаться в результаты поиска.
void TestMinusWords()
{
    // Сначала убеждаемся, что поиск корректно находит документы без минус слов
    SearchServer search_server;
    search_server.SetStopWords("и в на in the"s);
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

    // Затем убеждаемся,то поиск находит меньше документов с запрососм, содержащий минус-слово         

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
    SearchServer search_server;
    search_server.SetStopWords("и в на in the"s);
    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });

    // проверяем работу проверки соответсвия документа поисковому запросу
    {
        const vector <string> query_test = { "евгений"s, "скворец"s };
        const auto& [query, docs_status] = search_server.MatchDocument("скворец     евгений   "s, 3);
        ASSERT_EQUAL(docs_status, DocumentStatus::BANNED);
        ASSERT_EQUAL(query, query_test);
    }

    // проверяем работу проверки соответсвия документов поисковому запросу при несоответствии
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
    SearchServer search_server;
        search_server.SetStopWords("и о в по на in the"s);
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
    SearchServer search_server;
    search_server.SetStopWords("и в на in the"s);
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
        //  Замечание:
        //  Рейтинг имеет целочисленный тип, поэтому их лучше сравнивать на точное равенство.
        //  Лучше использовать числовые формулы вида(1 + 2 + 3) / 3 вместо точных значений.
        //  Таким образом мы покажем каким образом были на самом деле получены данные значения, 
        //  что облегчит понимание теста для читателя.
        //     ASSERT(abs(doc0.rating - 5) < 1e-6);
        //     ASSERT(abs(doc1.rating - 2) < 1e-6);

    // Совсем забыл, что рейтинг у нас целочисленный, помню, что считается, 
    // как среднее значение, поэтому работал с ним, как с double. 
    // По поводу прозрачности проверок для человека понял, учту.
        ASSERT_EQUAL(doc0.rating, ((7 + 2 + 7) / 3)); // Проверяем расчёт рейтинга, как среднее целочисленное из оценок
        ASSERT_EQUAL(doc1.rating, ((8 - 3) / 2));    // Проверяем расчёт рейтинга, как среднее целочисленное из оценок
    }
}
//Фильтрация результатов поиска с использованием предиката, задаваемого пользователем.
void TestPredicateFunction()
{
    // Create test server
    SearchServer search_server;
    search_server.SetStopWords("и в на in the"s);
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
    SearchServer search_server;
    search_server.SetStopWords("и в на in the"s);
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
    SearchServer search_server;
    search_server.SetStopWords("и в на in the"s);
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
        // Замечание:
        // Лучше использовать числовые формулы вида(log(server.GetDocumentCount() * 1.0 / 1) * (2.0 / 3))) 
        // вместо точных значений для релевантности.
        //  Это покажет каким образом были получены данные значения, потому что, если тест провалится, 
        //  (будет просто видно, что числа различаются), тогда как числовые формулы покажут, 
        //  как автор теста вычислял эти значения, что поможет разобраться, где вычисления отличаются.
        //ASSERT(abs(doc0.relevance - 0.606503783) < 1e-6);
        //ASSERT(abs(doc1.relevance - 0.086643398) < 1e-6);

        ASSERT(abs(doc0.relevance - log(search_server.GetDocumentCount() * 1.0 / 2) * (1.0 / 4)) < 1e-6);
        ASSERT(abs(doc1.relevance - log(search_server.GetDocumentCount() * 1.0 / 2) * (1.0 / 5)) < 1e-6);    }


}

template <typename FunctionName>
void RunTestImpl(FunctionName function, const string& str_function)
{
    function();
    cerr << str_function << " OK" << endl;
}

#define RUN_TEST(func) RunTestImpl(func, #func) //макрос для запуска функций тестирования

// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestDocumentSearching);
    RUN_TEST(TestMinusWords);
    RUN_TEST(TestDocumentMatching);
    // Замечание:
    // Двух элементов объективно не хватает для того, чтобы адекватно оценивать то, 
    // что последовательность является отсортированной.Лучше иметь больше.
    // Также подсчёт релевантности и отсортированность результата по релевантности - 
    // различный функционал и для них стоит иметь два различных теста.
    // Функционал сортировки просто использует релевантность, но при этом никак не влияет на её подсчёт.
    // Также в тесте нужно сравнивать не на точные значения, а именно то, 
    // что значения отсортированы в невозрастающем порядке(то есть сравнивать релевантность соседних элементов).
    // RUN_TEST(TestSortByRelivance); проверяется функцией TestRelevanceSearchihgDocuments

    // Реализовал проверку сортировки по реливантности с пятью документами:
    RUN_TEST(TestSortByRelivance);

    RUN_TEST(TestDocumentRatingCalculation);
    RUN_TEST(TestPredicateFunction);
    RUN_TEST(TestSearchingDocumentByStatus);
    RUN_TEST(TestRelevanceSearchihgDocuments);
}

// --------- Окончание модульных тестов поисковой системы -----------
// 
// Спасибо, что дочитали до этого места. Благодарю за конструктивные замечания!
int main() {
    TestSearchServer();
    // Если вы видите эту строку, значит все тесты прошли успешно
    cout << "Search server testing finished"s << endl;
}
