#ifndef LEXANALYZER_H
#define LEXANALYZER_H
// above can also be achieved with #pragma once

#include <iostream>
#include <map>
#include <vector>

class LexAnalyzer {
private:
    // map to hold language's valid lexeme/token pairs
    std::map<std::string, std::string> tokenmap;
    // parallel vectors to store valid lexemes and tokens from source code file
    std::vector<std::string> lexemes;
    std::vector<std::string> tokens;
    // other private methods

    static bool isEscaped(const std::string &source, int index); // completed
    bool isDelimeter(const std::string& source); // if the string is a delimeter e.g. ( { + - < ==, return true
    bool isKeyword(const std::string& source); // if the string is a keyword, return true
    bool evaluateWord(const std::string& source); // if this string is number, keyword, or id, add token/lexeme to vectors and return true
    void outputPairs(std::ostream& outfile); // outputs the entirety of the lexeme / token pairs to file and logs success or failure
    void addLexemeToken(const std::string& token, const std::string& lexeme); //adds lexemes / tokens to vectors

public:
    LexAnalyzer(std::istream& infile);
    // pre: parameter refers to open data file consisting of token and
    // lexeme pairs i.e.  t_and and
    // Each pair appears on its own input line.
    // post: tokenmap has been populated - key: lexeme, value: token

    void scanFile(std::istream& infile, std::ostream& outfile);
    // pre: 1st parameter refers to an open text file that contains source
    // code in the language, 2nd parameter refers to an open empty output
    // file
    // post: If no error, the token and lexeme pairs for the given input
    // file have been written to the output file (token : lexeme).
    // If there is an error, the incomplete token/lexeme pairs, as well as
    // an error message have been written to the output file.
    // A success or fail message has printed to the console.
};
#endif
