#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <execution>

/*
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
}*/

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

template <typename Iterator>
size_t Delete_Duplicates(Iterator first, Iterator last, Iterator begin)
{
    if (last == first)
    {
        return 0;
    }
    size_t new_size = 0;
    Iterator prev = first;
    first = std::next(first);
    while (first != last)
    {
        if (*prev == *first)
        {
            prev = first;
            first = std::next(first);
            continue;
        }
        *begin = *prev;
        begin = std::next(begin);
        prev = first;
        first = std::next(first);
        ++new_size;
    }
    *begin = *prev;
    ++new_size;

    return new_size;
}

template <typename ExecutionPolicy, typename Iterator>
size_t Delete_Duplicates(ExecutionPolicy&& policy, Iterator first, Iterator last, Iterator begin)
{
    if (last == first)
    {
        return 0;
    }
    size_t new_size = 0;
    new_size = std::transform_reduce(policy, next(first), last, first, new_size, std::plus{},
        [&begin, &last](auto& next, auto& prev)
        {            
            if (next != prev)
            {
                *begin++ = prev;                
                return 1;
            }
            return 0;
        });
    begin = prev(last);
    return ++new_size;
}

template <typename Iterator1, typename Iterator2>
bool IsIntersection(Iterator1 first1, Iterator1 last1, Iterator2 first2, Iterator2 last2)
{
    bool result = false;
    if ((first1 == last1) || (first2 == last2))
    {
        return result;
    }
    size_t d1 = distance(first1, last1);
    size_t d2 = distance(first2, last2);
    bool first_is_less = d1 < d2;
    if (first_is_less)
    {
        while (!result && first1 != last1)
        {
            result = std::binary_search(first2, last2, *first1++);
        }
    }
    else
    {
        while (!result && first2 != last2)
        {
            result = std::binary_search(first1, last1, *first2++);
        }
    }
    return result;
}

template <typename ExecutionPolicy, typename Iterator1, typename Iterator2>
bool IsIntersection(ExecutionPolicy&& policy, Iterator1 first1, Iterator1 last1, Iterator2 first2, Iterator2 last2)
{
    bool result = false;
    if ((first1 == last1) || (first2 == last2))
    {
        return result;
    }
    bool first_is_less = distance(first1, last1) < distance(first2, last2);
    if (first_is_less)
    {
        result = std::any_of(policy, first1, last1,
            [&first2, &last2](const auto& val1)
            {
                return
                    std::any_of(first2, last2,
                        [&val1](const auto& val2)
                        {
                            return val1 == val2;
                        });
            });
    }
    else
    {
        result = std::any_of(policy, first2, last2,
            [&first1, &last1](const auto& val2)
            {
                return
                    std::any_of(first1, last1,
                        [&val2](const auto& val1)
                        {
                            return val1 == val2;
                        });
            });
    }
    return result;
}