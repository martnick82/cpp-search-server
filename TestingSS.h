#pragma once
#include <iostream>
#include "TestFramework.h"
#include "SearchServer.h"
#include "String_processing.h"

void TestExcludeStopWordsFromAddedDocumentContent();
//���������� ����������. ����������� �������� ������ ���������� �� ���������� �������, ������� �������� ����� �� ���������.
void TestDocumentSearching();
//��������� �����-����. ���������, ���������� �����-����� ���������� �������, �� ������ ���������� � ���������� ������.
void TestMinusWords();
//������� ����������. ��� �������� ��������� �� ���������� ������� ������ ���� ���������� ��� ����� �� ���������� �������, �������������� � ���������. ���� ���� ������������ ���� �� �� ������ �����-�����, ������ ������������ ������ ������ ����.
void TestDocumentMatching();
//���������� ��������� ���������� �� �������������. ������������ ��� ������ ���������� ���������� ������ ���� ������������� � ������� �������� �������������.
void TestSortByRelivance();
//���������� �������� ����������. ������� ������������ ��������� ����� �������� ��������������� ������ ���������.
void TestDocumentRatingCalculation();
//���������� ����������� ������ � �������������� ���������, ����������� �������������.
void TestPredicateFunction();
//����� ����������, ������� �������� ������.
void TestSearchingDocumentByStatus();
//���������� ���������� ������������� ��������� ����������.
void TestRelevanceSearchihgDocuments();
//���� ������� �������� ID ��������� �� ��� ����������� ������
void TestGetDocumentId();
//���� ������ ����������
void TestException();

template <typename FunctionName>
void RunTestImpl(FunctionName function, const std::string& str_function)
{
    function();
    std::cerr << str_function << " OK" << std::endl;
}

#define RUN_TEST(func) RunTestImpl(func, #func) //������ ��� ������� ������� ������������

// ������� TestSearchServer �������� ������ ����� ��� ������� ������
void TestSearchServer();