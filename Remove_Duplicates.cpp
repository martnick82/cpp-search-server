#include "remove_duplicates.h"

void RemoveDuplicates(SearchServer& search_server)
{
	using namespace std;
	set <set<string>> words_to_documents;
	vector<int> removing_ids;

	// ЗАМЕЧАНИЕ: for (auto it = search_server.begin(); it != search_server.end(); ++it)
	//
	//	у вас в задании сказано.что в случае правильной реализации итераторов, 
	// можно будет использовать упрощенную версию цикла, то есть будет информативная переменная, а не итератор
	//

	//Пояснение: да. :-[
	for (int id : search_server)
	{
// ЗАМЕЧАНИЕ: set<string> words = search_server.GetWords(*it);
//
//		GetWords - не стоит создавать лишних методов, 
// у вас в задании есть метод GetWordFrequencies, его достаточно для разработки
// 
// Пояснение: да. :-[

		map<string, double> doc_words = search_server.GetWordFrequencies(id);
		set<string> set_words = MapKeysToSet(doc_words); // Поскольку мы считаем документы идентичными только по набору слов, то отбрасываем релевантность
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

// ЗАМЕЧАНИЕ: for (auto it1 = it + 1; it1 != search_server.end(); ++it1)
//
// двойной проход по всем записям дает квадратичную сложность, 
//	что не соответствует заданию.напомню, что дубликат - это документ 
// с одинаковым набором уникальных слов.набор слов может быть ключом в контейнере,
// если хранить такие ключи, то за один проход по записям сервера можно выявить все дубликаты

//Пояснение: да. :-[

// ЗАМЕЧАНИЕ: почему у вас все файлы именуются в разном стиле ? 
// обратите внимание как в тренажере файлы именуются.стоит этого варианта придерживаться.
// файлы реализации дублируются.непонятно какой проверять необходимо

// Пояснение: Мне очень стыдно из-за этого замечания. 
// Извините пожалуйста, что доставил такие неудобства в проверке по причине своей криворукости.
// Это произошло по той причине, что я очень сильно не подружился с GITHUBом до сих пор, 
// притом я на ранних спринтах накосячил с ветками и пока не получилось исправить это - не могу удалённо вернуться в ветку main.
// Буду пробовать разбираться с этим, но не хватает времени. Не исключено, что через куратора призову кого-то на помощь.
// Ещё раз извините!
//