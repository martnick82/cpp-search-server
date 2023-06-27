#include "remove_duplicates.h"
//Сложность точно не логармфмическая, и нужно перелопачивать класс
void RemoveDuplicates(SearchServer& search_server)
{
	using namespace std;
	vector<int> duplicates;
	for (auto it = search_server.begin(); it != search_server.end(); ++it)
	{
		set<string> words = search_server.GetWords(*it);
		for (auto it1 = it + 1; it1 != search_server.end(); ++it1)
		{
			set<string> words1 = search_server.GetWords(*it1);
			if (words == words1)
			{
				duplicates.push_back(*it1);
			}
		}
	}
	for (int id : duplicates)
	{
		cout << "Found duplicate document id "s << id << endl;
		search_server.RemoveDocument(id);
	}
}