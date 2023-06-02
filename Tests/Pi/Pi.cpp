#include <iostream>

import PIcalc;

using namespace std;

int precision = 6; // default precision
constexpr int maxPrecision = 100000000;
constexpr int minPrecision = 2;

int main(int argc, char** argv)
{
    cout << "PI calc" << endl;
    cout << "usage: PI.exe [precision]" << endl;
    if (argc > 1)
    {
        precision = atoi(argv[1]);
        precision = max(minPrecision, min(maxPrecision, precision));
    }

    cout << "calculating PI with precision: " << precision << " ..." << endl;
    cout << "result: " << calculate(precision) << endl;
    return 0;
}
