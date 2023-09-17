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
            // 1439 запросов с нулевым результатом
            for (int i = 0; i < 1439; ++i) {
                request_queue.AddFindRequest("empty request"s);
            }
            // все еще 1439 запросов с нулевым результатом
            {LOG_DURATION_STREAM("Operation time", cerr);
            request_queue.AddFindRequest("curly dog"s); }
            // новые сутки, первый запрос удален, 1438 запросов с нулевым результатом
            {LOG_DURATION_STREAM("Operation time", cerr);
            request_queue.AddFindRequest("big collar"s); }
            // первый запрос удален, 1437 запросов с нулевым результатом
            {LOG_DURATION_STREAM("Operation time", cerr);
            request_queue.AddFindRequest("sparrow"s); }
            cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << endl;
        }
        {
            LOG_DURATION_STREAM("Checking RemoveDuplicates", cerr);
            SearchServer search_server("and with"s);

            search_server.AddDocument(1, "funny pet and nasty rat"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
            search_server.AddDocument(2, "funny pet with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });

            // дубликат документа 2, будет удалён
            search_server.AddDocument(3, "funny pet with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });

            // отличие только в стоп-словах, считаем дубликатом
            search_server.AddDocument(4, "funny pet and curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });

            // множество слов такое же, считаем дубликатом документа 1
            search_server.AddDocument(5, "funny funny pet and nasty nasty rat"s, DocumentStatus::ACTUAL, { 1, 2 });

            // добавились новые слова, дубликатом не является
            search_server.AddDocument(6, "funny pet and not very nasty rat"s, DocumentStatus::ACTUAL, { 1, 2 });

            // множество слов такое же, как в id 6, несмотря на другой порядок, считаем дубликатом
            search_server.AddDocument(7, "very nasty rat and not very funny pet"s, DocumentStatus::ACTUAL, { 1, 2 });

            // есть не все слова, не является дубликатом
            search_server.AddDocument(8, "pet with rat and rat and rat"s, DocumentStatus::ACTUAL, { 1, 2 });

            // слова из разных документов, не является дубликатом
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

    const auto dictionary = GenerateDictionary(generator, 50'000, 25);
    const auto documents = GenerateQueries(generator, dictionary, 1'000, 100);
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
}
