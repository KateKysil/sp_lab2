#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <iomanip>

using namespace std;
namespace Color {
    const string reset = "\033[0m";
    const string black = "\033[30m";
    const string red = "\033[31m";
    const string green = "\033[32m";
    const string yellow = "\033[33m";
    const string blue = "\033[34m";
    const string magenta = "\033[35m";
    const string cyan = "\033[36m";
    const string white = "\033[37m";
    const string bold = "\033[1m";
}

enum class TokenType {
    Number,
    String,
    Char,
    Identifier,
    Keyword,
    Operator,
    Punctuator,
    Comment,
    Whitespace,
    Error,
    EndOfFile
};

struct Token {
    TokenType type;
    string lexeme;
    string message; 
};

static const unordered_set<string> java_keywords = {
    "abstract","assert","boolean","break","byte","case","catch","char","class","const",
    "continue","default","do","double","else","enum","extends","final","finally","float",
    "for","goto","if","implements","import","instanceof","int","interface","long","native",
    "new","package","private","protected","public","return","short","static","strictfp",
    "super","switch","synchronized","this","throw","throws","transient","try","void",
    "volatile","while","yield","record","sealed","non-sealed","permits","var","module",
    "opens","requires","exports","provides","uses","with","to","transitive", "sealed" 
};

class Lexer {
public:
    Lexer(const string& src) : src(src), pos(0) {}

    vector<Token> tokenize() {
        vector<Token> tokens;
        while (true) {
            Token t = nextToken();
            tokens.push_back(t);
            if (t.type == TokenType::EndOfFile) break;
        }
        return tokens;
    }

private:
    string src;
    size_t pos;

    char peek(size_t k = 0) {
        if (pos + k >= src.size()) return '\0';
        return src[pos + k];
    }
    char get() {
        if (pos >= src.size()) return '\0';
        char c = src[pos++];
        return c;
    }
    bool startsWith(const string& s) {
        if (pos + s.size() > src.size()) return false;
        return src.compare(pos, s.size(), s) == 0;
    }

    Token makeToken(TokenType type, const string& lexeme, const string& msg = "") {
        return Token{ type, lexeme, msg};
    }

    Token nextToken() {
         char c = peek();
         if (c == '\0') {
            return makeToken(TokenType::EndOfFile, "");
         }
         if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
             string lex;
             while (true) {
                 char p = peek();
                 if (p == ' ' || p == '\t' || p == '\r' || p == '\n') lex.push_back(get());
                 else break;
             }
             return makeToken(TokenType::Whitespace, lex);
            }
            

        c = peek();
        // Comments
        if (startsWith("//")) {
            string lex;
            lex.push_back(get()); // '/'
            lex.push_back(get()); // '/'
            while (peek() != '\n' && peek() != '\0') lex.push_back(get());
            return makeToken(TokenType::Comment, lex);
        }
        if (startsWith("/*")) {
            string lex;
            lex.push_back(get());
            lex.push_back(get());
            bool closed = false;
            while (peek() != '\0') {
                if (startsWith("*/")) {
                    lex.push_back(get());
                    lex.push_back(get());
                    closed = true;
                    break;
                }
                else {
                    lex.push_back(get());
                }
            }
            if (!closed) {
                return makeToken(TokenType::Error, lex, "Unterminated block comment");
            }
            return makeToken(TokenType::Comment, lex );
        }
        // Strings
        if (c == '"') {
            string lex;
            lex.push_back(get());
            bool closed = false;
            while (peek() != '\0') {
                char p = get();
                lex.push_back(p);
                if (p == '\\') {
                    if (peek() != '\0') lex.push_back(get());
                }
                else if (p == '"') {
                    closed = true;
                    break;
                }
                else {
                }
            }
            if (!closed) return makeToken(TokenType::Error, lex, "Error");
            return makeToken(TokenType::String, lex);
        }

        // Char
        if (c == '\'') {
            string lex;
            lex.push_back(get());
            bool closed = false;
            while (peek() != '\0') {
                char p = get();
                lex.push_back(p);
                if (p == '\\') {
                    if (peek() != '\0') lex.push_back(get());
                }
                else if (p == '\'') {
                    closed = true;
                    break;
                }
            }
            if (!closed) return makeToken(TokenType::Error, lex, "Error");
            return makeToken(TokenType::Char, lex);
        }

        // Number
        if (isdigit(c) || (c == '.' && isdigit(peek(1)))) {
            string lex;
            bool isFloat = false;
            if (c == '0' && (peek(1) == 'x' || peek(1) == 'X')) {
                lex.push_back(get());
                lex.push_back(get());
                bool anyHex = false;
                while (isxdigit(peek())) { lex.push_back(get()); anyHex = true; }
                if (!anyHex) return makeToken(TokenType::Error, lex, "Invalid hex literal");
                return makeToken(TokenType::Number, lex);
            }
            while (isdigit(peek())) lex.push_back(get());
            if (peek() == '.') {
                if (peek(1) == '.') {
                }
                else {
                    isFloat = true;
                    lex.push_back(get());
                    while (isdigit(peek())) lex.push_back(get());
                }
            }
            // exponent
            if (peek() == 'e' || peek() == 'E' || peek() == 'p' || peek() == 'P') {
                char e = peek();
                if (!lex.empty()) {
                    lex.push_back(get());
                    if (peek() == '+' || peek() == '-') lex.push_back(get());
                    bool anyDig = false;
                    while (isdigit(peek())) { anyDig = true; lex.push_back(get()); }
                    if (!anyDig) return makeToken(TokenType::Error, lex, "Malformed exponent in number");
                    isFloat = true;
                }
            }
            if (peek() == 'f' || peek() == 'F' || peek() == 'd' || peek() == 'D' || peek() == 'l' || peek() == 'L') {
                lex.push_back(get());
            }
            return makeToken(TokenType::Number, lex);
        }

        // Identifier or keyword
        if (isIdentifierStart(c)) {
            string lex;
            lex.push_back(get());
            while (isIdentifierPart(peek())) lex.push_back(get());
            if (java_keywords.count(lex)) return makeToken(TokenType::Keyword, lex);
            else return makeToken(TokenType::Identifier, lex);
        }

        // Operators and punctuators: attempt longest-match
        static const vector<string> multiOps = {
            ">>>=", ">>=", "<<=", ">>>", ">>", "<<",
            "==", "!=", "<=", ">=", "&&", "||", "++", "--",
            "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "->", "::", "..."
        };
        for (const auto& op : multiOps) {
            if (startsWith(op)) {
                string lex;
                for (size_t i = 0; i < op.size(); ++i) lex.push_back(get());
                return makeToken(TokenType::Operator, lex);
            }
        }

        // Single-char operators / punctuators
        char single = peek();
        string singleStr(1, single);
        const string operators_chars = "+-*/%&|^~!=<>?:";
        const string punctuators_chars = "(){},;[].@";
        if (operators_chars.find(single) != string::npos) {
            string lex;
            lex.push_back(get());
            return makeToken(TokenType::Operator, lex);
        }
        if (punctuators_chars.find(single) != string::npos) {
            string lex;
            lex.push_back(get());
            return makeToken(TokenType::Punctuator, lex);
        }
        {
            string lex;
            lex.push_back(get());
            return makeToken(TokenType::Error, lex, "There is no such character");
        }
    }

    static bool isIdentifierStart(char c) {
        return (c == '_' || c == '$' || isalpha((unsigned char)c));
    }
    static bool isIdentifierPart(char c) {
        return (c == '_' || c == '$' || isalnum((unsigned char)c));
    }
};
string colorFor(TokenType t) {
    switch (t) {
    case TokenType::Keyword: return Color::blue + Color::bold;
    case TokenType::Identifier: return Color::white;
    case TokenType::Number: return Color::magenta;
    case TokenType::String: return Color::green;
    case TokenType::Char: return Color::green;
    case TokenType::Comment: return Color::cyan;
    case TokenType::Operator: return Color::yellow;
    case TokenType::Punctuator: return Color::yellow;
    case TokenType::Error: return Color::red + Color::bold;
    case TokenType::Whitespace: return ""; 
    default: return "";
    }
}

string typeName(TokenType t) {
    switch (t) {
    case TokenType::Number: return "Number";
    case TokenType::String: return "String";
    case TokenType::Char: return "Char";
    case TokenType::Identifier: return "Identifier";
    case TokenType::Keyword: return "Keyword";
    case TokenType::Operator: return "Operator";
    case TokenType::Punctuator: return "Punctuator";
    case TokenType::Comment: return "Comment";
    case TokenType::Whitespace: return "Whitespace";
    case TokenType::Error: return "Error";
    case TokenType::EndOfFile: return "EOF";
    default: return "Other";
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    string input;
    string filepath;
    cout << "Enter path to Java file: ";
    getline(cin, filepath);
    ifstream ifs(filepath, ios::in | ios::binary);
    if (!ifs) {
        cerr << "Cannot open file: " << filepath << "\n";
        return 2;
    }
    ostringstream ss;
    ss << ifs.rdbuf();
    input = ss.str();
    Lexer lexer(input);
    vector<Token> tokens = lexer.tokenize();
    for (const Token& t : tokens) {
        if (t.type == TokenType::EndOfFile) break;
        string col = colorFor(t.type);
        if (!col.empty()) {
            cout << col << t.lexeme << Color::reset;
        }
        else {
            cout << t.lexeme;
        }
    }
    return 0;
}