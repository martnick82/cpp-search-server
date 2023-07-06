#include "remove_duplicates.h"

void RemoveDuplicates(SearchServer& search_server)
{
	using namespace std;
	set <set<string>> words_to_documents;
	vector<int> removing_ids;

	// ���������: for (auto it = search_server.begin(); it != search_server.end(); ++it)
	//
	//	� ��� � ������� �������.��� � ������ ���������� ���������� ����������, 
	// ����� ����� ������������ ���������� ������ �����, �� ���� ����� ������������� ����������, � �� ��������
	//

	//���������: ��. :-[
	for (int id : search_server)
	{
// ���������: set<string> words = search_server.GetWords(*it);
//
//		GetWords - �� ����� ��������� ������ �������, 
// � ��� � ������� ���� ����� GetWordFrequencies, ��� ���������� ��� ����������
// 
// ���������: ��. :-[

		map<string, double> doc_words = search_server.GetWordFrequencies(id);
		set<string> set_words = MapKeysToSet(doc_words); // ��������� �� ������� ��������� ����������� ������ �� ������ ����, �� ����������� �������������
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

// ���������: for (auto it1 = it + 1; it1 != search_server.end(); ++it1)
//
// ������� ������ �� ���� ������� ���� ������������ ���������, 
//	��� �� ������������� �������.�������, ��� �������� - ��� �������� 
// � ���������� ������� ���������� ����.����� ���� ����� ���� ������ � ����������,
// ���� ������� ����� �����, �� �� ���� ������ �� ������� ������� ����� ������� ��� ���������

//���������: ��. :-[

// ���������: ������ � ��� ��� ����� ��������� � ������ ����� ? 
// �������� �������� ��� � ��������� ����� ���������.����� ����� �������� ��������������.
// ����� ���������� �����������.��������� ����� ��������� ����������

// ���������: ��� ����� ������ ��-�� ����� ���������. 
// �������� ����������, ��� �������� ����� ���������� � �������� �� ������� ����� ������������.
// ��� ��������� �� ��� �������, ��� � ����� ������ �� ���������� � GITHUB�� �� ��� ���, 
// ������ � �� ������ �������� ��������� � ������� � ���� �� ���������� ��������� ��� - �� ���� ������� ��������� � ����� main.
// ���� ��������� ����������� � ����, �� �� ������� �������. �� ���������, ��� ����� �������� ������� ����-�� �� ������.
// ��� ��� ��������!
//