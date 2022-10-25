/*
 Copyright (c) 2022 Alex Huang <huangalex409@gmail.com>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "Main.hpp"

#include <string>
#include <fstream>
#include <iostream>
#include <list>

ECMA1997::Main::~Main() {}
ECMA1997::Main::Main() {}

void ECMA1997::Main::Preload(int argc, char** argv) {}

void ECMA1997::Main::Load(std::string& path) {
    std::ifstream FileHandle;
    FileHandle.open(path);
    std::string line, temp;
    ECMA1997::State StateEngine = ECMA1997::State::State_Standard;
    std::list<Tok*> toks;

    // lets save some memory, alloc some commonly used tokens and use them manytimes (dataless ones only)
    Tok* newline = new Tok();
    newline->Type = ECMA1997::Token::Tok_LineTerminator;
    Tok* space = new Tok();
    space->Type = ECMA1997::Token::Tok_WhiteSpace;

    /* stage one, pull out line terminators,
     * Spaces, and strings
     */
    while (std::getline(FileHandle, line)) {
        for (int index = 0; index < line.length(); index++) {
            char chr = line[index];
            char next;
            if (index == line.length() - 1) { //make sure we aren't at the end of the line
                next = '\n';
            } else {
                next = line[index + 1];
            }
            if (StateEngine == ECMA1997::State::State_Standard) {
                bool statechange = false;
                if (chr == '\'') { //"char" literal
                    StateEngine = ECMA1997::State::State_Chr;
                    statechange = true;
                }
                if (chr == '\"') { //"string" literal
                    StateEngine = ECMA1997::State::State_Str;
                    statechange = true;
                }
                if (chr == '/') { //"Possible" comment
                    StateEngine = ECMA1997::State::State_CommentPossible;
                }
                if (statechange) {
                    Tok* tmpnew = new Tok();
                    tmpnew->Type = ECMA1997::Token::Tok_MultiLineComment;
                    tmpnew->val = temp;
                    toks.push_back(tmpnew);
                    temp = "";
                }
            } else if (StateEngine == ECMA1997::State::State_Str || StateEngine == ECMA1997::State::State_Chr) {
                if ((StateEngine == ECMA1997::State::State_Str && chr == '\"') || (StateEngine == ECMA1997::State::State_Chr && chr == '\'')) {
                    //append to temp var until end of string is encountered.
                    Tok* tmpnew = new Tok();
                    tmpnew->Type = ECMA1997::Token::Tok_Str;
                    tmpnew->val = temp;
                    toks.push_back(tmpnew);
                    StateEngine = ECMA1997::State::State_Standard;
                    temp = "";
                } else {
                    temp += chr;
                }
                continue;
            } else if (StateEngine == ECMA1997::State::State_CommentPossible) {
                bool statechange = false;
                if (chr == '/') { //Line comment
                    StateEngine = ECMA1997::State::State_SingleLineComment;
                    statechange = true;
                } else if (chr == '*') { //Multiline comment
                    StateEngine = ECMA1997::State::State_MultiLineComment;
                    statechange = true;
                }
                if (statechange) {
                    Tok* tmpnew = new Tok();
                    tmpnew->Type = ECMA1997::Token::Tok_MultiLineComment;
                    tmpnew->val = temp;
                    toks.push_back(tmpnew);
                    temp = "";
                }
            } else if (StateEngine == ECMA1997::State::State_SingleLineComment) {
                if (next == '\n') {
                    temp += chr;
                    Tok* tmpnew = new Tok();
                    tmpnew->Type = ECMA1997::Token::Tok_SingleLineComment;
                    tmpnew->val = temp;
                    toks.push_back(tmpnew);
                    StateEngine = ECMA1997::State::State_Standard;
                    temp = "";
                } else {
                    temp += chr;
                }
                continue;
            } else if (StateEngine == ECMA1997::State::State_MultiLineComment) {
                if (chr == '*' && next == '/') {
                    Tok* tmpnew = new Tok();
                    tmpnew->Type = ECMA1997::Token::Tok_MultiLineComment;
                    tmpnew->val = temp;
                    toks.push_back(tmpnew);
                    StateEngine = ECMA1997::State::State_Standard;
                    temp = "";
                } else {
                    temp += chr;
                }
                continue;
            }

            /* If we got this far, it means that it's not a string, or comment
             * now we generate source text and whitespace tokens for the extras.
             */
            if (chr == ' ') {
                if (temp.length() > 0) {
                    Tok* tmpnew = new Tok();
                    tmpnew->Type = ECMA1997::Token::Tok_SourceText;
                    tmpnew->val = temp;
                    toks.push_back(tmpnew);
                    temp = "";
                }
                toks.push_back(space);
            } else {
                temp += chr;
            }
        }

        //push back on everything except if we are in a multiline comment
        if (StateEngine == ECMA1997::State::State_MultiLineComment) {
            temp += '\n';
        } else {
            toks.push_back(newline);
        }
    }

    /* purge comment tokens
     */
    {
        std::list<Tok*> newtoks;
        Tok* current;
        for (auto it = toks.begin(); it != toks.end(); it++) {
            current = *it;
            if (current->Type == ECMA1997::Token::Tok_MultiLineComment || current->Type == ECMA1997::Token::Tok_SingleLineComment) {
                delete current;
                continue;
            } else {
                newtoks.push_back(current);
            }
        }
        toks = newtoks;
    }


    /* Stage two, split the source text into more tokens.
     * in a nested scope to allow the c++ compiler to automatically free the newtoks list
     */
    {
        /* punctuator token initialization, memory save, humongous block of code, maybe i can do better?
         * Because this is all manually allocated (no automatic allocation)
         * none of these class instances are freed, which is what we want
         */
        Tok* EQ = new Tok(Punk_EQ); Tok* CloseAngle = new Tok(Punk_CloseAngle); Tok* OpenAngle = new Tok(Punk_OpenAngle); Tok* EQEQ = new Tok(Punk_EQEQ); Tok* LTEQ = new Tok(Punk_LTEQ); Tok* GTEQ = new Tok(Punk_GTEQ);
        Tok* NEQ = new Tok(Punk_EXEQ); Tok* Comma = new Tok(Punk_Comma); Tok* NOT = new Tok(Punk_EX); Tok* Tilde = new Tok(Punk_Tilde); Tok* Question = new Tok(Punk_Question); Tok* Colon = new Tok(Punk_Colon);
        Tok* Dot = new Tok(Punk_dot); Tok* LogiAnd = new Tok(Punk_ANDAND); Tok* LogiOr = new Tok(Punk_PIPEPIPE); Tok* Incr = new Tok(Punk_PLUSPLUS); Tok* Decr = new Tok(Punk_MINUSMINUS); Tok* Add = new Tok(Punk_Plus);
        Tok* Minus = new Tok(Punk_Minus); Tok* Multi = new Tok(Punk_Star); Tok* Div = new Tok(Punk_ForwdSlash); Tok* BitAnd = new Tok(Punk_Amp); Tok* BitOr = new Tok(Punk_Pipe); Tok* BitXOR = new Tok(Punk_Hat);
        Tok* Modulo = new Tok(Punk_Mod); Tok* LeftShift = new Tok(Punk_OpenAngleAngle); Tok* SignRightShift = new Tok(Punk_CloseAngleAngle); Tok* UnsignRightShift = new Tok(Punk_CloseAngleAngleAngle); Tok* PlusAssign = new Tok(Punk_AddAsign); Tok* MinusAssign = new Tok(Punk_MinusAssign);
        Tok* MultiAssign = new Tok(Punk_StarAssign); Tok* DivAssign = new Tok(Punk_SlashAsign); Tok* BitAndAssign = new Tok(Punk_AmpAssign); Tok* BitOrAssign = new Tok(Punk_PipeAssign); Tok* XORAssign = new Tok(Punk_HatAssign); Tok* ModAssign = new Tok(Punk_ModAssign);
        Tok* LeftShiftAssign = new Tok(Punk_OpenAngleAngleEQ); Tok* SignRightShiftAssign = new Tok(Punk_CloseAngleAngleEQ); Tok* UnsignRightShiftAssign = new Tok(Punk_CloseAngleAngleAngleEQ); Tok* OpenParentheses = new Tok(Punk_OpenPrentheses); Tok* CloseParentheses = new Tok(Punk_CloseParentheses); Tok* OpenCurly = new Tok(Punk_OpenCurlyBracket);
        Tok* CloseCurly = new Tok(Punk_CloseCurlyBracket); Tok* OpenSquare = new Tok(Punk_OpenSquare); Tok* CloseSquare = new Tok(Punk_CloseSquare); Tok* SemiColon = new Tok(Punk_Semicolon);

        std::list<Tok*> newtoks;
        Tok* current;
        for (auto it = toks.begin(); it != toks.end(); it++) {
            current = *it;
            if (current->Type != ECMA1997::Token::Tok_SourceText) {
                newtoks.push_back(current);
            } else {
                std::string temp;
                std::string section;
                for (int index = 0; index < section.length(); index++) {
                    char chr = section[index];
                    char next;
                    if (index == section.length() - 1) {
                        next = ' ';
                    } else {
                        next = section[index + 1];
                    }
                    
                    /* Checking for scope indicators (aka brackets)
                     */
                    bool PunctuatorFound = true;
                    Tok* ToAdd = nullptr;
                    if (chr == '{') {
                        ToAdd = OpenCurly;
                    } else if (chr == '}') {
                        ToAdd = CloseCurly;
                    } else if (chr == '[') {
                        ToAdd = OpenSquare;
                    } else if (chr == ']') {
                        ToAdd = CloseSquare;
                    } else if (chr == '(') {
                        ToAdd = OpenParentheses;
                    } else if (chr == ')') {
                        ToAdd = CloseParentheses;
                    } else {
                        PunctuatorFound = false;
                    }

                    /* Checking for unsortables, one char operators and denominators
                     * without anything to pair to
                     */
                    if (!PunctuatorFound) {
                    PunctuatorFound = true;
                    if (chr == ';') {
                        ToAdd = SemiColon;
                    } else if (chr == ',') {
                        ToAdd = Comma;
                    } else if (chr == '~') {
                        ToAdd = Tilde;
                    } else if (chr == '?') {
                        ToAdd = Question;
                    } else if (chr == ':') {
                        ToAdd = Colon;
                    } else if (chr == '.') {
                        ToAdd = Dot;
                    } else {
                        PunctuatorFound = false;
                    }
                    }

                    /* Checking for those that begin with EQ
                     */
                    if (!PunctuatorFound) {
                    PunctuatorFound = true;
                    if (chr == '=') {
                        if (next == '=') {
                            ToAdd = EQEQ;
                        } else {
                            ToAdd = EQ;
                        }
                    } else {
                        PunctuatorFound = false;
                    }
                    }

                    /* Checking for punctuators that begin with Close angle bracket
                     * There are a load of them with these (6 to be precise)
                     * Will also tack on the checks for open angle brackets
                     * 4 possible punctuators
                     */
                    if (!PunctuatorFound) {
                        PunctuatorFound = true;
                        if (chr == '>') { //possible next chars include ">" or "=" or anything else
                            if (next != '>' && next != '=') {
                                ToAdd = CloseAngle;
                            } else if (next == '>') { //next char ">" or "="
                                if (section.length() - 2 <= index) { //check that there is actually more
                                    if (section[index + 2] == '=') {
                                        ToAdd = SignRightShiftAssign;
                                    } else if (section[index + 2] == '>') { // nothing or "=" possible
                                        if (section.length() - 3 <= index) {
                                            if (section[index + 3] != '=') {
                                                ToAdd = UnsignRightShift;
                                            } else {
                                                ToAdd = UnsignRightShiftAssign;
                                            }
                                        } else {
                                            ToAdd = UnsignRightShift;
                                        }
                                    }
                                } else {
                                    ToAdd = SignRightShift;
                                }
                            } else if (next == '=') { 
                                ToAdd = GTEQ;
                            }
                        } else if (chr == '<') { //possible next chars: nothing or "=" or "<"
                            if (next != '=' && next != '<') {
                                ToAdd = OpenAngle;
                            } else if (next == '=') {
                                ToAdd = LTEQ;
                            } else if (next == '<') {
                                if (section.length() - 2 <= index) { //make sure we aren't doing a OOBR
                                    if (section[index + 2] != '=') {
                                        ToAdd = LeftShift;
                                    } else {
                                        ToAdd = LeftShiftAssign;
                                    }
                                } else {
                                    ToAdd = LeftShift;
                                }
                            }
                        } else {
                            PunctuatorFound = false;
                        }
                    }

                    /* Check for punctuators beginning with the exclamation operator
                     * or punctuators beginning with the ampersand operator
                     * or punctuators that begin with the pipe operator
                     */
                    if (!PunctuatorFound) {
                        PunctuatorFound = true;
                        if (chr == '!') {
                            if (next != '=') {
                                ToAdd = NOT;
                            } else {
                                ToAdd = NEQ;
                            }
                        } else if (chr == '&') {
                            if (next == '&') {
                                ToAdd = LogiAnd;
                            } else if (next == '=') {
                                ToAdd = BitAndAssign;
                            } else {
                                ToAdd = BitAnd;
                            }
                        } else if (chr == '|') {
                            if (next == '|') {
                                ToAdd = LogiOr;
                            } else if (next == '=') {
                                ToAdd = BitOrAssign;
                            } else {
                                ToAdd = BitOr;
                            }
                        } else {
                            PunctuatorFound = false;
                        }
                    }

                    /* Check for Math punctuators
                     * those who begin with (+, -, *, /, %)
                     * special case here is that the Hat operator is here too
                     */
                    if (!PunctuatorFound) {
                        PunctuatorFound = true;
                        if (chr == '+') {
                            if (next == '+') {
                                ToAdd = Incr;
                            } else if (next == '=') {
                                ToAdd = PlusAssign;
                            } else {
                                ToAdd = Add;
                            }
                        } else if (chr == '-') {
                            if (next == '-') {
                                ToAdd = Decr;
                            } else if (next == '=') {
                                ToAdd = MinusAssign;
                            } else {
                                ToAdd = Minus;
                            }
                        } else if (chr == '*') {
                            ToAdd = (next == '=') ? MultiAssign : Multi;
                        } else if (chr == '/') {
                            ToAdd = (next == '=') ? DivAssign : Div;
                        } else if (chr == '%') {
                            ToAdd = (next == '=') ? ModAssign : Modulo;
                        } else if (chr == '^') {
                            ToAdd = (next == '=') ? XORAssign : BitXOR;
                        } else {
                            PunctuatorFound = false;
                        }
                    }

                    /* if there was an Identifier we were processing prior,
                     * and we discover a punctuator,
                     * we generate a tok for the ident, and then add our punctuator
                     */
                    if (!PunctuatorFound) {
                        temp += chr;
                    } else {
                        if (!temp.empty()) {
                            Tok* newtok = new Tok(ECMA1997::Token::Tok_IdentORKW);
                            newtok->val = temp;
                            temp.clear();
                            newtoks.push_back(newtok);
                        }

                        if (ToAdd != nullptr) {
                            newtoks.push_back(ToAdd);
                        }
                    }
                }

                /* After processing, make sure that any ident we missed is mopped up
                 */
                if (!temp.empty()) {
                    Tok* newtok = new Tok(ECMA1997::Token::Tok_IdentORKW);
                    newtok->val = temp;
                    temp.clear();
                    newtoks.push_back(newtok);
                }

                /* After processing, delete the old token to not cause a mem leak
                 */
                delete current;
            }
        }

        toks = newtoks;
    }

    /* Process Idents into KW and actual idents
     */
    {   
        // Allocate tokens for keywords
        Tok* Break = new Tok(KW_break); Tok* For = new Tok(KW_for); Tok* New = new Tok(KW_new); Tok* Var = new Tok(KW_var);
        Tok* Continue = new Tok(KW_continue); Tok* Function = new Tok(KW_function); Tok* Return = new Tok(KW_return); Tok* Void = new Tok(KW_void);
        Tok* Delete = new Tok(KW_delete); Tok* If = new Tok(KW_if); Tok* This = new Tok(KW_this); Tok* While = new Tok(KW_while);
        Tok* Else = new Tok(KW_else); Tok* In = new Tok(KW_in); Tok* Typeof = new Tok(KW_typeof); Tok* With = new Tok(KW_with); 
        Tok* Reserved = new Tok(KW_ReservedKeyWord);

        std::vector<std::string> ReservedKeywords = {
            "case", "debugger", "export", "super",
            "catch", "default", "extends", "switch",
            "class", "do", "finally", "throw",
            "const", "enum", "import", "try"
        };

        std::list <Tok*> newtoks;
        Tok* current;
        for (auto it = toks.begin(); it != toks.end(); it++) {
            current = *it;
            if (current->Type != ECMA1997::Token::Tok_IdentORKW) {
                newtoks.push_back(current);
            } else {
                std::string val = current->val;
                if (val == "break") { newtoks.push_back(Break);}
                else if (val == "for") { newtoks.push_back(For);}
                else if (val == "new") { newtoks.push_back(New);}
                else if (val == "var") { newtoks.push_back(Var);}
                else if (val == "continue") { newtoks.push_back(Continue);}
                else if (val == "function") { newtoks.push_back(Function);}
                else if (val == "return") { newtoks.push_back(Return);}
                else if (val == "void") { newtoks.push_back(Void);}
                else if (val == "delete") { newtoks.push_back(Delete);}
                else if (val == "if") { newtoks.push_back(If);}
                else if (val == "this") { newtoks.push_back(This);}
                else if (val == "while") { newtoks.push_back(While);}
                else if (val == "else") { newtoks.push_back(Else);}
                else if (val == "in") { newtoks.push_back(In);}
                else if (val == "typeof") { newtoks.push_back(Typeof);}
                else if (val == "with") { newtoks.push_back(With); }
                else {
                    bool isReserved = false;
                    for (auto et = ReservedKeywords.begin(); et != ReservedKeywords.end(); it++) {
                        if (*et == val) {
                            isReserved = true;
                            break;
                        }
                    }
                    if (isReserved) {
                        //Make this Error more helpful than this.
                        std::cout << "Reserved keyword (" << val << ") used." << std::endl;
                        throw new std::runtime_error("A reserved keyword (" + val + ") was used in code.");
                    }
                }
            }
        }
        toks = newtoks;
    }

    // aaand we are done (for now)
    // throw the toks into a vector and we can call it a day
    this->ProcessedSource = std::vector<Tok*>{std::begin(toks), std::end(toks)};
}

void ECMA1997::Main::Postload() {

}

int ECMA1997::Main::Run() {

    return 0;
}