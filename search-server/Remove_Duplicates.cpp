#include "remove_duplicates.h"

void RemoveDuplicates(SearchServer& search_server)
{
	using namespace std;
	set <set<string_view>> words_to_documents;
	vector<int> removing_ids;
	for (int id : search_server)
	{
		map<string_view, double> doc_words = search_server.GetWordFrequencies(id);
		set<string_view> set_words = MapKeysToSet(doc_words); // ��������� �� ������� ��������� ����������� ������ �� ������ ����, �� ����������� �������������
		if (words_to_documents.count(set_words)) //Logarithmic in the size of the container plus linear in the number of the elements found
		{
			removing_ids.push_back(id);
		}
		else
		{
			words_to_documents.insert(set_words);
		}
	}
	for (int id : removing_ids)
	{
		cout << "Found duplicate document id " << id << endl;
		search_server.RemoveDocument(id);
	}
}// O(w*(N + LogW))