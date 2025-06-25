c

using namespace std;

enum TokenType {
    IDENT, NUM, SIGN, QUIT, OPERATOR, UNRECOGNIZED
};

struct Token {
    TokenType type;
    string value;
};

vector<Token> tokenize(const string &input);
bool isDigit(char ch);
bool isLetter(char ch);
bool isOperator(char ch);
bool parseExpression(const vector<Token> &tokens, size_t &index, double &result);
bool parseTerm(const vector<Token> &tokens, size_t &index, double &result);
bool parseFactor(const vector<Token> &tokens, size_t &index, double &result);
void skipWhitespace(const string &input, size_t &index);
bool handleStatement(const string &input);
void error(const string &message);

// 全局變量表
unordered_map<string, double> variables;

int main() {
    cout << "Program starts..." << endl;
    string input;
    while (true) {
        cout << "> ";
        getline(cin, input);
        if (!handleStatement(input)) {
            break;
        }
    }
    cout << "Program exits..." << endl;
    return 0;
}

vector<Token> tokenize(const string &input) {
    vector<Token> tokens;
    size_t index = 0;
    while (index < input.length()) {
        skipWhitespace(input, index);
        if (index >= input.length()) break;
        if (isLetter(input[index])) {
            string ident;
            while (index < input.length() && (isLetter(input[index]) || isDigit(input[index]) || input[index] == '_')) {
                ident += input[index++];
            }
            if (ident == "quit") {
                tokens.push_back({QUIT, ident});
            } else {
                tokens.push_back({IDENT, ident});
            }
        } else if (isDigit(input[index]) || input[index] == '.') {
            string num;
            while (index < input.length() && (isDigit(input[index]) || input[index] == '.')) {
                num += input[index++];
            }
            tokens.push_back({NUM, num});
        } else if (isOperator(input[index])) {
            tokens.push_back({OPERATOR, string(1, input[index++])});
        } else if (input[index] == '+' || input[index] == '-') {
            tokens.push_back({SIGN, string(1, input[index++])});
        } else if (input[index] == '/') {
            if (index + 1 < input.length() && input[index + 1] == '/') {
                break; // 跳過行註解
            } else {
                tokens.push_back({OPERATOR, string(1, input[index++])});
            }
        } else {
            tokens.push_back({UNRECOGNIZED, string(1, input[index++])});
        }
    }
    return tokens;
}

bool isDigit(char ch) {
    return ch >= '0' && ch <= '9';
}

bool isLetter(char ch) {
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

bool isOperator(char ch) {
    return ch == '=' || ch == '<' || ch == '>' || ch == '*' || ch == '/' || ch == ';' || ch == '(' || ch == ')';
}

bool parseExpression(const vector<Token> &tokens, size_t &index, double &result) {
    if (!parseTerm(tokens, index, result)) return false;
    while (index < tokens.size() && (tokens[index].value == "+" || tokens[index].value == "-")) {
        string op = tokens[index++].value;
        double term;
        if (!parseTerm(tokens, index, term)) return false;
        if (op == "+") result += term;
        else if (op == "-") result -= term;
    }
    return true;
}

bool parseTerm(const vector<Token> &tokens, size_t &index, double &result) {
    if (!parseFactor(tokens, index, result)) return false;
    while (index < tokens.size() && (tokens[index].value == "*" || tokens[index].value == "/")) {
        string op = tokens[index++].value;
        double factor;
        if (!parseFactor(tokens, index, factor)) return false;
        if (op == "*") result *= factor;
        else if (op == "/") result /= factor;
    }
    return true;
}

bool parseFactor(const vector<Token> &tokens, size_t &index, double &result) {
    if (index >= tokens.size()) return false;
    if (tokens[index].type == NUM) {
        result = stod(tokens[index++].value);
        return true;
    } else if (tokens[index].type == IDENT) {
        string ident = tokens[index++].value;
        if (variables.find(ident) == variables.end()) {
            error("Undefined identifier: " + ident);
            return false;
        }
        result = variables[ident];
        return true;
    } else if (tokens[index].value == "(") {
        index++;
        if (!parseExpression(tokens, index, result)) return false;
        if (index >= tokens.size() || tokens[index].value != ")") {
            error("Expected ')'");
            return false;
        }
        index++;
        return true;
    } else if (tokens[index].value == "+" || tokens[index].value == "-") {
        string sign = tokens[index++].value;
        if (!parseFactor(tokens, index, result)) return false;
        if (sign == "-") result = -result;
        return true;
    }
    error("Unexpected token: " + tokens[index].value);
    return false;
}

void skipWhitespace(const string &input, size_t &index) {
    while (index < input.length() && isspace(input[index])) {
        index++;
    }
}

bool handleStatement(const string &input) {
    vector<Token> tokens = tokenize(input);
    if (tokens.empty()) return true;

    size_t index = 0;
    if (tokens[index].type == QUIT) {
        return false;
    }

    double result = 0;
    if (tokens[index].type == IDENT && tokens.size() > 2 && tokens[index + 1].value == ":=") {
        string ident = tokens[index++].value;
        index++; // Skip ':='
        if (!parseExpression(tokens, index, result)) return false;
        variables[ident] = result;
        cout << result << endl;
    } else if (parseExpression(tokens, index, result)) {
        cout << result << endl;
    } else {
        error("Unexpected token");
    }

    return true;
}

void error(const string &message) {
    cerr << message << endl;
}
