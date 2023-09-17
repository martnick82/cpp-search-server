#pragma once
#include <iostream>
#include "test_framework.h"
#include "search_server.h"
#include "string_processing.h"
#include "log_duration.h"

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
//Тест функции возврата ID документа по его порядковому номеру. В 6 спринте удалён
//void TestGetDocumentId();
//Тест работы исключений
void TestException();
//Тест работы методов begin(), end()
void TestIterators();
//Тест получения GetWordFrequencies по ID документа
void TestGetWordFrequencies();
//Тест удаления документа
void TestRemoveDocument();
// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer();

