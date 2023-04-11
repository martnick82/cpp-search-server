// Решите загадку: Сколько чисел от 1 до 1000 содержат как минимум одну цифру 3?
// Напишите ответ здесь:
#include <iostream>
#include <string>

using namespace std;

bool IsThree (int number) // Проверяет есть ли в числе цифра 3
{
    string numstring = to_string(number);
    for (char c : numstring)
        if (c == '3')
            return true;
    return false;
}

int main() {
    int count = 0; //счётчик чисел
    for (int i = 1; i<=1000; ++i)
        if (IsThree(i))
            ++count; // Считаем числа с цифрой 3
    cout << count << endl;
}
// Насчитал 271 число
    
// Закомитьте изменения и отправьте их в свой репозиторий.
