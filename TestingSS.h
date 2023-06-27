#pragma once
#include <iostream>
#include "Test_Framework.h"
#include "Search_Server.h"
#include "String_processing.h"
#include "Log_Duration.h"

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
//���� ������� �������� ID ��������� �� ��� ����������� ������. � 6 ������� �����
//void TestGetDocumentId();
//���� ������ ����������
void TestException();
//���� ������ ������� begin(), end()
void TestIterators();
//���� ��������� GetWordFrequencies �� ID ���������
void TestGetWordFrequencies();
//���� �������� ���������
void TestRemoveDocument();
//���� �������� ���� �� ID ���������
void TestGetWords();
// ������� TestSearchServer �������� ������ ����� ��� ������� ������
void TestSearchServer();

