#include "document.h"

// Перегрузка оператора << для вывода значения DocumentStatus в юнит-тестах
std::ostream& operator<<(std::ostream& os, DocumentStatus status)
{
    switch (status)
    {
    case DocumentStatus::ACTUAL:
        os << "ACTUAL";
        return os;
    case DocumentStatus::BANNED:
        os << "BANNED";
        return os;
    case DocumentStatus::IRRELEVANT:
        os << "IRRELEVANT";
        return os;
    case DocumentStatus::REMOVED:
        os << "REMOVED";
        return os;
    }
    return os;
}