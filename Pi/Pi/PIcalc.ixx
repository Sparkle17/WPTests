module;

#include <string>
#include <vector>

export module PIcalc;

using namespace std;

// אכדמנטעל Spigot https://en.wikipedia.org/wiki/Spigot_algorithm
export string calculate(int n)
{
    int M = (n * 10 + 2) / 3;
    vector<int> r(M, 2);
    string pi;
    pi.reserve(n + 1);
    for (int i = 0; i < n; ++i)
    {
        int carry = 0;
        int sum = 0;
        for (int j = M - 1; j >= 0; --j)
        {
            r[j] *= 10;
            sum = r[j] + carry;
            int q = sum / (2 * j + 1);
            r[j] = sum % (2 * j + 1);
            carry = q * j;
        }
        r[0] = sum % 10;
        int q = sum / 10;
        if (q >= 10)
        {
            q = q - 10;
            for (size_t j = pi.length() - 1;; --j)
            {
                if (pi[j] == '9')
                    pi[j] = '0';
                else
                {
                    ++pi[j];
                    break;
                }
            }
        }
        pi += ('0' + q);
        if (i == 0) pi += '.';
    }
    return pi;
}
