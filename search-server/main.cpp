<<<<<<< HEAD
#include "search_server.h"
#include "document.h"
#include "paginator.h"
#include "request_queue.h"
#include "testingss.h"
#include "string_processing.h"
#include "read_input_functions.h"
#include "log_duration.h"
#include "remove_duplicates.h"
#include "process_queries.h"
#include <random>



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


// ------ 9 sprint ---------

string GenerateWord(mt19937& generator, int max_length) {
    const int length = uniform_int_distribution(1, max_length)(generator);
    string word;
    word.reserve(length);
    for (int i = 0; i < length; ++i) {
        word.push_back(uniform_int_distribution(0, 26)(generator) + 'a');
    }
    return word;
}
vector<string> GenerateDictionary(mt19937& generator, int word_count, int max_length) {
    vector<string> words;
    words.reserve(word_count);
    for (int i = 0; i < word_count; ++i) {
        words.push_back(GenerateWord(generator, max_length));
    }
    sort(words.begin(), words.end());
    words.erase(unique(words.begin(), words.end()), words.end());
    return words;
}
string GenerateQuery(mt19937& generator, const vector<string>& dictionary, int max_word_count) {
    const int word_count = uniform_int_distribution(1, max_word_count)(generator);
    string query;
    for (int i = 0; i < word_count; ++i) {
        if (!query.empty()) {
            query.push_back(' ');
        }
        query += dictionary[uniform_int_distribution<int>(0, dictionary.size() - 1)(generator)];
    }
    return query;
}
vector<string> GenerateQueries(mt19937& generator, const vector<string>& dictionary, int query_count, int max_word_count) {
    vector<string> queries;
    queries.reserve(query_count);
    for (int i = 0; i < query_count; ++i) {
        queries.push_back(GenerateQuery(generator, dictionary, max_word_count));
    }
    return queries;
}
template <typename QueriesProcessor>
void Test(string_view mark, QueriesProcessor processor, const SearchServer& search_server, const vector<string>& queries) {
    LOG_DURATION(mark);
    const auto documents_lists = processor(search_server, queries);
}
#define TEST(processor) Test(#processor, processor, search_server, queries)


template <typename ExecutionPolicy>
void TestRD(string_view mark, SearchServer search_server, ExecutionPolicy&& policy) {
    LOG_DURATION(mark);
    const int document_count = search_server.GetDocumentCount();
    for (int id = 0; id < document_count; ++id) {
        search_server.RemoveDocument(policy, id);
    }
    cout << search_server.GetDocumentCount() << endl;
}

#define TEST_RD(mode) TestRD(#mode, search_server, execution::mode)

void TestS(string_view mark, SearchServer search_server) {
    LOG_DURATION(mark);
    const int document_count = search_server.GetDocumentCount();
    for (int id = 0; id < document_count; ++id) {
        search_server.RemoveDocument(id);
    }
    cout << search_server.GetDocumentCount() << endl;
}
#define TEST_S(mode) TestS(#mode, search_server)

//---------MatchDocument---------------
string GenerateQuery(mt19937& generator, const vector<string>& dictionary, int word_count, double minus_prob = 0) {
    string query;
    for (int i = 0; i < word_count; ++i) {
        if (!query.empty()) {
            query.push_back(' ');
        }
        if (uniform_real_distribution<>(0, 1)(generator) < minus_prob) {
            query.push_back('-');
        }
        query += dictionary[uniform_int_distribution<int>(0, dictionary.size() - 1)(generator)];
    }
    return query;
}

template <typename ExecutionPolicy>
void Test_md(string_view mark, SearchServer search_server, const string& query, ExecutionPolicy&& policy) {
    LOG_DURATION(mark);
    const int document_count = search_server.GetDocumentCount();
    int word_count = 0;
    for (int id = 0; id < document_count; ++id) {
        const auto [words, status] = search_server.MatchDocument(policy, query, id);
        word_count += words.size();
    }
    cout << word_count << endl;
}

#define TEST_MD(policy) Test_md(#policy, search_server, query, execution::policy)

void Test_mds(string_view mark, SearchServer search_server, const string& query) {
    LOG_DURATION(mark);
    const int document_count = search_server.GetDocumentCount();
    int word_count = 0;
    for (int id = 0; id < document_count; ++id) {
        const auto [words, status] = search_server.MatchDocument(query, id);
        word_count += words.size();
    }
    cout << word_count << endl;
}

#define TEST_MDS(policy) Test_mds(#policy, search_server, query)

template <typename ExecutionPolicy>
vector<double> TestFTD(string_view mark, const SearchServer& search_server, const vector<string>& queries, ExecutionPolicy&& policy) {
    LOG_DURATION(mark);
    double total_relevance = 0;
    vector<double> result;
    int doc_counts = 0;
    for (const string_view query : queries) {
        for (const auto& document : search_server.FindTopDocuments(policy, query)) {
            total_relevance += document.relevance;
            ++doc_counts;
            result.push_back(document.relevance);            
        }
    }
    cout << "Found docs "s << doc_counts << endl << total_relevance << endl;
    return result;
}
#define TEST_FTD(policy) TestFTD(#policy, search_server, queries, execution::policy)

vector<double> TestFTDS(string_view mark, const SearchServer& search_server, const vector<string>& queries) {
    LOG_DURATION(mark);
    double total_relevance = 0;
    int doc_counts = 0;
    vector<double> result;
    for (const string_view query : queries) {
        for (const auto& document : search_server.FindTopDocuments(query)) {
            total_relevance += document.relevance;
            ++doc_counts;
            result.push_back(document.relevance);
          
        }
    }
    cout << "Found docs "s << doc_counts << endl << total_relevance << endl; 
    return result;
}
#define TEST_FTDS(policy) TestFTDS(#policy, search_server, queries)

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
          
        }
     }

    // ------ 9 sprint ---------

    {/*
        LOG_DURATION_STREAM("SearchServer generates and tests in ", cerr);
        mt19937 generator;
        const auto dictionary = GenerateDictionary(generator, 2'000, 25);
        const auto documents = GenerateQueries(generator, dictionary, 20'000, 10);
        SearchServer search_server(dictionary[0]);
        for (size_t i = 0; i < documents.size(); ++i) {
            search_server.AddDocument(i, documents[i], DocumentStatus::ACTUAL, { 1, 2, 3 });
        }
        const auto queries = GenerateQueries(generator, dictionary, 2'000, 7);

        TEST(ProcessQueries);
        TEST(ProcessQueriesSeq);
        TEST(ProcessQueriesJoined);*/
    }

    
    {
        LOG_DURATION("ProcessQueriesJoined ");
        SearchServer search_server("and with"s);
        int id = 0;
        for (
            const string& text : {
                "funny pet and nasty rat"s,
                "funny pet with curly hair"s,
                "funny pet and not very nasty rat"s,
                "pet with rat and rat and rat"s,
                "nasty rat with curly hair"s,
            }
            ) {
            search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, { 1, 2 });
        }
        const vector<string> queries = {
            "nasty rat -not"s,
            "not very funny nasty pet"s,
            "curly hair"s
        };
        for (const Document& document : ProcessQueriesJoined(search_server, queries)) {
            cout << "Document "s << document.id << " matched with relevance "s << document.relevance << endl;
        }
    }

    
    mt19937 generator;

    const auto dictionary = GenerateDictionary(generator, 20'000, 25);
    const auto documents = GenerateQueries(generator, dictionary, 10'000, 100);
    SearchServer search_server(dictionary[0]);
    for (size_t i = 0; i < documents.size(); ++i) 
    {
        search_server.AddDocument(i, documents[i], DocumentStatus::ACTUAL, { 1, 2, 3 });
    }  
    const auto queries = GenerateQueries(generator, dictionary, 100, 70);
    cout << "-------------"s << endl;
    {
        LOG_DURATION_STREAM("ProcessQueries tests ", cerr);
        TEST(ProcessQueries);
        TEST(ProcessQueriesSeq);
        TEST(ProcessQueriesJoined);
    }
    
    const string query = GenerateQuery(generator, dictionary, 5000, 0.1);
    cout << "-------------"s << endl;
    {
        LOG_DURATION("MatchDocument tests ");
        //TEST_MDS(simple);
        TEST_MD(seq);
        TEST_MD(par);
    }
   
    cout << "-------------"s << endl;
    {
        LOG_DURATION("FindTopDocuments tests ");
        vector<double> simple = TEST_FTDS(simple);
        vector<double> seqpol = TEST_FTD(seq);
        vector<double> parpol = TEST_FTD(par);       
    }
    cout << "-------------"s << endl;
    {
        LOG_DURATION("RemoveDocument tests "); TEST_S(simple);
        TEST_RD(seq);
        TEST_RD(par);
    }
    return 0;
=======


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
//  ЗАМЕЧАНИЕ: также не стоит нагружать проверками метод разбивки на слова.
//  требования с проверками - это требования класса, без этих проверок эту функцию можно 
//  будет использовать и в других проектах
//  ПОЯСНЕНИЕ: Вернул функцию на место, без проверок.
static vector<string> SplitIntoWords(const string& text)
{
    vector<string> words;
    string word;
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

class SearchServer
{
public:
    template <typename StopWords>
    explicit SearchServer(const StopWords& stop_words)
    {
        for (const string& word : stop_words)
        {  
            if (word.empty() || !IsValidWord(word))
            {
                throw invalid_argument("Invalid stop-word"s);
            }
            stop_words_.insert(word);
        }
    }
 
    explicit SearchServer(const string& raw_stop_words)
        : SearchServer(SplitIntoWords(raw_stop_words)) {}
    
    void AddDocument(int document_id, const string& document, DocumentStatus status,
        const vector<int>& ratings)
    {
        if (document_id < 0)
        {
            throw invalid_argument("Invalid document ID"s);
        }
        if (IsDocumentID(document_id))
        {
            throw invalid_argument("Document ID is already exist"s);
        }        
            const vector<string> words = SplitIntoWordsNoStop(document);

            const double inv_word_count = 1.0 / words.size();
            for (const string& word : words)
            {
                if (!IsValidWord(word))
                {
                    throw invalid_argument("Invalid document word"s);
                }
                word_to_document_freqs_[word][document_id] += inv_word_count;
            }
            documents_.emplace(document_id, DocumentData(ComputeAverageRating(ratings), status));
            document_id_chain_.push_back(document_id);
    }
 //     ЗАМЕЧАНИЕ: так как вы пытаетесь проверить сразу всю строку, то обработка сложной получилась.
 //     если будете проверять отдельные слова, то гораздо проще всё будет
 //     (вам ведь надо только начальные символы проверять)
 //     ПОЯСНЕНИЕ: Всё переделал в этой части
    //Проверка строки на отсутствие спецсимволов
    static bool IsValidWord(const string& text)
    {                
        for (const char& ch : text)
        {
            if (!IsValidChar(ch))
            {
                return false;
            }
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
//  УТОЧНЕНИЕ: можно не перебрасывать исключения, так как оно и так будет доброшено до самого верхнего уровня
//  ПОЯСНЕНИЕ: Кажется везде убрал проброс. Но мне понравилась концепция 
//  вывести всю цепочку вызова функция в проекте, чтобы сразу было видно, что привело к исключению.
//  В совокупности с этим вашим уточнением я теперь лучше понимаю механизм исключений.
    }

    int GetDocumentId(const int index) const
    {
        if (index >= static_cast<int>(documents_.size()))
            throw out_of_range("Document ID out of range"s);
        if (index < 0)
            throw out_of_range("Document ID is negative"s);
//  УТОЧНЕНИЕ: отлично, хотя можно еще лучше, если использовать не квадратные скобки, 
//  а использовать at(https://en.cppreference.com/w/cpp/container/vector/at), 
//  который сам справится со всем и выбросит исключения (то есть проверки реализовывать не надо будет)
//  ПОЯСНЕНИЕ: Спасибо. Буду стараться делать лучше. Исключения оставил, так как ошибка будет более информативной
        return document_id_chain_.at(index); //считаем порядок добавления от 0
    }

    int GetDocumentCount() const {
        return static_cast<int> (documents_.size());
    }

    tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query,
        int document_id) const    
    {   
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

private:
    struct DocumentData 
    {
        DocumentData() : rating(0), status(DocumentStatus::ACTUAL) {};
        DocumentData(int rate, DocumentStatus status)
            : rating(rate), status(status) {};
        int rating;
        DocumentStatus status;
    };

    vector<int> document_id_chain_; //содержит ID документов в последовательности их добавления на сервер
    set<string> stop_words_; //База стоп-слов
    map<string, map<int, double>> word_to_document_freqs_;  //словарь, где для каждого слова на сервере храниться ID документов, в которых слово встречается и IDF слова для документа
    map<int, DocumentData> documents_; //словарь ID документов с информацией о рейтинге и статусе
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
   //Из прошлого варианта: //  Замечание:    а где проверка на минусы слов запроса 
                           //  Пояснение:  В функции IsValidWord у меня введена проверка 
                           // на корректность сочетаний минусов в тексте 
 // ЗАМЕЧАНИЕ:   вот это не очень хорошо, потому что на минусы вы проверяете только запросы, 
 // а IsValidWord используете не только для них.
 // а еще усложняя функцию вы  делаете сложным код и её использование.
 // ориентируйтесь на создание более простых функций.
 // вот есть задача "проверить на спец символы" - пусть функция только спец символы проверяет,
 //  надо проверить на минусы, если хотите создать отдельную функцию для этого, 
 // то пусть она отдельной будет(передайте параметром туда отдельное слово и верните результат)
 // 
 // ПОЯСНЕНИЕ: Все замечания учёл. Но не во всём я согласен. 
 // В данной реализации не проверяются слова документов на корректность, 
 // а слова запроса проверяются, складывается ситуация при которой в документах могут существовать слова, 
 // которые невозможно найти, так как такой поисковый запрос будет признан некорректным.
 // На мой взгляд, документы при добавлении должны пройти такую эе проверку, как и запросы.
 // Именно поэтому я в предыдущий раз включил все проверки в функцию SplitIntoWords, 
 // ну и соответственно она стала принадлежностью класса.
 // Получается что добавить на сервер слово "кот-" я могу, а найти его не смогу.
 // В любом случае задача учебная, лишний раз что-то переделать - это тренировка.
 // Спасибо большое за терпение!
        QueryWord ParseQueryWord(string text) const 
    {
        bool is_minus = false;
        if ((text.empty() ||
            (text[0] == '-' && text.size() == 1)) || 
            (text[0] == '-' && text[1] == '-') ||
            (text[text.size()-1] == '-'))
        {
            throw invalid_argument("Invalid query word"s);
        }
        
        if (text[0] == '-') 
        {
            is_minus = true;
            text = text.substr(1);
        }
        return { text, is_minus, IsStopWord(text) };
    }

    struct Query 
    {
        set<string> plus_words;
        set<string> minus_words;
    };

    Query ParseQuery(const string& text) const {
        
            Query query;
            for (const string& word : SplitIntoWords(text)) {
                if (!IsValidWord(word))
                {
                    throw invalid_argument("Invalid query word"s);
                }
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
        ASSERT_EQUAL(search_server.GetDocumentId(i), test_chain.at(i));
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
        SearchServer search_server("C++\0\2"s);
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
            search_server.AddDocument(40, "белый модный \2\0 -ошейник"s, DocumentStatus::ACTUAL, { 1 });
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
>>>>>>> b9fe6ec093be2c493e63afd443c89ee752860ea5
}
