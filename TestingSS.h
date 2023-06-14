#pragma once
#include <iostream>
#include "TestFramework.h"
#include "SearchServer.h"
#include "String_processing.h"

void TestExcludeStopWordsFromAddedDocumentContent();
//Добавление документов. Добавленный документ должен находиться по поисковому запросу, который содержит слова из документа.
void TestDocumentSearching();
//Поддержка минус-слов. Документы, содержащие минус-слова поискового запроса, не должны включаться в результаты поиска.
void TestMinusWords();
//Матчинг документов. При матчинге документа по поисковому запросу должны быть возвращены все слова из поискового запроса, присутствующие в документе. Если есть соответствие хотя бы по одному минус-слову, должен возвращаться пустой список слов.
void TestDocumentMatching();
//Сортировка найденных документов по релевантности. Возвращаемые при поиске документов результаты должны быть отсортированы в порядке убывания релевантности.
void TestSortByRelivance();
//Вычисление рейтинга документов. Рейтинг добавленного документа равен среднему арифметическому оценок документа.
void TestDocumentRatingCalculation();
//Фильтрация результатов поиска с использованием предиката, задаваемого пользователем.
void TestPredicateFunction();
//Поиск документов, имеющих заданный статус.
void TestSearchingDocumentByStatus();
//Корректное вычисление релевантности найденных документов.
void TestRelevanceSearchihgDocuments();
//Тест функции возврата ID документа по его порядковому номеру
void TestGetDocumentId();
//Тест работы исключений
void TestException();

template <typename FunctionName>
void RunTestImpl(FunctionName function, const std::string& str_function)
{
    function();
    std::cerr << str_function << " OK" << std::endl;
}

#define RUN_TEST(func) RunTestImpl(func, #func) //макрос для запуска функций тестирования

// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer();