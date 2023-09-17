#pragma once
#include <iostream>

struct Document
{
    Document() : id(0), relevance(0.0), rating(0) {}

    Document(int ID, double rel, int rate)
        : id(ID), relevance(rel), rating(rate) {}

    int id;
    double relevance;
    int rating;
};

enum class DocumentStatus
{
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

// Перегрузка оператора << для вывода значения DocumentStatus в юнит-тестах
std::ostream& operator<<(std::ostream& os, DocumentStatus status);

