#include "search_server.h"

//    -------------------- Реализация SearchServer --------------------

using namespace std;

void SearchServer::AddDocument(int document_id, const std::string_view document, DocumentStatus status,
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
    documents_.emplace(document_id, DocumentData(ComputeAverageRating(ratings), status, document.data()));
   // storage_.emplace_back(document.data());
    const vector<std::string_view> words = SplitIntoWordsNoStop(documents_.at(document_id).data);

    std::vector<string_view> key_words;
    const double inv_word_count = 1.0 / words.size();
    for (const std::string_view word : words)
    {
        if (!IsValidWord(word))
        {
            throw invalid_argument("Invalid document word"s);
        }
            word_to_document_freqs_[word][document_id] += inv_word_count;
            key_words.push_back(word);        
    }
    std::sort(key_words.begin(), key_words.end());
    key_words.erase(std::unique(key_words.begin(), key_words.end()), key_words.end());

    //word_sets_to_documents_[key_words].push_back(document_id);
    documents_to_word_sets_[document_id] = key_words;
    
    document_ids_.insert(document_id);
}

bool SearchServer::IsValidWord(const std::string_view text)
{    
    return std::all_of(text.begin(), text.end(),  IsValidChar);
}

bool SearchServer::IsValidChar(const char& c)
{
    return !(c >= '\0' && c < ' ');
}

vector<Document> SearchServer::FindTopDocuments(const std::string_view raw_query) const
{
    return  FindTopDocuments(raw_query, []([[maybe_unused]] int document_id,
        DocumentStatus status, [[maybe_unused]] int rating)
        {
            return status == DocumentStatus::ACTUAL;
        });
}

vector<Document> SearchServer::FindTopDocuments(const std::string_view raw_query,
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

tuple<vector<string_view>, DocumentStatus> SearchServer::MatchDocument(const std::string_view raw_query,
    int document_id) const
{
    const Query query = ParseQuery(raw_query);
    vector<std::string_view> matched_words;
    const std::vector<std::string_view> document_words = documents_to_word_sets_.at(document_id);
    bool is_minus_word = false;
    for (const std::string_view word : query.minus_words)
    {
        if (std::binary_search(document_words.begin(), document_words.end(), word))
        {
            is_minus_word = true;
        }
    }
    if (!is_minus_word)
    {
        for (const std::string_view word : query.plus_words)
        {
            if (std::binary_search(document_words.begin(), document_words.end(), word))
            {
                matched_words.push_back(word_to_document_freqs_.find(word)->first);
            }
        }
    }
    return tie(matched_words, documents_.at(document_id).status);
}

bool SearchServer::IsStopWord(const std::string_view word) const
{
    string str = word.data();
    str.resize(word.size());
    return stop_words_.count(str) > 0;
}

bool SearchServer::IsDocumentID(const int document_id) const
{
    return document_ids_.count(document_id) > 0;
}

int  SearchServer::GetWordsCount(int document_id) const
{
    return documents_to_word_sets_.at(document_id).size();
}

vector<string_view> SearchServer::SplitIntoWordsNoStop(const std::string_view text) const
{
    vector<std::string_view> words;
    try 
    {
        for (const std::string_view word : SplitIntoWords(text))
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

SearchServer::QueryWord SearchServer::ParseQueryWord(std::string_view text) const
{
    bool is_minus = false;
    if ((text.empty() ||
        (text[0] == '-' && text.size() == 1)) ||
        (text[0] == '-' && text[1] == '-') ||
        (text.back() == '-'))

    {
        throw invalid_argument("Invalid query word"s);
    }

    if (text[0] == '-')
    {
        is_minus = true;
        //text = text.substr(1);
    }
    return { is_minus, IsStopWord(text) };
}
/*
SearchServer::Query SearchServer::ParseQuery(const std::string_view text) const {

    Query query;
    for (const std::string_view word : SplitIntoWords(text)) 
    {
        if (!IsValidWord(word))
        {
            throw invalid_argument("Invalid query word"s);
        }
        const QueryWord query_word = ParseQueryWord(word);
        if (!query_word.is_stop)
        {
            if (query_word.is_minus) 
            {
                query.minus_words.push_back(word.substr(1));
            }
            else 
            {
                query.plus_words.push_back(word);
            }
        }       
    }
    sort(query.plus_words.begin(), query.plus_words.end());
    sort(query.minus_words.begin(), query.minus_words.end());
    query.plus_words.erase(std::unique(query.plus_words.begin(), query.plus_words.end()), query.plus_words.end());
    query.minus_words.erase(std::unique(query.minus_words.begin(), query.minus_words.end()), query.minus_words.end());
    return query;
}*/

SearchServer::Query SearchServer::ParseQuery(const std::string_view text, bool sequence) const
{
    using namespace std::string_literals;
    
    Query query;
    if (sequence)
    {
        for (const std::string_view word : SplitIntoWords(text))
        {
            if (!IsValidWord(word))
            {
                throw invalid_argument("Invalid query word"s);
            }
            const QueryWord query_word = ParseQueryWord(word);
            if (!query_word.is_stop)
            {
                if (query_word.is_minus)
                {
                    query.minus_words.push_back(word.substr(1));
                }
                else
                {
                    query.plus_words.push_back(word);
                }
            }
        }
        sort(query.plus_words.begin(), query.plus_words.end());
        sort(query.minus_words.begin(), query.minus_words.end());
        query.plus_words.erase(std::unique(query.plus_words.begin(), query.plus_words.end()), query.plus_words.end());
        query.minus_words.erase(std::unique(query.minus_words.begin(), query.minus_words.end()), query.minus_words.end());
    }
    else
    {
        execution::parallel_policy policy = execution::par;
        std::vector<std::string_view> query_vector = SplitIntoWords(text);
        sort(policy, query_vector.begin(), query_vector.end());
        query_vector.erase(
            unique(query_vector.begin(), query_vector.end()), query_vector.end());
        std::vector<std::string_view>::iterator it = std::lower_bound(query_vector.begin(), query_vector.end(), "-"s,
            [](const std::string_view word, const std::string_view val)
            { return word.at(0) == val.at(0); });
        if (it == query_vector.end())
        {
            std::swap(query.plus_words, query_vector);
        }
        else
        {
            query.plus_words = std::move(std::vector(it, query_vector.end()));
            query.minus_words = std::move(std::vector(query_vector.begin(), it));
        }
        std::for_each(policy,
            query.minus_words.begin(), query.minus_words.end(),
            [](std::string_view& word)
            {
                word = word.substr(1);
            });
    }
    return query;
}

double SearchServer::ComputeWordInverseDocumentFreq(const std::string_view word) const
{
    return log(static_cast<double>(GetDocumentCount() * 1.0) / word_to_document_freqs_.at(word).size());
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
const std::map<std::string_view, double>& SearchServer::GetWordFrequencies(int document_id) const
{
    static std::map<std::string_view, double>* words_frequencies_in_document;
    if (!words_frequencies_in_document)
    {
        words_frequencies_in_document = new map<std::string_view, double>();
    }
    else if (words_frequencies_in_document && !words_frequencies_in_document->empty())
    {
        words_frequencies_in_document->clear();
    }

    if (IsDocumentID(document_id))
    {
        vector<string_view> words_by_id = SplitIntoWordsNoStop(documents_.at(document_id).data);

        for (const auto& word : words_by_id) //w
        {
            (*words_frequencies_in_document)[word] =
                ComputeWordInverseDocumentFreq(word) * word_to_document_freqs_.at(word).at(document_id); // LogW * LogN
        }
    }
    return *words_frequencies_in_document;
}

void SearchServer::RemoveDocument(int document_id)
{
    if (!IsDocumentID(document_id))
    {
        return;
    }
    vector<string_view> key_words = documents_to_word_sets_.at(document_id);
    for (const string_view word : key_words)
    {
        word_to_document_freqs_.at(word).erase(document_id);
    }
    document_ids_.erase(document_id);
    documents_.erase(document_id);
    documents_to_word_sets_.erase(document_id);
}
