#pragma once
#include <iostream>
#include <vector>
#include <map>
#include <set>

        // Перегрузка операторов << для вывода сообщений об ошибках тестирования
        // Вывод значения пары
        template <typename KeyElement, typename SubjElement>
    std::ostream& operator<<(std::ostream& os, const std::pair<KeyElement, SubjElement>& map_element)
    {
        using namespace std;
        os << map_element.first << ": "s << map_element.second;
        return os;
    }

    // Служебная функция для вывода значений контейнеров
    template <typename Obj>
    std::ostream& Print(std::ostream& os, const Obj& object)
    {
        using namespace std;
        bool is_first = true;
        for (const auto& part : object)
        {
            if (!is_first)
                cout << ", "s;
            cout << part;
            is_first = false;
        }
        return os;
    }

    // Вывод значения вектора
    template <typename Element>
    std::ostream& operator<<(std::ostream& os, const  std::vector<Element>& vect)
    {
        using namespace std;
        os << "["s;
        Print(os, vect) << "]"s;
        return os;

    }

    // Вывод значения набора
    template <typename Element>
    std::ostream& operator<<(std::ostream& os, std::set<Element> setobj)
    {
        using namespace std;
        os << "{"s;
        Print(os, setobj);
        os << "}"s;
        return os;
    }

    // Вывод значения словаря
    template <typename KeyElement, typename SubjElement>
    std::ostream& operator<<(std::ostream& os, std::map<KeyElement, SubjElement> dict)
    {
        using namespace std;
        os << "{"s;
        Print(os, dict);
        os << "}"s;
        return os;
    }
    //Функции и макросы для юнит-тестов
    template <typename T, typename U>
    void AssertEqualImpl(const T& t, const U& u, const  std::string& t_str, const  std::string& u_str, const  std::string& file,
        const  std::string& func, unsigned line, const  std::string& hint) {
        using namespace std;
        if (t != u) {
            cerr << boolalpha;
            cerr << file << "("s << line << "): "s << func << ": "s;
            cerr << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
            cerr << t << " != "s << u << "."s;
            if (!hint.empty()) {
                cerr << " Hint: "s << hint;
            }
            cerr << endl;
            abort();
        }
    }
void AssertImpl(bool value, const  std::string& expr_str, const  std::string& file, const  std::string& func, unsigned line,
        const  std::string& hint);

#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

#define ASSERT(expr) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_HINT(expr, hint) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

