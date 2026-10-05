//Daniel McCarthy
//Jonathan Gomez
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;
#include "SyntaxAnalyzer.h"

SyntaxAnalyzer::SyntaxAnalyzer(istream& infile) {
    string tok, lex, line;
    while(getline(infile, line)) {
        if (line.empty()) continue;
        tok = "";
        lex = "";
        for(int i = 0; i < line.length(); i++) {
            if(isspace(line[i])) {
                tok = line.substr(0, i);
                lex = line.substr(i + 1);
                break;
            }
        }
        tokens.push_back(tok);
        lexemes.push_back(lex);
    }
}

bool SyntaxAnalyzer::parse() {
    lexitr = lexemes.begin();
    tokitr = tokens.begin();
    if (!vdec() && tokitr != tokens.end()) {
        cout << "Error at token/lexeme pair: " << *tokitr << " " << *lexitr << endl;
        return false;
    }
    if (tokitr != tokens.end() && *tokitr == "t_main") {
        tokitr++; lexitr++;
        if (!stmtlist() && tokitr != tokens.end()) {
            cout << "Error at token/lexeme pair: " << *tokitr << " " << *lexitr << endl;
            return false;
        }
        if (tokitr != tokens.end() && *tokitr == "t_end") {
            tokitr++; lexitr++;
            if (tokitr != tokens.end()) {
                cout << "Error, remaining tokens/lexemes found after end statement: " << *tokitr << " " << *lexitr << endl;
                return false;
            }
            cout << "Source code is correct" << endl;
            cout << "Symbol Table:" << endl;
            for (const auto& [variable, type] : symboltable)
                cout << variable << " : " << type << endl;
            return true;
        }
    }
    if (tokitr != tokens.end()) {
        cout << "Error at token/lexeme pair: " << *tokitr << " " << *lexitr << endl;
    }
    return false;
}

bool SyntaxAnalyzer::vdec() {
    if(tokitr != tokens.end() && *tokitr == "t_var") {
        tokitr++; lexitr++;
        int result = vars();
        if(result == 0)
            return false;
        if(result == -1)
            return false;
        while(tokitr != tokens.end() &&
             (*tokitr == "t_integer" || *tokitr == "t_string")) {
            result = vars();
            if(result == 0)  return false;
            if(result == -1) return false;
        }
        return true;
    }
    return true;
}

bool SyntaxAnalyzer::stmtlist() {
    while(tokitr != tokens.end() &&
         (*tokitr == "t_if"    ||
          *tokitr == "t_while" ||
          *tokitr == "t_id"    ||
          *tokitr == "t_input" ||
          *tokitr == "t_output")) {
        int result = stmt();
        if(result == 0)  return false;
        if(result == -1) return true;
    }
    return true;
}

bool SyntaxAnalyzer::ifstmt() {
    if(tokitr != tokens.end() && *tokitr == "t_if") {
        tokitr++; lexitr++;
        if(tokitr != tokens.end() && *tokitr == "s_lparen") {
            tokitr++; lexitr++;
            if(logexpr()) {
                if(tokitr != tokens.end() && *tokitr == "s_rparen") {
                    tokitr++; lexitr++;
                    if(tokitr != tokens.end() && *tokitr == "t_then") {
                        tokitr++; lexitr++;
                        if(stmtlist()) {
                            if(elsepart()) {
                                if(tokitr != tokens.end() && *tokitr == "t_end") {
                                    tokitr++; lexitr++;
                                    if(tokitr != tokens.end() && *tokitr == "t_if") {
                                        tokitr++; lexitr++;
                                        return true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}

bool SyntaxAnalyzer::elsepart() {
    if(tokitr != tokens.end() && *tokitr == "t_else") {
        tokitr++; lexitr++;
        if(stmtlist())
            return true;
        return false;
    }
    return true;
}

bool SyntaxAnalyzer::assignstmt() {
    if(tokitr != tokens.end() && *tokitr == "t_id") {
        string varType = *lexitr;
        tokitr++; lexitr++;
        if(tokitr != tokens.end() && *tokitr == "s_assign") {
            tokitr++; lexitr++;
            if(symboltable.contains(varType) &&
               symboltable[varType] == "t_integer") {
                if(arithexpr()) {
                    if(tokitr != tokens.end() && *tokitr == "s_semi") {
                        tokitr++; lexitr++;
                        return true;
                    }
                }
            }
            else if(symboltable.contains(varType) &&
                    symboltable[varType] == "t_string") {
                if(strterm()) {
                    if(tokitr != tokens.end() && *tokitr == "s_semi") {
                        tokitr++; lexitr++;
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool SyntaxAnalyzer::outputstmt() {
    if(tokitr != tokens.end() && *tokitr == "t_output") {
        tokitr++; lexitr++;
        if(tokitr != tokens.end() && *tokitr == "s_lparen") {
            tokitr++; lexitr++;
            if(numterm()) {
                if(tokitr != tokens.end() && *tokitr == "s_rparen") {
                    tokitr++; lexitr++;
                    return true;
                }
            }
            else if(strterm()) {
                if(tokitr != tokens.end() && *tokitr == "s_rparen") {
                    tokitr++; lexitr++;
                    return true;
                }
            }
        }
    }
    return false;
}

bool SyntaxAnalyzer::numterm() {
    if(tokitr != tokens.end() && *tokitr == "t_number") {
        tokitr++; lexitr++;
        return true;
    }
    if(tokitr != tokens.end() && *tokitr == "t_id") {
        if(symboltable.contains(*lexitr) &&
           symboltable[*lexitr] == "t_integer") {
            tokitr++; lexitr++;
            return true;
        }
    }
    return false;
}

bool SyntaxAnalyzer::logexpr() {
    if(relexpr()) {
        while(logicop()) {
            if(!relexpr())
                return false;
        }
        return true;
    }
    return false;
}

bool SyntaxAnalyzer::logicop() {
    if(tokitr != tokens.end() && *tokitr == "t_and") {
        tokitr++; lexitr++;
        return true;
    }
    else if(tokitr != tokens.end() && *tokitr == "t_or") {
        tokitr++; lexitr++;
        return true;
    }
    return false;
}

bool SyntaxAnalyzer::type() {
    if (tokitr != tokens.end() && (*tokitr == "t_integer" || *tokitr == "t_string")) {
        tokitr++; lexitr++;
        return true;
    }
    return false;
}

int SyntaxAnalyzer::vars() {
    string varType;
    if (!type()) {
        return -1;
    }
    if (tokitr != tokens.end()) {
        varType = *(tokitr - 1);
    }
    if (tokitr != tokens.end() && *tokitr == "t_id") {
        if (symboltable.contains(*lexitr)) {
            return 0;
        }
        symboltable[*lexitr] = varType;
        tokitr++; lexitr++;
        while (tokitr != tokens.end() && *tokitr == "s_comma") {
            tokitr++; lexitr++;
            if (tokitr != tokens.end() && *tokitr == "t_id") {
                if (symboltable.contains(*lexitr)) {
                    return 0;
                }
                symboltable[*lexitr] = varType;
                tokitr++; lexitr++;
            }
            else {
                return 0;
            }
        }
        if (tokitr != tokens.end() && *tokitr == "s_semi") {
            tokitr++; lexitr++;
            return 1;
        }
        return 0;
    }
    return 0;
}

int SyntaxAnalyzer::stmt() {
    if (tokitr != tokens.end() && *tokitr == "t_if") {
        return ifstmt();
    }
    if (tokitr != tokens.end() && *tokitr == "t_while") {
        return whilestmt();
    }
    if (tokitr != tokens.end() && *tokitr == "t_id") {
        return assignstmt();
    }
    if (tokitr != tokens.end() && *tokitr == "t_input") {
        return inputstmt();
    }
    if (tokitr != tokens.end() && *tokitr == "t_output") {
        return outputstmt();
    }
    return -1;
}

bool SyntaxAnalyzer::relexpr() {
    if (!arithexpr()) {
        return false;
    }
    if (!relop()) {
        return false;
    }
    if (!arithexpr()) {
        return false;
    }
    return true;
}

bool SyntaxAnalyzer::arithexpr() {
    if (!numterm()) {
        return false;
    }
    while (tokitr != tokens.end() &&
           (*tokitr == "s_plus"  ||
            *tokitr == "s_minus" ||
            *tokitr == "s_mult"  ||
            *tokitr == "s_div"   ||
            *tokitr == "s_mod")) {
        if (!arithop()) {
            return false;
        }
        if (!numterm()) {
            return false;
        }
    }
    return true;
}

bool SyntaxAnalyzer::strterm() {
    if (tokitr != tokens.end()) {
        if (*tokitr == "t_text") {
            tokitr++; lexitr++;
            return true;
        }
        if (*tokitr == "t_id" && symboltable.contains(*lexitr) && symboltable[*lexitr] == "t_string") {
            tokitr++; lexitr++;
            return true;
        }
    }
    return false;
}

bool SyntaxAnalyzer::whilestmt() {
    tokitr++; lexitr++;
    if (tokitr != tokens.end() && *tokitr == "s_lparen") {
        tokitr++; lexitr++;
        if (!logexpr()) {
            return false;
        }
        if (tokitr != tokens.end() && *tokitr == "s_rparen") {
            tokitr++; lexitr++;
            if (tokitr != tokens.end() && *tokitr == "t_loop") {
                tokitr++; lexitr++;
                if (!stmtlist()) {
                    return false;
                }
                if (tokitr != tokens.end() && *tokitr == "t_end") {
                    tokitr++; lexitr++;
                    if (tokitr != tokens.end() && *tokitr == "t_loop") {
                        tokitr++; lexitr++;
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool SyntaxAnalyzer::inputstmt() {
    tokitr++; lexitr++;
    if (tokitr != tokens.end() && *tokitr == "s_lparen") {
        tokitr++; lexitr++;
        if (tokitr != tokens.end() && *tokitr == "t_id") {
            if (symboltable.contains(*lexitr)) {
                tokitr++; lexitr++;
                if (tokitr != tokens.end() && *tokitr == "s_rparen") {
                    tokitr++; lexitr++;
                    return true;
                }
            }
        }
    }
    return false;
}

bool SyntaxAnalyzer::relop() {
    if (tokitr != tokens.end()) {
        if (*tokitr == "s_eq" ||
            *tokitr == "s_ne" ||
            *tokitr == "s_lt" ||
            *tokitr == "s_le" ||
            *tokitr == "s_gt" ||
            *tokitr == "s_ge") {
            tokitr++; lexitr++;
            return true;
        }
    }
    return false;
}

bool SyntaxAnalyzer::arithop() {
    if (tokitr != tokens.end()) {
        if (*tokitr == "s_plus"  ||
            *tokitr == "s_minus" ||
            *tokitr == "s_mult"  ||
            *tokitr == "s_div"   ||
            *tokitr == "s_mod") {
            tokitr++; lexitr++;
            return true;
        }
    }
    return false;
}
