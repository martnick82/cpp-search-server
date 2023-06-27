#include "Test_Framework.h"

void AssertImpl(bool value, const  std::string& expr_str, const  std::string& file, const  std::string& func, unsigned line,
    const  std::string& hint) 
{
    using namespace std;
    if (!value) 
    {
        cerr << file << "("s << line << "): "s << func << ": "s;
        cerr << "ASSERT("s << expr_str << ") failed."s;
        if (!hint.empty()) 
        {
            cerr << " Hint: "s << hint;
        }
        cerr << endl;
        abort();
    }
}
