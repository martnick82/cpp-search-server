#pragma once
#include <iostream>
#include "test_framework.h"
#include "search_server.h"
#include "string_processing.h"
#include "log_duration.h"

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
// ������� TestSearchServer �������� ������ ����� ��� ������� ������
void TestSearchServer();

