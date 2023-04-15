/* Решите загадку: Сколько чисел от 1 до 1000 содержат как минимум одну цифру 3?
// Напишите ответ здесь:
#include <iostream>
#include <string>

using namespace std;

bool IsThree (int number) // Проверяет есть ли в числе цифра 3
{
    string numstring = to_string(number);
    for (char c : numstring)
        if (c == '3')
            return true;
    return false;
}

int main() {
    int count = 0; //счётчик чисел
    for (int i = 1; i<=1000; ++i)
        if (IsThree(i))
            ++count; // Считаем числа с цифрой 3
    cout << count << endl;
}
// Насчитал 271 число
    
// Закомитьте изменения и отправьте их в свой репозиторий.*/

#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <cmath>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
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
        } else {
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
};

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document) {
        const vector<string> words = SplitIntoWordsNoStop(document);
        int w_size = words.size();
        for (const string& word : words)
        {
            documents_[word][document_id] = (static_cast<double>(count (words.begin(), words.end(), word))/static_cast<double>(w_size));           
        }
        ++document_count_;
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        const Query query = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query);

        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                 return lhs.relevance > rhs.relevance;
             });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

private:    
    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };
    
    map<string, map <int, double>> documents_; // <слово, <<id документов, в которых встречается слово, TF слава в документе> 
    
    set<string> stop_words_;
    
    int document_count_ = 0;

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

    Query ParseQuery(const string& text) const {
        Query query;
        for (const string& word : SplitIntoWordsNoStop(text)) {
            if (word.size() > 2 && word[0] == '-')
            {                
                query.minus_words.insert(word.substr(1));
            }
            else if (!IsStopWord(word))
                query.plus_words.insert(word);
        }
        return query;
    }

    vector<Document> FindAllDocuments(const Query& query) const {
        vector<Document> matched_documents;
        map<int, double> docs_relevance; // <id документа, релевантность документа (TF*IDF)>
        for (const string& word : query.plus_words) {
            if (documents_.count(word)>0) {
                double count_docs = static_cast<double>(documents_.at(word).size());
                for (const pair<int, double>& doc : documents_.at(word))
                    docs_relevance[doc.first] += doc.second * log(static_cast<double> (document_count_)/count_docs);
                }       
            }
        for (const string& word : query.minus_words) {
            if (documents_.count(word)>0)
                for (const pair<int, double>& doc : documents_.at(word))
                    if (docs_relevance.count(doc.first))
                        docs_relevance.erase(doc.first);
            }
        for (const pair<int, double>& doc : docs_relevance)
            matched_documents.push_back({doc.first, doc.second});
        return matched_documents;
    }   
};

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());

    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }

    return search_server;
}

int main() {
    const SearchServer search_server = CreateSearchServer();

    const string query = ReadLine();
    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", "
             << "relevance = "s << relevance << " }"s << endl;
    }
}