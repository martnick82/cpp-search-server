

#include <iostream>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <map>
#include <set>
#include <string>
#include <vector>

using namespace std;

// Перегрузка операторов << для вывода сообщений об ошибках тестирования
// Вывод значения пары
template <typename KeyElement, typename SubjElement>
ostream& operator<<(ostream& os, const pair<KeyElement, SubjElement>& map_element)
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
ostream& operator<<(ostream& os, const vector<Element>& vect)
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
//Функции и макросы для юнит-тестов
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

const int MAX_RESULT_DOCUMENT_COUNT = 5;
const double DOUBLE_ACCURACY = 1e-6;

//    -------------------- Реализация SearchServer --------------------



struct Document 
{
    Document() : id(0), relevance(0.0), rating(0) {}

    Document(int ID, double rel, int rate)
        : id(ID), relevance(rel), rating(rate) {}

    int id;
    double relevance;
    int rating;
};

enum class DocumentStatus 
{
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

// Перегрузка оператора << для вывода значения DocumentStatus в юнит-тестах
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


class SearchServer 
{
public:
    template <typename StopWords>
    explicit SearchServer(const StopWords& stop_words)
    {
        for (const string& word : stop_words)
        {
 //     Уточнение:       даже одну операцию стоит обрамлять в фигурные скобки https ://habr.com/ru/articles/61323/
//      Пояснение: Спасибо. с интересом прочёл. Понял, что единого мнения на этот счёт нет. 
//      Такие замечания очень ценны на мой взгляд. А за ссылку отдельное спасибо!
//      Здесь я всё не поправлю, но постараюсь следовать в дальнейшем
                if (word.empty() || !IsValidWord(word))
                {
                    throw invalid_argument("Invalid stop-word"s);
                }
                stop_words_.insert(word);
        }
    }
//  Уточнение: метод SplitIntoWords формирует контейнер слов, с которым работает шаблонный конструктор, 
//  можно делегировать работу ему через список инициализации

 // Пояснение: Не помню такой информации в теории. Благодаря вашему замечанию поискал, и реализовал так:
    explicit SearchServer(const string& raw_stop_words)
        : SearchServer(SplitIntoWords(raw_stop_words)) {}

//  Так конечно намного лучше. Моя предыдущая реализация мне самому не нравилась из-за дублирования кода, 
//  но не получилось у меня реализовать имеющимися знаниями, наверное где-то я упустил, 
//  что можно прям целые функции в список инициализации записывать. 
//  Даже непонятно было, чем список инициализации лучше, чем значения полей по-умолчанию, 
//  так как примеры в теории на простых данных приведены.
       
    void AddDocument(int document_id, const string& document, DocumentStatus status,
        const vector<int>& ratings) 
    {        
        if  (document_id < 0)
        {
            throw invalid_argument("Invalid document ID"s);
        }
        if (IsDocumentID(document_id))
        {
            throw invalid_argument("Document ID is already exist"s);
        }
        try {
            const vector<string> words = SplitIntoWordsNoStop(document);        
        
        const double inv_word_count = 1.0 / words.size();
        for (const string& word : words) 
        {
            word_to_document_freqs_[word][document_id] += inv_word_count;
        }
        documents_.emplace(document_id, DocumentData(ComputeAverageRating(ratings), status));   

        // Переменная для хранения последовательности добавления документов, пояснения ниже
        document_id_chain_.push_back(document_id);
        }
        catch (invalid_argument& error)
        {
            throw invalid_argument("SplitIntoWordsNoStop :: "s + error.what());
        }
    }
   
    //Проверка строки на отсутствие спецсимволов и корректность дефисов
    static bool IsValidWord(const string& text)
    {
        char previous_char = ' ';
        if (text.size() == 1 && text == "-")
        {
            return false;
        }
        for (const char& ch : text)
        {
            if (!IsValidChar(ch))
            {
                return false;
            }
            if ((ch == '-' && previous_char == '-') || (ch == ' ' && previous_char == '-'))
            {
                return false;
            }
            previous_char = ch;
        }
        if (previous_char == '-')
        {
            return false;
        }
        return true;
    }
    static bool IsValidChar(const char& c)
    {
        return !(c >= '\0' && c < ' ');
    }

    vector<Document> FindTopDocuments(const string& raw_query) const 
    {
        return  FindTopDocuments(raw_query, []([[maybe_unused]] int document_id, 
            DocumentStatus status, [[maybe_unused]] int rating) 
            {
            return status == DocumentStatus::ACTUAL; 
            });
    }

    static vector<string> SplitIntoWords(const string& text) 
    {
        vector<string> words;
        string word;
        if (!IsValidWord(text))
        {
            throw invalid_argument("Invalid argument."s);
        }
        for (const char c : text) 
        {
            if (c == ' ') 
            {
                if (!word.empty()) 
                {
                    words.push_back(word);
                    word.clear();
                }
            }
            else 
            {
                word += c;
            }
        }
        if (!word.empty()) 
        {
            words.push_back(word);
        }

        return words;
    }


    vector<Document> FindTopDocuments(const string& raw_query,
        DocumentStatus status) const 
    {
        return  FindTopDocuments(raw_query, [status]([[maybe_unused]] int document_id, 
            DocumentStatus doc_status, [[maybe_unused]] int rating) 
            {
            return status == doc_status; 
            });
    }

    template <typename Filtr>
    vector<Document> FindTopDocuments(const string& raw_query,
        Filtr filtr) const 
    {
//Замечание:   каждый раз перед парсингом запроса делаете проверку - 
//       можно перенести вовнутрь метода и не будет этого дублирования
//Пояснение: Проверку IsValidWord перенёс в метод SplitIntoWords и обработку исключений протащил наружу
        try {            
            Query query = ParseQuery(raw_query);
            auto matched_documents = FindAllDocuments(query, filtr);
            sort(matched_documents.begin(), matched_documents.end(),
                [](const Document& lhs, const Document& rhs) 
                {
                    if (abs(lhs.relevance - rhs.relevance) < DOUBLE_ACCURACY) 
                    {
                        return lhs.rating > rhs.rating;
                    }
                    else {
                        return lhs.relevance > rhs.relevance;
                    }
                });
            if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) 
            {
                matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
            }
            return matched_documents;
        }
        catch (invalid_argument& error) 
        {
            throw  invalid_argument("ParseQuery :: "s + error.what());
        }
    }

 /*   int GetDocumentId(const int index) const
    {
 // Замечание:       не надо в одну строку размещать переменные и присваивать друг друга,
 //       int id = INVALID_DOCUMENT_ID, inc = 0;
    Пояснение: Понял. Читаемость хуже.
        int id = -1; 
        int inc = 0;
        if (index >= static_cast<int>(documents_.size()))
            throw out_of_range("Document ID out of range"s);
        if (index < 0)
            throw out_of_range("Document ID is negative"s);
//  Замечание: словарь упорядочивает значения, а значит не будет работать при неупорядоченном добавлении.
//  вам надо еще один контейнер для хранения идентификаторов в порядке добавления

//  Пояснение: В задании сказано было следующее: 
//  "Также добавьте метод GetDocumentId, позволяющий получить идентификатор документа по его порядковому номеру."
//  Не пояснялось, что это номер в порядке добавления документов, поэтому я реализовал поиск по порядку в словаре,
//  Тестирование в тренажёре прошло, так что я полагал, что решил задание верно, исходя из того, 
//  что, вероятно, тесты учитывают логику задания. Соответственно ниже в функции TestGetDocumentId(), проверялась
//  именно такая логика, что документы сортируются по ID вне зависимости от порядка их добавления.
//  Но пока я только учусь, так что, раз есть замечание, то буду переделывать. Тяжело в учении, легко на работе :)

        for (const auto& [doc_id, data] : documents_)
        {
            if (inc == index)
            {
                id = doc_id;
                break;
            }
            ++inc;
        }
        return id;
    } */
//Новый вариант:
    int GetDocumentId(const int index) const
    {
        if (index >= static_cast<int>(documents_.size()))
            throw out_of_range("Document ID out of range"s);
        if (index < 0)
            throw out_of_range("Document ID is negative"s);
        return document_id_chain_[index]; //считаем порядок добавления от 0
    }

    int GetDocumentCount() const {
        return static_cast<int> (documents_.size());
    }

    tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query,
        int document_id) const    
    {        
//  Замечание:    в матчинге уже не надо проверять идентификаторы
//        if (document_id < 0)
//           throw invalid_argument("Invalid document ID"s);
//        if (!IsDocumentID(document_id))
//            throw invalid_argument("Non existing document ID"s);
//  Пояснение: убрать не сложно. Интересно почему в матчинге не обязательно проверять входные данные?
//  Исходил из того, что есть функция, которая принимает параметры, от которых зависит работа функции, 
//  поэтому для стабильности работы было бы неплохо эти параметры проверить. Так, без проверки, 
//  в дальнейшем возможна попытка обращения по невалидному ID
            try {
                const Query query = ParseQuery(raw_query);
                vector<string> matched_words;
                for (const string& word : query.plus_words) 
                {
                    if (word_to_document_freqs_.count(word) == 0) 
                    {
                        continue;
                    }
                    if (word_to_document_freqs_.at(word).count(document_id)) 
                    {
                        matched_words.push_back(word);
                    }
                }
                for (const string& word : query.minus_words) 
                {
                    if (word_to_document_freqs_.count(word) == 0) 
                    {
                        continue;
                    }
                    if (word_to_document_freqs_.at(word).count(document_id)) 
                    {
                        matched_words.clear();
                        break;
                    }
                }
                return tie(matched_words, documents_.at(document_id).status);
            }
            catch (invalid_argument& error) {
                throw  invalid_argument("ParseQuery :: "s + error.what());
            }
    }
//  Уточнение:    вам надо обойтись уже без этой переменной
//    const static int INVALID_DOCUMENT_ID = -1;
//  Пояснение: Да, согласен. Убрал в данной реализации. Я видел это, просто оставил на память о предыдущих уроках :)

private:
    struct DocumentData 
    {
        DocumentData() : rating(0), status(DocumentStatus::ACTUAL) {};
        DocumentData(int rate, DocumentStatus status)
            : rating(rate), status(status) {};
        int rating;
        DocumentStatus status;
    };
/* Пояснение: Пока я не очень понимаю назначение функции int GetDocumentId(const int index), то реализую замечание,
* касающееся её так, чтобы наименьшим образом затрагивать структуру сервера. Если не рассматривать это моё замечание,
* то сделал бы так:
* 
* чтобы не плодить новых переменных, я бы ввёл ещё одно поле в
* struct DocumentData {
        ...
        int chain_link; // номер документа в порядке его добавления на сервер
    };

    Тогда в функции AddDocument изменяется одна строка:
* ...
* void AddDocument(int document_id, const string& document, DocumentStatus status,
        const vector<int>& ratings) {
        ...
        documents_.emplace(document_id, DocumentData(ComputeAverageRating(ratings), status, GetDocumentCount()+1));   
        ...
    }
    ...
    Поиск номера был бы таким:
     int GetDocumentId(const int index) const
    {            
        int id = -1;
        if (index >= static_cast<int>(documents_.size()))
            throw out_of_range("Document ID out of range"s);
        if (index < 0)
            throw out_of_range("Document ID is negative"s);
        for (const auto& [doc_id, data] : documents_)
        {
            if (data.chain_link == index)
            {
                id = doc_id;
                break;
            }           
        }
        return id;
    } // Извините, из-за нехватки времени не проверил работу предложенного кода.
* Изменение struct DocumentData требует проверки и изменений всех функций, где мы с ней работаем
* Но чтобы пока не переделывать, введу новую переменную в состав сервера, может оно и не хуже:
*/
    vector<int> document_id_chain_; // содержит ID документов в последовательности их добавления на сервер

    set<string> stop_words_;
    map<string, map<int, double>> word_to_document_freqs_;
    map<int, DocumentData> documents_;
    bool IsStopWord(const string& word) const 
    {
        return stop_words_.count(word) > 0;
    }

    bool IsDocumentID(const int doc_id) const
    {
        return documents_.count(doc_id) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const 
    {
        vector<string> words;
        try {
            for (const string& word : SplitIntoWords(text)) 
            {
                if (!IsStopWord(word)) 
                {
                    words.push_back(word);
                }
            }
        }
        catch (invalid_argument& error) {
            throw invalid_argument("SplitIntoWords : "s + error.what());
        }
        return words;
    }

    static int ComputeAverageRating(const vector<int>& ratings) 
    {
        if (ratings.empty()) 
        {
            return 0;
        }
        int rating_sum = 0;
        for (const int rating : ratings) 
        {
            rating_sum += rating;
        }
        return rating_sum / static_cast<int>(ratings.size());
    }

    struct QueryWord 
    {
        string data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(string text) const 
    {
        bool is_minus = false;
        // Word shouldn't be empty
        if (text[0] == '-') 
        {
            is_minus = true;
            text = text.substr(1);
        }
        return { text, is_minus, IsStopWord(text) };
    }
//  Замечание:    а где проверка на минусы слов запроса
//  Пояснение:  В функции IsValidWord у меня введена проверка на корректность сочетаний минусов в тексте
//  поэтому здесь не добавлял никаких проверок. Тесты в тренажёре данное решение прошло. И сам я проверяю.
    struct Query 
    {
        set<string> plus_words;
        set<string> minus_words;
    };

    Query ParseQuery(const string& text) const {
        try {
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
        catch (invalid_argument& error) {
            throw  invalid_argument("SplitIntoWords :: "s + error.what());
        }
    }
        
    double ComputeWordInverseDocumentFreq(const string& word) const 
    {
        return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
    }

    template <typename Filtr>
    vector<Document> FindAllDocuments(const Query& query, Filtr filtr) const {
        map<int, double> document_to_relevance;
        for (const string& word : query.plus_words) 
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

        for (const string& word : query.minus_words) 
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
        vector<Document> matched_documents;
        for (const auto [document_id, relevance] : document_to_relevance) {
            matched_documents.push_back(
                { document_id, relevance, documents_.at(document_id).rating });
        }
        return matched_documents;
    }
};

// Это было интересно)) Спасибо за рекомендации и замечания! Извиняюсь за много букв. 
// Хорошо ещё у меня времени нет, эти суббота с воскресением выдались рабочими)))

// --------- Начало модульных тестов поисковой системы -----------

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
//Тест функции возврата ID документа по его порядковому номеру
void TestGetDocumentId()
{
    SearchServer search_server("и в на in the"s);
    search_server.AddDocument(0, "белый кот и красивый модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(30, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(10, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(20, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });
    vector<int> test_chain = { 0, 30, 10, 20 };
    // Проверяем каждый ID
    ASSERT_EQUAL(search_server.GetDocumentCount(), 4);
    for (int i = 0; i < search_server.GetDocumentCount(); ++i)
        ASSERT_EQUAL(search_server.GetDocumentId(i), test_chain[i]);
}

/* пока оставлю старую версию здесь
void TestGetDocumentId()
{
    SearchServer search_server("и в на in the"s);
    search_server.AddDocument(0, "белый кот и красивый модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(30, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(10, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(20, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });
    
    // Проверяем каждый ID
    ASSERT_EQUAL(search_server.GetDocumentCount(), 4);
    for (int i = 0; i < search_server.GetDocumentCount(); ++i)
        ASSERT_EQUAL(search_server.GetDocumentId(i), i * 10);
}
*/
//Тест работы исключений
void TestException()
{
    //Test invalid stop-words string
    try {
        SearchServer search_server("C++--"s);
        cerr << "Testing invalid stop-words string"<<  " --- TEST FAILED ---"s << endl;
    }
    catch (const invalid_argument& error) 
    { cerr << "Don't worry. Its just testing stop-words string: "s << error.what() << " --- TEST OK --- "s << endl; };

    try {
        vector<string> stop_words = { "-"s, "\0"s, "--"s };
        SearchServer search_server(stop_words);
        cerr << "Testing invalid stop-words vector" << " --- TEST FAILED --- "s << endl;
    }
    catch (const invalid_argument& error)
    {
        cerr << "Don't worry. Its just testing stop-words vector: "s << error.what() <<  " --- TEST OK ---"s << endl;
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
            search_server.AddDocument(40, "белый модный --ошейник"s, DocumentStatus::ACTUAL, { 1 });
            cerr << "Testing invalid text in AddDocument " << " --- TEST FAILED ---"s << endl;
        }
        catch (const invalid_argument& error)
        {
            cerr << "Don't worry. Its just testing AddDocument: "s << error.what() << " --- TEST OK --- "s << endl;
        }

// Test invalid ID in GetDocumentId
    try {
        cout << search_server.GetDocumentId(-5);
        cerr << "Testing negative ID " << " --- TEST FAILED ---"s << endl;
    }
    catch (const out_of_range& error)
    {
        cerr << "Don't worry. Its just testing GetDocumentId: "s 
            << error.what() << " --- TEST OK --- "s << endl;
    }

    try {
        cout << search_server.GetDocumentId(7);
        cerr << "Testing out of range ID" << " --- TEST FAILED ---"s << endl;
    }
    catch (const out_of_range& error)
    {
        cerr << "Don't worry. Its just testing GetDocumentId: "s << error.what() << " --- TEST OK --- "s << endl;
    }
// Test invalid words in query FindTopDocuments
    try {
        search_server.FindTopDocuments( "белый модный \20 ошейник"s);
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
/*
    //Test negative ID in MatchDocument
    try {
        const auto result = search_server.MatchDocument("белый ошейник"s, -5);
        cerr << "Testing negative ID in MatchDocument " << " --- TEST FAILED ---"s << endl;
    }
    catch (const invalid_argument& error)
    {
        cerr << "Don't worry. Its just testing negative ID in MatchDocument: "s 
            << error.what() << " --- TEST OK --- "s << endl;
    }

    //Test non existing ID in MatchDocument
    try {
        const auto result = search_server.MatchDocument("белый ошейник"s, 55);
        cerr << "Testing non existing ID in MatchDocument " << " --- TEST FAILED ---"s << endl;
    }
    catch (const invalid_argument& error)
    {
        cerr << "Don't worry. Its just testing non existing ID in MatchDocument: "s 
            << error.what() << " --- TEST OK --- "s << endl;
    }    */
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
    RUN_TEST(TestSortByRelivance);
    RUN_TEST(TestDocumentRatingCalculation);
    RUN_TEST(TestPredicateFunction);
    RUN_TEST(TestSearchingDocumentByStatus);
    RUN_TEST(TestRelevanceSearchihgDocuments);
    RUN_TEST(TestGetDocumentId);
    RUN_TEST(TestException);
}
// --------- Окончание модульных тестов поисковой системы -----------

void PrintDocument(const Document& document) {
    cout << "{ "s
        << "document_id = "s << document.id << ", "s
        << "relevance = "s << document.relevance << ", "s
        << "rating = "s << document.rating << " }"s << endl;
}
int main() {
    TestSearchServer();
    // Если вы видите эту строку, значит все тесты прошли успешно
    cout << "Search server testing finished"s << endl;
}
