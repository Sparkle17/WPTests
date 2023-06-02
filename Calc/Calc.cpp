#include <algorithm>
#include <iostream>
#include <string>

import Parser;

using namespace std;

int main(int argc, char** argv)
{
    cout << "Calc: Calculate simple math expressions" << endl;
    if (argc < 2)
    {
        cout << "usage: Calc.exe \"expression\"" << endl;
        return 0;
    }

    string expr = argv[1];
    transform(expr.begin(), expr.end(), expr.begin(), ::toupper);
    cout << expr << " = " << calculate(expr) << endl;
    return 0;
}
