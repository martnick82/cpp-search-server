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
// ЗАМЕЧАНИЕ: count также как и цикл проходит по всем словам, что не эффективно, к тому же при повторных словах вы будете заново все пересчитывать. проще вычислить до цикла значение веса одного слова и затем прибавлять к ячейке
   /*   for (const string& word : words) 
        { 
            documents_[word][document_id] = (static_cast<double>(count (words.begin(), words.end(), word))/static_cast<double>(w_size));
        }*/
// Согласен, поторопился и не проанализировал структуру вычислений, сделал "в лоб", вынес только вычесление w_size за цикл. Исправил по замечаниям:
        double one_TF = 1.0 / words.size();        
        for (const string& word : words)
        {
            documents_[word][document_id] += one_TF;           
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
// КОМЕНТАРИЙ: можно улучшить и выделить метод обработки слов запроса, который в структурированном виде вернул бы информацию - само слова, признак минуса, признак стопа
/* Пока не стал исправлять, так как обработка выполняется только в одном месте, не вижу большого смысла в создании структуры, подозреваю правда, что в будущем структура пригодится, придётся переделывать. Изначально сделал отдельную функцию для выделения минус-слов, но потом запихнул всё сюда, видимо нужно мозги перестраивать, чтобы делать более читаемый код. 
Сделал проверку if (word.size() > 2 && word[0] == '-') (в авторском решении if (text[0] == '-')) для общего случая, так как в строке помимо минус-слов может быть отдельное слово "-", понимаю, что в нашем случае оно не несёт информации и должно попасть в стоп-слова, но тогда оно отсеется позже в else if (!IsStopWord(word)) */
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
    //ЗАМЕЧАНИЕ: идф не зависит от слова и от идентификатора, нет смысла на каждом витке его вычислять, это можно сделать до цикла. а еще лучше, если организуете отдельную функцию для вычисления ИДФ
    // Выделил вычесление IDF в отдельную функцию. Изначально плохо вник в теорию и рассматривал IDF как атрибут документа, поэтому вычислял в цикле, хотя математически, действительно, параметр можно вычмслить сразу.
    double IDFCalc (const string& word) const { 
        const int num_found_docs = documents_.at(word).size(); // Количество документов, содержащтих слово word
        return log ( static_cast<double>(document_count_) / num_found_docs);
    }

    vector<Document> FindAllDocuments(const Query& query) const {
        vector<Document> matched_documents;
        map<int, double> docs_relevance; // <id документа, релевантность документа (TF*IDF)>
        for (const string& word : query.plus_words) {
            if (documents_.count(word) > 0) {
                double word_IDF = IDFCalc (word);
//ЗАМЕЧАНИЕ: стоит декомпозировать переменную через [переменная1, переменная2]. работать с именованными переменными лучше, чем с first / second
// Переделал в: const auto& [doc_id, word_TF]. Согласен с замечанием, но сделал изначально const pair<int, double>& doc, потому что так понятна структура данных (мне это больше нравиться при небольшом объёме кода с данной переменной). На "идеальный" вариант const pair<int, double>& [doc_id, word_TF] компилятор выдаёт ошибку.
                for (const auto& [doc_id, word_TF] : documents_.at(word)) 
                    docs_relevance[doc_id] += word_TF * word_IDF;
                }       
            }
        for (const string& word : query.minus_words) {
            if (documents_.count(word)>0)
                for (const auto& [doc_id, word_TF] : documents_.at(word))
                    if (docs_relevance.count(doc_id))
                        docs_relevance.erase(doc_id);
            }
        for (const pair<int, double>& doc : docs_relevance)
            matched_documents.push_back({doc.first, doc.second});
        return matched_documents;
    }   
};
// Спасибо за конструктивные замечания, текст программы получается более лаконичным и понятным.
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