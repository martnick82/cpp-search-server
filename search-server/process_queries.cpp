
#include "process_queries.h"

std::vector<std::vector<Document>> ProcessQueries(
    const SearchServer& search_server,
    const std::vector<std::string>& queries)
{
    std::vector<std::vector<Document>> result(queries.size());
    transform(std::execution::par,
    queries.begin(), queries.end(),
        result.begin(),
        [&search_server](const std::string& str) { return search_server.FindTopDocuments(str); });
    return result;
}

std::vector<std::vector<Document>> ProcessQueriesSeq(
    const SearchServer& search_server,
    const std::vector<std::string>& queries)
{
    std::vector<std::vector<Document>> result;
    for (const std::string& query : queries) 
    {
        result.push_back(search_server.FindTopDocuments(query));
    }
    return result;
}

std::list<Document> ProcessQueriesJoined(
    const SearchServer& search_server,
    const std::vector<std::string>& queries)
{
    std::list<Document> result;
    for (const std::vector<Document>& res : ProcessQueries(search_server, queries))
    {
        result.insert(result.end(), res.begin(), res.end());
    }
    return result;
}