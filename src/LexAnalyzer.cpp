#include "LexAnalyzer.h"

// pre: parameter refers to open data file consisting of token and
// lexeme pairs i.e.  t_and and
// Each pair appears on its own input line.
// post: tokenmap has been populated - key: lexeme, value: token

LexAnalyzer::LexAnalyzer(std::istream &infile) {
    std::string token, lexeme;
    while (infile >> token >> lexeme) {
        tokenmap[lexeme] = token;
    }
}

void LexAnalyzer::addLexemeToken(const std::string& token, const std::string& lexeme) {
    tokens.emplace_back(token);
    lexemes.emplace_back(lexeme);
}

// pre: 1st parameter refers to an open text file that contains source
// code in the language, 2nd parameter refers to an open empty output
// file
// post: If no error, the token and lexeme pairs for the given input
// file have been written to the output file (token : lexeme).
// If there is an error, the incomplete token/lexeme pairs, as well as
// an error message have been written to the output file.
// A success or fail message has printed to the console.

void LexAnalyzer::scanFile(std::istream &infile, std::ostream &outfile) {
    std::string line;
    bool inString = false;
    int len = 0;
    std::string buffer;
    std::string thisStr;
    while (std::getline(infile, line)) {
        line.insert(line.length(), "  ");
        len = static_cast<int>(line.length());
        for (int i = 0; i < len - 2; i++) {
            std::string cur = line.substr(i, 1);
            if (!inString) {
                if (cur == "\"" && !isEscaped(line, i)) {
                    if (!buffer.empty()) {
                        evaluateWord(buffer);
                        addLexemeToken("ERROR", "Cannot begin string.");
                        outputPairs(outfile);
                        return;
                    }
                    inString = true;
                } else {
                    std::string curNext = line.substr(i, 2);
                    if (cur == " ") {
                        if (!buffer.empty()) {
                            if (!evaluateWord(buffer)) {
                                outputPairs(outfile);
                                return;
                            }
                            buffer.clear();
                        }
                    } else if (isDelimeter(curNext)) {
                        if (!buffer.empty()) {
                            if (!evaluateWord(buffer)) {
                                outputPairs(outfile);
                                return;
                            }
                            buffer.clear();
                        }
                        addLexemeToken(tokenmap[curNext], curNext);
                        i++;
                    } else if (isDelimeter(cur)) {
                        if (!buffer.empty()) {
                            if (!evaluateWord(buffer)) {
                                outputPairs(outfile);
                                return;
                            }
                            buffer.clear();
                        }
                        addLexemeToken(tokenmap[cur], cur);
                    } else {
                        buffer += cur;
                    }
                }
            } else {
                if (cur == "\"" && !isEscaped(line, i)) {
                    std::string next = line.substr(i + 1, 1);
                    std::string next2 = line.substr(i + 1, 2);
                    addLexemeToken("t_text", thisStr);
                    if (isDelimeter(next2)) {
                        addLexemeToken(tokenmap[next2], next2);
                        i += 2;
                    } else if (isDelimeter(next)) {
                        addLexemeToken(tokenmap[next], next);
                        i += 1;
                    } else if (next != " ") {
                        addLexemeToken("ERROR", "Cannot end string.");
                        outputPairs(outfile);
                        return;
                    }
                    inString = false;
                    thisStr = "";
                } else {
                    thisStr += cur;
                }
            }
        }
        if (!buffer.empty()) {
            if (!evaluateWord(buffer)) {
                outputPairs(outfile);
                return;
            }
            buffer.clear();
        }
    }
    if (inString) {
        addLexemeToken("ERROR", "Unclosed string.");
        outputPairs(outfile);
        return;
    }
    if (!buffer.empty()) {
        if (!evaluateWord(buffer)) {
            outputPairs(outfile);
            return;
        }
    }
    outputPairs(outfile);
}

void LexAnalyzer::outputPairs(std::ostream &outfile) {
    for (int i = 0; i < static_cast<int>(tokens.size()); i++) {
        outfile << tokens[i] << " " << lexemes[i] << std::endl;
    }
    if (tokens.empty()) {
        std::cout << "Error. No data received." << std::endl;
    } else if (tokens.back() != "ERROR") {
        std::cout << "Success! Good data received." << std::endl;
    } else {
        std::cout << "Read failed. Bad data." << std::endl;
    }
}

bool LexAnalyzer::isDelimeter(const std::string &source) {
    if (tokenmap.contains(source)) {
        if (tokenmap[source].substr(0, 1) == "s") {
            return true;
        }
    }
    return false;
}

bool LexAnalyzer::isEscaped(const std::string &source, int index) {
    int count = 0;
    index--;
    while (source[index] == '\\') {
        count++;
        index--;
    }
    return count % 2 == 1;
}

bool LexAnalyzer::evaluateWord(const std::string &word) {
    if (word.empty()) return true;
    if (tokenmap.contains(word)) {
        addLexemeToken(tokenmap[word], word);
    } else if (isdigit(word[0])) {
        bool isNum = true;
        for (int i = 1; i < (int) word.length(); i++) {
            if (!isdigit(word[i])) isNum = false;
        }
        if (isNum) {
            addLexemeToken("t_number", word);
        } else {
            addLexemeToken("ERROR", "Cannot parse number: " + word);
            return false;
        }
    } else if (isalpha(word[0])) {
        bool isId = true;
        for (int i = 1; i < (int) word.length(); i++) {
            if (!isalpha(word[i]) && !isdigit(word[i]) && word[i] != '_') isId = false;
        }
        if (isId) {
            addLexemeToken("t_id", word);
        } else {
            addLexemeToken("ERROR", "Cannot process ID: " + word);
            return false;
        }
    } else {
        addLexemeToken("ERROR", "Cannot process word: " + word);
        return false;
    }
    return true;
}
