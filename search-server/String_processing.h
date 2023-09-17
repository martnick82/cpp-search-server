#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <execution>

static std::vector<std::string> SplitIntoWords(const std::string& text)
{
    std::vector<std::string> words;
    std::string word;
    for (const char c : text)
    {
        if (c == ' ')
        {
            if (!word.empty())
            {
                words.push_back(word);
                word.clear();
            }
        }
        else
        {
            word += c;
        }
    }
    if (!word.empty())
    {
        words.push_back(word);
    }
    return words;
}

static std::vector<std::string_view> SplitIntoWords(std::string_view text)
{
    std::vector<std::string_view> words;
    const size_t pos_end = text.npos;
    while (!text.empty())
    {
        size_t not_space = text.find_first_not_of(" ");
        text.remove_prefix(not_space != pos_end ? not_space : text.size());
        size_t space = text.find(' ');
        if (text.empty())
        {
            break;
        }
        words.push_back(space != pos_end ? text.substr(0, space) : text.substr(0, pos_end));
        text.remove_prefix(space != pos_end ? space : text.size());
    }
    return words;
}
//ЗАМЕЧАНИЕ: 1. стиль именования функций(не используйте сразу оба разделителя, здесь достаточно, 
// что каждое слово с заглавной буквы). 2. удаление дубликатов - это не относится к обработке строк
// Пояснение: 1. исправил, как такое вышло не помню уже.
// 2. Убрал из проекта.
// В самом начале спринта, решая задачи по распараллеливанию методов, сделал несколько функций, 
// которые в дальнейшем заменил на функции из стандартной библиотеки, жалко было их выкидывать, поэтому пока сюда положил.
// нужно будет их потестить на досуге, вроде бы они не проигрывали в скорости решениям на стандартных функциях.
//