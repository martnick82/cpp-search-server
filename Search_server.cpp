#include "Search_Server.h"

//    -------------------- ���������� SearchServer --------------------

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

    const double inv_word_count = 1.0 / words.size();
    for (const std::string& word : words)
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
        (text[text.size() - 1] == '-'))
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

vector<int>::iterator SearchServer::begin()
{

    return document_id_chain_.begin();
}

vector<int>::iterator SearchServer::end()
{
    return document_id_chain_.end();
}
//������� ������ �������, ������ ��������� �� �� ���������, � ������� �� ������������
const std::map<std::string, double>& SearchServer::GetWordFrequencies(int document_id) const
{
    map<std::string, double>* words_frequencies_in_document = new map<std::string, double>();
    auto it = word_to_document_freqs_.begin();
    auto end = word_to_document_freqs_.end();
    for (const auto& [word, documents] : word_to_document_freqs_)
    {
        if (documents.count(document_id))
        {
            (*words_frequencies_in_document)[word] = ComputeWordInverseDocumentFreq(word) * documents.at(document_id);
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
    document_id_chain_.erase(
        find(document_id_chain_.begin(), document_id_chain_.end(), document_id));

    documents_.erase(
        find_if(documents_.begin(), documents_.end(),
            [document_id](pair<int, SearchServer::DocumentData> data)
            { return document_id == data.first; }));

    for (auto& [word, documents] : word_to_document_freqs_)
    {
        if (documents.count(document_id))
        {
            documents.erase(document_id);
        }
    }
    // �������� ����, ������� ������� ������ � ��������� ��������
    for (auto it = word_to_document_freqs_.begin(); it != word_to_document_freqs_.end(); ++it)
    {
        if (it->second.size() == 0) // ����� ��� �� � ����� ���������
        {
            it = word_to_document_freqs_.erase(it);
            if (it == word_to_document_freqs_.end())
            {
                break;
            }
        }
    }
}

std::set<std::string> SearchServer::GetWords(int document_id)
{
    std::set<std::string> result;
    if (!IsDocumentID(document_id))
    {
        return result;
    }
    for (const auto& [word, documents] : word_to_document_freqs_)
    {
        if (documents.count(document_id))
        {
            result.insert(word);
        }
    }
    return result;
}