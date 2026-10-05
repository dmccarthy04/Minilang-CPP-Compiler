#ifndef SYNTAXANALYZER_H
#define SYNTAXANALYZER_H
//Daniel McCarthy
//Jonathan Gomez
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;

class SyntaxAnalyzer {
private:
    vector<string> lexemes;
    vector<string> tokens;
    vector<string>::iterator lexitr;
    vector<string>::iterator tokitr;
    // map of variables and their datatype
    // i.e. sum t_integer
    map<string, string> symboltable;

    // other private methods
    bool vdec();
    int vars();
    bool stmtlist();
    int stmt();
    bool ifstmt();
    bool elsepart();
    bool whilestmt();
    bool assignstmt();
    bool inputstmt();
    bool outputstmt();
    bool logicop();
    bool type();
    bool numterm();
    bool logexpr();
    bool relexpr();
    bool arithexpr();
    bool strterm();
    bool relop();
    bool arithop();

public:
    SyntaxAnalyzer(istream& infile);
    // pre: 1st parameter consists of an open file containing a source code's
    // valid scanner/lexical analyzer output.  This data must be in the form: token : lexeme
    // post: the vectors have been populated
    bool parse();
    // pre: none
    // post: The lexemes/tokens have been parsed.
    // If an error occurs, a message prints indicating the token/lexeme pair
    // that caused the error.
    // If no error, vectors contain syntactically correct source code
};

#endif
