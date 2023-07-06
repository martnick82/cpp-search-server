#include "search_server.h"

//    -------------------- Реализация SearchServer --------------------

using namespace std;

void SearchServer::AddDocument(int document_id, const std::string& document, DocumentStatus status,
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
    const vector<std::string> words = SplitIntoWordsNoStop(document);
    set<string> key_words;

    const double inv_word_count = 1.0 / words.size();
    for (const std::string& word : words)
    {
        if (!IsValidWord(word))
        {
            throw invalid_argument("Invalid document word"s);
        }
        word_to_document_freqs_[word][document_id] += inv_word_count;
        key_words.insert(word);
    }
    word_sets_to_documents_[key_words].insert(document_id);
 //   ids_to_sets_words_[document_id] = key_words;
    documents_.emplace(document_id, DocumentData(ComputeAverageRating(ratings), status));
    document_ids_.insert(document_id);
}

bool SearchServer::IsValidWord(const std::string& text)
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
bool SearchServer::IsValidChar(const char& c)
{
    return !(c >= '\0' && c < ' ');
}

vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query) const
{
    return  FindTopDocuments(raw_query, []([[maybe_unused]] int document_id,
        DocumentStatus status, [[maybe_unused]] int rating)
        {
            return status == DocumentStatus::ACTUAL;
        });
}

vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query,
    DocumentStatus status) const
{
    return  FindTopDocuments(raw_query, [status]([[maybe_unused]] int document_id,
        DocumentStatus doc_status, [[maybe_unused]] int rating)
        {
            return status == doc_status;
        });
}

int SearchServer::GetDocumentCount() const
{
    return static_cast<int> (documents_.size());
}

tuple<vector<string>, DocumentStatus> SearchServer::MatchDocument(const std::string& raw_query,
    int document_id) const
{
    const Query query = ParseQuery(raw_query);
    vector<std::string> matched_words;
    for (const std::string& word : query.plus_words)
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
    for (const std::string& word : query.minus_words)
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

bool SearchServer::IsStopWord(const std::string& word) const
{
    return stop_words_.count(word) > 0;
}

bool SearchServer::IsDocumentID(const int doc_id) const
{
    return documents_.count(doc_id) > 0;
}

vector<string> SearchServer::SplitIntoWordsNoStop(const std::string& text) const
{
    vector<std::string> words;
    try {
        for (const std::string& word : SplitIntoWords(text))
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

int SearchServer::ComputeAverageRating(const vector<int>& ratings)
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

SearchServer::QueryWord SearchServer::ParseQueryWord(std::string text) const
{
    bool is_minus = false;
    if ((text.empty() ||
        (text[0] == '-' && text.size() == 1)) ||
        (text[0] == '-' && text[1] == '-') ||
        (text.back() == '-'))

//  УТОЧНЕНИЕ:       (text[text.size() - 1] == '-'))
//
//      text[text.size() - 1] аналогично text.back()
// 
// Пояснение: Спасибо, нужно будет запомнить.
// 
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

SearchServer::Query SearchServer::ParseQuery(const std::string& text) const {

    Query query;
    for (const std::string& word : SplitIntoWords(text)) {
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

double SearchServer::ComputeWordInverseDocumentFreq(const std::string& word) const
{
    return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}

set<int>::iterator SearchServer::begin()
{

    return document_ids_.begin();
}

set<int>::iterator SearchServer::end()
{
    return document_ids_.end();
}
//метод получения частот слов по id документа
const std::map<std::string, double>& SearchServer::GetWordFrequencies(int document_id) const
{
    std::map<std::string, double>* words_frequencies_in_document = new map<std::string, double>();
    if (!IsDocumentID(document_id))
    {
        throw invalid_argument("Invalid document ID"s);
    }
        std::pair <set<string>, set<int>> words_by_id = *find_if(word_sets_to_documents_.begin(), word_sets_to_documents_.end(),
            [document_id](pair<std::set<string>, set<int>> sw_to_id) 
            { bool res =  sw_to_id.second.count(document_id);
                return res; });

    for (const auto& word : words_by_id.first) //w
    {
        (*words_frequencies_in_document)[word] =
            ComputeWordInverseDocumentFreq(word) * word_to_document_freqs_.at(word).at(document_id); // LogW * LogN
    }    
    return *words_frequencies_in_document;
}

void SearchServer::RemoveDocument(int document_id)
{
    try
    {
        // ЗАМЕЧАНИЕ:   documents_.erase(
         //     find_if(documents_.begin(), documents_.end(),
         //         [document_id](pair<int, SearchServer::DocumentData> data)
         //         { return document_id == data.first; }));
         //   из словаря удаление просто по ключу
         // 
         // Пояснение: Тут даже не знаю, что и сказать. Это же нужно было такое изобразить! :-[
        //
         // ЗАМЕЧАНИЕ:  for (auto& [word, documents] : word_to_document_freqs_)
        //
        // здесь не слишком эффективно - проход по всем словам, что есть в документе, 
        // достаточно пройтись по словам удаляемого документа(новый метод вам пригодится для этого)
        // 
        // Пояснение: :-[
        // 
        set<string> key_words;
        {
            const map<string, double> words_relevance = GetWordFrequencies(document_id); //Logarithmic in the size of the container. LogN

            for (const auto& [word, rel] : words_relevance) //w
            {
                word_to_document_freqs_.at(word).erase(document_id); //log(a.size()) //Logarithmic in the size of the container.
                key_words.insert(word);
            }//LogW*w
        }
        for (const string& word : key_words) //w
        {
            if (word_to_document_freqs_.at(word).empty()) //Logarithmic in the size of the container   Const   LogW
            {
                word_to_document_freqs_.erase(word); //log(a.size()) LogW
            }
        }//LogW*w
        document_ids_.erase(document_id); // log(a.size()) logN
        documents_.erase(document_id); // log(a.size()) LogN
        word_sets_to_documents_.at(key_words).erase(document_id); //log(a.size()) + a.count(k)  LogW 
        if (word_sets_to_documents_.at(key_words).empty())
        {
            word_sets_to_documents_.erase(key_words);
        }
    } catch (...)
    {
        throw;
    }
}
//O(LogN + wLogW)
