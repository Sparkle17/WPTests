module;

#include <format>
#include <map>
#include <regex>
#include <string>
#include <vector>

export module Parser;

using namespace std;

enum class TokenType {Number, OpenBracket, CloseBracket, Plus, Minus, Multiple, Divide, FunctionLog, FunctionPow, Comma };

struct Token
{
    TokenType   type;
    double      value;
};

map<char, TokenType> tokenMap {{ '(', TokenType::OpenBracket }, { ')', TokenType::CloseBracket }, { '+', TokenType::Plus },
                               { '-', TokenType::Minus },       { '*', TokenType::Multiple },     { '/', TokenType::Divide }, 
                               { ',', TokenType::Comma }};
map<string_view, TokenType> functionMap {{ "LOG", TokenType::FunctionLog }, {"POW", TokenType::FunctionPow}};

string g_EvalError{};

double evaluate(vector<Token>::const_iterator start, vector<Token>::const_iterator end, int priority);

double evaluateFunction(TokenType function, vector<Token>::const_iterator start, vector<Token>::const_iterator end)
{
    int depth = 0;
    auto it = start;
    while (it != end)
    {
        if (it->type == TokenType::OpenBracket) depth++;
        else if (it->type == TokenType::CloseBracket) depth--;

        if (depth == 0)
        {
            if (it->type == TokenType::Comma)
            {
                double value1 = evaluate(start, it, 0);
                double value2 = evaluate(it + 1, end, 0);
                if (function == TokenType::FunctionLog)
                {
                    if (value1 <= 0.0 || value2 <= 0.0 || value1 == 1.0)
                    {
                        g_EvalError = "Invalid logarithm parameters";
                        return 0.0;
                    }
                    return log(value2) / log(value1);
                }
                else if (function == TokenType::FunctionPow)
                {
                    if (value1 <= 0.0 && value2 <= 0.0)
                    {
                        g_EvalError = "Invalid power parameters";
                        return 0.0;
                    }
                    return pow(value1, value2);
                }
                else
                {
                    g_EvalError = "Invalid expression";
                    return 0.0;
                }
            }
        }
        ++it;
    }

    g_EvalError = "Invalid function parameters";
    return 0.0;
}

// priority: 0={-+}; 1={*/}; 2={() of function}
double evaluate(vector<Token>::const_iterator start, vector<Token>::const_iterator end, int priority)
{
    if (end - start == 1 && start->type == TokenType::Number)
        return start->value;
    if (end - start < 2 || priority > 2)
    {
        g_EvalError = "Invalid expression";
        return 0.0;
    }

    if (priority == 2)
    {
        if (start->type == TokenType::FunctionLog || start->type == TokenType::FunctionPow)
        {
            if (end - start >= 3 && start->type != TokenType::OpenBracket || (end - 1)->type != TokenType::CloseBracket)
                return evaluateFunction(start->type, start + 2, end - 1);
            else
            {
                g_EvalError = "Invalid function call expression";
                return 0.0;
            }
        }
        if (start->type != TokenType::OpenBracket || (end - 1)->type != TokenType::CloseBracket)
        {
            g_EvalError = "Invalid brackets in expression";
            return 0.0;
        }
        return evaluate(start + 1, end - 1, 0);
    }

    int depth = 0;

    auto it = start;
    while (it != end)
    {
        if (it->type == TokenType::OpenBracket) depth++;
        else if (it->type == TokenType::CloseBracket) depth--;

        if (depth == 0)
        {
            if (priority == 0)
            {
                if (it->type == TokenType::Minus || it->type == TokenType::Plus)
                {
                    double value1 = evaluate(start, it, 1);
                    double value2 = evaluate(it + 1, end, 0);
                    if (it->type == TokenType::Minus)
                        return value1 - value2;
                    else
                        return value1 + value2;
                }
            }
            else
            {
                if (it->type == TokenType::Multiple || it->type == TokenType::Divide)
                {
                    double value1 = evaluate(start, it, 2);
                    double value2 = evaluate(it + 1, end, 1);
                    if (it->type == TokenType::Divide)
                    {
                        if (value2 == 0.0)
                        {
                            g_EvalError = "Division by zero";
                            return 0.0;
                        }
                        return value1 / value2;
                    }
                    else
                        return value1 * value2;
                }
            }
        }
        ++it;
    }
    return evaluate(start, end, priority + 1);
}

// expr must be in upper case!
export string calculate(string_view expr)
{
    const regex rx_Number(R"([-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?)");
    vector<Token> parsed;

    // parsing expr to tokens
    size_t i = 0;
    while (i < expr.size())
    {
        auto it = tokenMap.find(expr[i]);
        if (it != tokenMap.end())
        {
            parsed.emplace_back(it->second, 0.0);
            ++i;
        }
        else if ((expr[i] >= '0' && expr[i] <= '9') || expr[i] == '.')
        {
            match_results<string_view::const_iterator> sm;
            if (regex_search(expr.cbegin() + i, expr.cend(), sm, rx_Number))
            {
                double value;
                size_t size = sm[0].second - sm[0].first;
                from_chars(expr.data() + i, expr.data() + i + size, value);
                parsed.emplace_back(TokenType::Number, value);
                i += size;
            }
            else
                return format("Invalid number expression at position {}", i);
        }
        else if (expr[i] >= 'A' && expr[i] <= 'Z')
        {
            size_t j = i;
            while (j < expr.size() && expr[j] >= 'A' && expr[j] <= 'Z')
                ++j;
            auto it = functionMap.find(expr.substr(i, j - i));
            if (it != functionMap.end())
            {
                parsed.emplace_back(it->second, 0.0);
                i = j;
            }
            else
                return format("Invalid function at position {}", i);
        }
        else if (expr[i] != ' ')
            return format("Invalid expression at position {}", i);
        else
            ++i;
    }

    // allow unary minus in brackets
    auto it = parsed.begin();
    while (it != parsed.end())
    {
        if (it->type == TokenType::OpenBracket)
        {
            auto next = it + 1;
            if (next != parsed.end() && next->type == TokenType::Minus)
            {
                it = parsed.insert(next, { TokenType::Number, 0.0 });
            }
            else
                ++it;
        }
        else
            ++it;
    }

    // evaluate
    double result = evaluate(parsed.cbegin(), parsed.cend(), 0);
    if (g_EvalError.size() != 0)
        return g_EvalError;
    return to_string(result);
}
