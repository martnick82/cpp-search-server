#pragma once
#include <iostream>
#include <vector>

//  РЕКОМЕНДАЦИЯ: после подключенных библиотек стоит пустую строку добавить
//  Пояснение: Исправил во всех файлах
template <typename It>
class IteratorRange
{
public:
    IteratorRange(It begin, It end)
        : begin_(begin),
        end_(end),
        size_(distance(begin_, end_)) {}
    It begin()
    {
        return begin_;
    }

    It end()
    {
        return end_;
    }
    size_t size()
    {
        return size_;
    }
private:
    // РЕКОМЕНДАЦИЯ: каждую переменную стоит на отдельной строке объявлять
    // Пояснение: Да. Снова виновен. Помню было у меня это же замечание от Вас. Буду стараться так не делать.
    It begin_;
    It end_;
    size_t size_;
};

template <typename It>
std::ostream& operator<< (std::ostream& out, IteratorRange<It> ir)
{
    {
        for (It iterator = ir.begin(); iterator != ir.end(); ++iterator)
        {
            out << *iterator;
        }
        return out;
    }
}

template <typename Iterator>
class Paginator {
public:
    explicit Paginator(const Iterator beg_it, const Iterator end_it, const size_t page_size)
    {
   // ЗАМЕЧАНИЕ:   не слишком хорошо - неявное приведение типов(distance дает целочисленный тип, теоретически может быть отрицательным),
   //              а вы его записываете в переменную беззнакового типа, что может дать переполнение
   // Пояснение: тип size_t был установлен в задании, его менять не стал. Вставил static_cast для явного преобразования типа,
    //  но в общем случае мне всё равно неясно, как поведёт себя программа в случае отрицательного значения, полученного от distance.
    // в задаче данный класс применяется к вектору, с ним проблем быть не должно даже без преобразования. 
    // Перед применением с другими контейнерами нужно поэкспериментировать.
        const size_t num_pages = static_cast<size_t>(distance(beg_it, end_it) / page_size);
        Iterator begin_last_page = beg_it;
        advance(begin_last_page, page_size * num_pages);
        for (Iterator it = beg_it; it != begin_last_page; advance(it, page_size))
        {
            Iterator end_page = it;
            advance(end_page, page_size);
            pages_.push_back(IteratorRange(it, end_page));
        }
        if (begin_last_page != end_it)
        {
            pages_.push_back(IteratorRange(begin_last_page, end_it));
        }
    }

    auto end() const
    {
        return pages_.end();
    }

    auto begin() const
    {
        return pages_.begin();
    }

    size_t size() const
    {
        return pages_.size();
    }
private:
    std::vector<IteratorRange<Iterator>> pages_;
};
