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

#ifndef B49CBBD8_AE6E_4E8D_87A8_11CA61EE95A0
#define B49CBBD8_AE6E_4E8D_87A8_11CA61EE95A0

#include <modules/Base/Main.hpp>

#include <string>
#include <vector>

namespace ECMA1997 {
enum Token {
    Tok_SourceText,
    Tok_WhiteSpace,
    Tok_LineTerminator,
    Tok_SingleLineComment,
    Tok_MultiLineComment,
    Tok_Str,
    Tok_ProcessableIdent,
    
    Punk_EQ, Punk_CloseAngle, Punk_OpenAngle, Punk_EQEQ, Punk_LTEQ, Punk_GTEQ, //=, >, <, ==, <=, >=
    Punk_EXEQ, Punk_Comma, Punk_EX, Punk_Tilde, Punk_Question, Punk_Colon, //!=, ",", !, ~, ?, : 
    Punk_dot, Punk_ANDAND, Punk_PIPEPIPE, Punk_PLUSPLUS, Punk_MINUSMINUS, Punk_Plus, //., &&, ||, ++, --, +
    Punk_Minus, Punk_Star, Punk_ForwdSlash, Punk_Amp, Punk_Pipe, Punk_Hat, //-, *, /, &, |, ^
    Punk_Mod, Punk_OpenAngleAngle, Punk_CloseAngleAngle, Punk_CloseAngleAngleAngle, Punk_AddAsign, Punk_MinusAssign, //%, <<, >>, >>>, +=, -=
    Punk_StarAssign, Punk_SlashAsign, Punk_AmpAssign, Punk_PipeAssign, Punk_HatAssign, Punk_ModAssign, //*=, /=, &=, |=, ^=, %=
    Punk_OpenAngleAngleEQ, Punk_CloseAngleAngleEQ, Punk_CloseAngleAngleAngleEQ, Punk_OpenPrentheses, Punk_CloseParentheses, Punk_OpenCurlyBracket, //<<=, >>=, >>>=, (, ), {
    Punk_CloseCurlyBracket, Punk_OpenSquare, Punk_CloseSquare, Punk_Semicolon,

    Tok_IdentORKW, Tok_IDENT,

    KW_break, KW_for, KW_new, KW_var,
    KW_continue, KW_function, KW_return, KW_void,
    KW_delete, KW_if, KW_this, KW_while,
    KW_else, KW_in, KW_typeof, KW_with,
    KW_ReservedKeyWord,

    Lit_Null, Lit_True, Lit_False
};
enum State {
    State_Standard,
    State_CommentPossible,
    State_SingleLineComment,
    State_MultiLineComment,
    State_Str,
    State_Chr
};
struct Tok {
    Token Type;
    std::string val;
    Tok() {}
    Tok(Token type) {
        Type = type;
    }
};
class Main : public ECMABase::Main {
public:
    Main();
    ~Main();
    void Preload(int, char**);
    void Load(std::string&);
    void Postload(void);
    int Run(void);
private:
    void printTok(Tok*);
    std::vector<Tok*> ProcessedSource;
};
}

#endif /* B49CBBD8_AE6E_4E8D_87A8_11CA61EE95A0 */
