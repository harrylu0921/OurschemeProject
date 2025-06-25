#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <cstring>
#include <map>


using namespace std;

enum TokenType {
    LP, RP, INT, STRING, FLOAT, NIL, T, QUOTE, SYMBOL, DOT, SEMICON, DOUBLEQUOTE, UNRECOGNIZED, UNEXPECTED, END, NOQUOTE
};

struct Token {
    string content;
    TokenType type;
    int column ;
    int line ;
};

struct Node {
    Token token;  // 存放 Token，包含內容、行數、列數
    Node *left = NULL ;
    Node *right = NULL ;
    Node *identity = NULL; // 指出它是哪個symbol的值


};

struct Variable { // 存取被define的內容
  Node *node ;
  string name ;






};

Node *groot = new Node() ; //初始的文法樹
vector < Variable > Symbols ;// 存被定義變數
void PrintList(Node* node) ;
void PrettyPrint(Node* node) ;
Node *result = new Node() ;

bool printdefine = false ;

int level = 0 ;

void EvalSexp( Node *node, Node* &result, bool IsToplevel = false ) ;



vector<Token> tokenBuffer; // peek用
int gindex = -1 ;
int gcolumn = 1 ; // 列數
int gline = 1 ; // 行數
bool first = false ;
int gLP = 0 ;
int gRP = 0 ;
bool gend = false ;
vector< Token > Forprint  ; //輸出樹的內容
bool Noquote = false ; //有這個error嗎
bool Unexpected = false ;
bool Istoken = false ;
bool gpairs = false ; //判斷是否為pairs
map<string, Node*> SymbolIdentity;
bool Doterror = false ;



bool Seperator( char ch ) {
  if ( isspace( ch ) )
    return true ;
  else if ( ch == '(' || ch == ')' || ch == ';' || ch == '\'' || ch == '"' )
    return true ;
  else
    return false ;
} // Seperator()

bool escape( char ch ) { // 處理反斜線後接的東西
  if ( ch == '\\' || ch == '"' || ch == 'n' || ch == 't' )
    return true ;
  else
    return false ;

} // escape()

void BackToken() {
  gindex -= 1 ;


} // BackToken()

bool IsSymbol( Token token ) {
  if ( token.type == SYMBOL )
    return true ;
  else
    return false ;




}


Token GetToken() {
  char ch;
  string buffer;

  if (!tokenBuffer.empty()) {
    Token t = tokenBuffer.back();
    tokenBuffer.pop_back();
    return t;
  }

  while (cin.get(ch)) {
    if (Seperator(ch)) {
      if (isspace(ch)) {
        if (ch == '\n') {
          gcolumn = 1;
          if (first)
            gline += 1;
          first = true;


        }

        else
          gcolumn += 1 ;


      }

      else if (ch == '(' && cin.peek() != ')') {
        int begincolumn = gcolumn;
        int beginline = gline;
        while (isspace(cin.peek()) && !cin.eof()) {
          cin.get();
          if (cin.peek() == '\n') {
            gline += 1;
            gcolumn = 1;
          } else gcolumn += 1;
        } // while

        gcolumn += 1 ;


        if (cin.peek() != ')' && !cin.eof()) {
          Token temp;
          temp.content = "(";
          temp.type = LP;
          temp.column = begincolumn;
          temp.line = beginline;
          return temp;
        }

        else if (!cin.eof()) {
          cin.get();
          Token temp;
          temp.content = "nil";
          temp.type = NIL;
          temp.column = begincolumn;
          temp.line = beginline;
          gcolumn += 1;
          return temp;
        }
      }

      else if (ch == '(' && cin.peek() == ')') {
        cin.get(ch);
        Token temp;
        temp.content = "nil";
        temp.type = NIL;
        temp.column = gcolumn;
        temp.line = gline;
        gcolumn += 2;
        return temp;
      }

      else if (ch == ')') {
        Token temp;
        temp.content = ")";
        temp.type = RP;
        temp.column = gcolumn;
        temp.line = gline;
        gcolumn += 1 ;

        return temp;
      }

      else if (ch == '\'') {
        Token temp;
        temp.content = "\'";
        temp.type = QUOTE ;
        temp.column = gcolumn;
        temp.line = gline;
        gcolumn += 1 ;
        return temp;
      }

      else if (ch == ';') {
        while (!cin.eof() && ch != '\n') {
          cin.get(ch);
          gcolumn += 1;
        }
        gline += 1;
        gcolumn = 1;
      }

      else if (ch == '"') {
        int beginline = gline;
        int begincolumn = gcolumn;
        //cout << gline << gcolumn ;
        buffer += ch;
        if (!cin.get(ch)) break;
        int doublequote = 1;
        while (!cin.eof() && doublequote != 2 && ch != '\n') {
          if (ch == '"') {
            doublequote++;
            buffer += ch;
            gcolumn += 1 ;
          }

          else if (ch == '\\' && !escape(cin.peek())) {
            buffer += ch;
            if (!cin.get(ch))
              break;
            gcolumn += 1;
          }

          else if (ch == '\\' && escape(cin.peek())) {
            buffer += ch;
            if (!cin.get(ch))
              break;
            buffer += ch;
            gcolumn += 1;
            if (!cin.get(ch))
              break;
            gcolumn += 1;
          }

          else {
            buffer += ch;
            if (!cin.get(ch))
              break;
            gcolumn += 1;
          }





        }

        gcolumn += 1 ;

        if (doublequote >= 2) {
          Token token;
          token.content = buffer;
          token.type = STRING;
          token.line = beginline;
          token.column = begincolumn;
          return token;
        } else {
          buffer.clear();
          buffer += ch;
          Token token;
          token.content = buffer;
          token.type = NOQUOTE;
          token.line = gline;
          token.column = gcolumn;
          return token;
        }
      }
    }

    else if (ch == '.' && !isdigit(cin.peek()) && !isalpha(cin.peek()) && (Seperator(cin.peek()) || isspace(cin.peek()))) {

      Token token;
      token.content = ".";
      token.type = DOT;
      token.line = gline;
      token.column = gcolumn;
      gcolumn += 1 ;
      return token;
    }

    else if (ch == '#' && cin.peek() == 'f') {
      buffer += ch;
      int beginline = gline;
      int begincolumn = gcolumn;
      while (!Seperator(cin.peek()) && !cin.eof()) {
        cin.get(ch);
        gcolumn += 1;
        buffer += ch;
      }
      Token token;
      token.content = (buffer == "#f" ? "nil" : buffer);
      token.type = (buffer == "#f" ? NIL : SYMBOL);
      token.line = beginline;
      token.column = begincolumn;
      gcolumn += 1;
      return token;
    }

    else if (ch == '#' && cin.peek() == 't') {
      buffer += ch;
      int beginline = gline;
      int begincolumn = gcolumn;
      while (!Seperator(cin.peek()) && !cin.eof()) {
        cin.get(ch);
        gcolumn += 1;
        buffer += ch;
      }
      Token token;
      token.content = (buffer == "#t" ? "#t" : buffer);
      token.type = (buffer == "#t" ? T : SYMBOL);
      token.line = beginline;
      token.column = begincolumn;
      gcolumn += 1;
      return token;
    }

    else if (isalpha(ch)) {
      int beginline = gline;
      int begincolumn = gcolumn;
      buffer += ch;
      while (!Seperator(cin.peek()) && !cin.eof()) {
        cin.get(ch);
        gcolumn += 1;
        buffer += ch;
      }
      Token token;
      if (buffer == "t")
        token.content = "#t", token.type = T;
      else if (buffer == "nil")
        token.content = "nil", token.type = NIL;
      else
        token.content = buffer, token.type = SYMBOL;
      token.line = beginline;
      token.column = begincolumn;
      gcolumn += 1;
      return token;
    }

    else if (ch == '.' && isdigit(cin.peek())) {
      int beginline = gline;
      int begincolumn = gcolumn;
      buffer += ch;
      bool allnum = true;
      while (!Seperator(cin.peek()) && !cin.eof()) {
        if (!isdigit(cin.peek()))
          allnum = false;
        cin.get(ch);
        gcolumn += 1;
        buffer += ch;
      }
      Token token;
      if (allnum) {
        buffer.insert(0, "0");
        token.content = buffer;
        token.type = FLOAT;
      } else {
        token.content = buffer;
        token.type = SYMBOL;
      }
      token.column = begincolumn;
      token.line = beginline;
      gcolumn += 1;
      return token;
    }

    else if (isdigit(ch)) {
      buffer += ch;
      bool allnum = true;
      int dot = 0;
      bool symbol = false;
      int beginline = gline;
      int begincolumn = gcolumn;
      while (!Seperator(cin.peek()) && !cin.eof()) {
        if (!isdigit(cin.peek())) {
          allnum = false;
          if (cin.peek() == '.')
            dot += 1;
          else
            symbol = true;
        }
        cin.get(ch);
        gcolumn += 1;
        buffer += ch;
      }
      Token token;
      if (dot == 1 && !symbol)
        token.content = buffer, token.type = FLOAT;
      else if (dot > 1)
        token.content = buffer, token.type = SYMBOL;
      else if (allnum)
        token.content = buffer, token.type = INT;
      else
        token.content = buffer, token.type = SYMBOL;
      token.column = begincolumn;
      token.line = beginline;
      gcolumn += 1;
      return token;
    }

    else if (ch == '+' || ch == '-') {
      int beginline = gline;
      int begincolumn = gcolumn;
      buffer += ch;
      bool allnum = true;
      int dot = 0;
      bool symbol = false;
      bool digit = false;
      while (!Seperator(cin.peek()) && !cin.eof()) {
        if (!isdigit(cin.peek())) {
          allnum = false;
          if (cin.peek() == '.')
            dot += 1;
          else if (cin.peek() == '-' || cin.peek() == '+')
            symbol = true;
          else
            symbol = true;
        } else
          digit = true;
        cin.get(ch);
        gcolumn += 1;
        buffer += ch;
      }

      Token token;
      if (buffer == "+")
        token.content = "+", token.type = SYMBOL;
      else if (symbol)
        token.content = buffer, token.type = SYMBOL;
      else if (dot == 1 && !symbol && digit) {
        if (buffer[0] == '+') buffer.erase(0, 1);
        token.content = buffer;
        token.type = FLOAT;
      } else if (dot > 1) {
        token.content = buffer;
        token.type = SYMBOL;
      } else if (allnum) {
        if (buffer[0] == '+') buffer.erase(0, 1);
        token.content = buffer;
        token.type = INT;
      } else {
        token.content = buffer;
        token.type = SYMBOL;
      }

      token.column = begincolumn;
      token.line = beginline;
      gcolumn += 1;
      return token;
    }

    else {
      int beginline = gline;
      int begincolumn = gcolumn;
      buffer += ch;
      while (!Seperator(cin.peek()) && !cin.eof()) {
        cin.get(ch);
        gcolumn += 1;
        buffer += ch;
      }
      Token token;
      token.content = buffer;
      token.type = SYMBOL;
      token.column = begincolumn;
      token.line = beginline;
      gcolumn += 1;
      return token;
    }
  }

  if (cin.eof()) {
    Token token;
    token.content = "";
    token.type = END;
    return token;
  }
}

void UngetToken(Token t) {
    tokenBuffer.push_back(t); // 推回堆疊
}

Token PeekToken() {
    Token t = GetToken();
    UngetToken(t);
    return t;
}

void PrintString( Token token ) {
        string stored = "\0" ;
        int index = 0 ;
        int skewline = 0 ;
        int doublequote = 0 ;
        while ( doublequote != 2 ) {
          if ( token.content[index] == '\\' ) { // 字串中出現反斜線
            skewline += 1 ;
            stored += token.content[index] ;
            index += 1 ;
            while ( token.content[index] == '\\' ) {
              index += 1 ;
              skewline += 1 ;
            } // while

            if ( skewline == 1 ) { // 下一個字元非反斜線
              if ( token.content[index] == 'n' )
                cout << endl ;
              else { // 印出，除了遇到跳脫字元外
                if ( escape( token.content[index] ) )
                  cout << token.content[index] ;
                else {
                  cout << '\\' ;
                  cout << token.content[index] ;
                } // else

              } // else

            } // if

            else  { // 有多個反斜線，只取其一
              cout << '\\' ;
              cout << token.content[index] ;
            } // else



          } // if

          else {
            if ( token.content[index] == '"' ) // 算到第二次雙引號就結束，避免遇到空白(未知問題==)
              doublequote += 1 ;
            cout << token.content[index] ;
          } // else


          index += 1 ;
          skewline = 0 ;


        } // while

        cout << endl ;



} // PrintString()

void PrintFloat( Token token ) {
    float sum = atof( token.content.c_str() ) ;
    printf( "%.3f", sum ) ;
    cout << endl ;

} // PrintFloat()

void NoQuoteError( Token token ) {
  if ( token.type == NOQUOTE ) {
    if ( gline == 0 )
      gline = 1 ;
    cout << "ERROR (no closing quote) : END-OF-LINE encountered at Line " << gline << " Column " << gcolumn << endl ;

  } // if



} // ErrorMessage()

void UnExpectedError( Token token ) {
  cout << "ERROR (unexpected token) : atom or '(' expected when token at Line " << gline << " Column " << gcolumn << "is >>" << token.content << "<<" ;




} // UnExpectedError()

bool IsAtom( Token token ) {
  if ( token.type == SYMBOL || token.type == INT || token.type == FLOAT || token.type == STRING || token.type == NIL || token.type == T )
    return true ;
  else
    return false ;


} // IsAtom()

int MatchSymbol( string name ) {
  int i = 0 ;
  while ( i < Symbols.size() ) {
    if ( Symbols[i].name == name )
      return i ;
    else
      i += 1 ;
  } // while

  return -1 ;
}




void Parser(Node* &node) { // 解析token變成sexp
  Token token = PeekToken();
  //cout << token.line << "and" << token.column ;

  if (token.type == END) { // 如果遇到檔案結束
    gend = true;
    node = NULL;
    return;
  } // if


  if (IsAtom(token) ) { // atom直接放入
    token = GetToken();
    Forprint.push_back(token);
    node = new Node();
    node->token = token;
    node->left = NULL;
    node->right = NULL;
    return;
  } // if

  else if (token.type == QUOTE && Doterror == false ) { // 處理 quote
    GetToken();
    node = new Node();
    node->left = new Node();
    node->left->token.content = "quote";
    node->left->token.type = SYMBOL;
    node->left->left = NULL;
    node->left->right = NULL;
    node->right = new Node();
    Parser(node->right->left); // quote 的內容
    node->right->right = NULL;
    return;
  } // else if

  else if (token.type == RP && Doterror == false ) { // 遇到 ) 檢查錯誤
    GetToken();
    if (gLP == 0) {
      if (token.line == 0)
        token.line = 1;
      cout << "ERROR (unexpected token) : atom or '(' expected when token at Line "
           << token.line << " Column " << token.column << " is >>" << token.content << "<<" << endl;
      node = NULL;
      Unexpected = true;
      while ( cin.peek() != '\n' && !cin.eof() )
        cin.get();


      return;
    } // if
    else {
      gRP++;
      node = NULL;
      return;
    } // else
  } // else if

  else if (token.type == DOT && Doterror == false ) { // 單一個 DOT 是錯誤
    GetToken();
    while (cin.peek() != '\n' && !cin.eof() )
      cin.get();
    node = NULL;
    if (token.line == 0)
      token.line = 1;
    cout << "ERROR (unexpected token) : atom or '(' expected when token at Line "
         << token.line << " Column " << token.column << " is >>" << token.content << "<<" << endl;
    Unexpected = true;
    return;
  } // else if

  else if (token.type == LP) { // 處理一個新的 list
    gLP++;
    GetToken();
    Forprint.push_back(token);
    Token next = PeekToken();
    if (next.type == END) {
      gend = true;
      node = NULL;
      return;
    } // if

    node = new Node(); // 建新的 node
    Parser(node->left); // 處理第一個元素
    if ( Unexpected || Doterror || Noquote )
      return ;

    Node* cur = node;

    while (true) {
      Token peek = PeekToken();

      if (peek.type == NOQUOTE) { // 處理未關閉的字串錯誤
        peek = GetToken();
        NoQuoteError(peek);
        Noquote = true;
        node = NULL;
        return;
      } // if


      if (peek.type == END) { // 中途遇到 EOF
        gend = true;
        node = NULL;
        return;
      } // if

      if (peek.type == RP) { // list 結束
        GetToken();
        Forprint.push_back(peek);
        gRP++;
        cur->right = NULL;
        return;
      } // if

      else if (peek.type == DOT) { // 遇到 dot
        Forprint.push_back(peek);
        GetToken(); // 讀掉 dot

        if ( PeekToken().type == RP && Doterror == false ) {
          token = GetToken() ;
          if (token.line == 0)
            token.line = 1;
          cout << "ERROR (unexpected token) : atom or '(' expected when token at Line "
               << token.line << " Column " << token.column << " is >>" << token.content << "<<" << endl;
          node = NULL;
          Unexpected = true;
          while (cin.peek() != '\n' && !cin.eof() )
            cin.get();
          return;
        } // if

        else if ( PeekToken().type == DOT ) { // 連續兩個dot
          token = GetToken() ;
          if (token.line == 0)
            token.line = 1;
          cout << "ERROR (unexpected token) : atom or '(' expected when token at Line "
               << token.line << " Column " << token.column << " is >>" << token.content << "<<" << endl;
          node = NULL;
          Unexpected = true;
          while (cin.peek() != '\n' && !cin.eof() )
            cin.get();
          return;
        } // if


        cur->right = new Node();
        Parser(cur->right) ;
        if ( Unexpected || Doterror || Noquote)
          return ;



        if (PeekToken().type == END) {
          gend = true;
          node = NULL;
          return;
        } // if


        if (PeekToken().type != RP && Doterror == false ) { //遇到兩個.或是.後
          Token error = PeekToken();
          GetToken() ;
          cout << "ERROR (unexpected token) : ')' expected when token at Line "
               << error.line << " Column " << error.column << " is >>" << error.content << "<<" << endl;
          node = NULL;
          Unexpected = true;
          while (cin.peek() != '\n' && !cin.eof() )
            cin.get();
          Doterror = true ;
          return;
        } // if

        Token end = GetToken(); // 把 ) 吃掉
        Forprint.push_back(end);
        gRP++;
        return;
      } // else if

      else { // 下一個元素
        cur->right = new Node();
        cur = cur->right;
        Parser(cur->left);
        if ( Unexpected || Doterror || Noquote)
          return ;
      } // else
    } // while
  } // else if

  else if (token.type == NOQUOTE && Doterror == false ) { // 處理未關閉的字串錯誤
    token = GetToken();
    NoQuoteError(token);
    Noquote = true;
    node = NULL;
    return;
  } // else if

  else { // 其他奇怪的錯誤
    token = GetToken();
    node = NULL;
    return;
  } // else
} // Parser()

void PrintSpaces(int count) {
  for (int i = 0; i < count; ++i)
    cout << " ";
}

void PrintAtom(Token token, int indent) {
  PrintSpaces(indent);
  if (token.type == STRING)
    PrintString(token);
  else if (token.type == FLOAT)
    PrintFloat(token);
  else
    cout << token.content;
  cout << endl;
}

bool FunctionName( Token token ) {
  if ( token.content == "cons" || token.content == "list" || token.content == "quote" || token.content == "'" || token.content == "define" ||
       token.content == "cdr" || token.content == "car" || token.content == "eqv" || token.content == "equal" || token.content == "if"     ||
       token.content == "begin" || token.content == "cond" || token.content == "clean-environment" || token.content == "pair?" ||
       token.content == "null?" || token.content == "integer?" || token.content == "real?" || token.content == "number?"   ||
       token.content == "boolean?" || token.content == "symbol?" || token.content == "+" || token.content == "-"  ||
       token.content == "/" || token.content == "*" || token.content == "string?" || token.content == "not" || token.content == "=" ||
       token.content == ">" || token.content == "<" || token.content == ">=" || token.content == "<=" || token.content == "string-append" ||
       token.content == "string>?" || token.content == "string<?" || token.content == "string=?" || token.content == "eqv?" ||
       token.content == "equal?" || token.content == "and" || token.content == "or" || token.content == "atom?" || token.content == "if" ||
       token.content == "cond" || token.content == "begin" )
    return true ;

  else
    return false ;






}

bool IsProcedureName(Token token) { // 專門區隔or not
  string name = token.content;
  return name == "cons" || name == "car" || name == "cdr" || name == "+" ||
         name == "-" || name == "*" || name == "/" || name == "list" ||
         name == "equal?" || name == "eqv?" || name == "string-append" ||
         name == "string>?" || name == "string<?" || name == "string=?" ||
         name == "pair?" || name == "null?" || name == "atom?" ||
         name == "integer?" || name == "real?" || name == "number?" ||
         name == "symbol?" || name == "string?" || name == "boolean?";
}





void PrintSExp(Node* node, int indent = 0) {
    if (!node) return;
    if (!node->left && !node->right) {
        PrintSpaces(indent);
        if (node->token.type == STRING)
          PrintString(node->token);
        else if (node->token.type == FLOAT)
          PrintFloat(node->token);
        else if (IsProcedureName(node->token) )
          cout << "#<procedure " << node->token.content << ">" << endl;
        else
          cout << node->token.content << endl;
        return;
    }
    //cout << indent ;
    int gap = indent ;
    if ( indent >= 2)
      PrintSpaces(indent-gap);
    else
      PrintSpaces(indent);
    cout << "( ";
    Node* cur = node;
    bool first = true;
    while (cur) {
        if (cur->left->left || cur->left->right) {
            if (first) {
                PrintSExp(cur->left, indent + 2);
                first = false;
            } else {
                PrintSpaces(indent + 2);
                PrintSExp(cur->left, indent + 2);
            }
        } else {
            if (first) {
              if (node->left->token.type == STRING)
                PrintString(node->left->token);
              else if (node->left->token.type == FLOAT)
                PrintFloat(node->left->token);
              else if (IsProcedureName(node->left->token) )
                cout << "#<procedure " << node->left->token.content << ">" << endl;
              else
                cout << node->left->token.content << endl;
              first = false;
            }

            else {
                PrintSpaces(indent + 2);
                if (cur->left->token.type == STRING)
                  PrintString(cur->left->token);
                else if (cur->left->token.type == FLOAT)
                  PrintFloat(cur->left->token);
                else if (IsProcedureName(cur->left->token) )
                  cout << "#<procedure " << cur->left->token.content << ">" << endl;
                else
                  cout << cur->left->token.content << endl;
            }
        }
        if (!cur->right) {
            break;
        } else if (!cur->right->left && !cur->right->right) {
            if (cur->right->token.type == NIL) {
                break;
            } else {
                PrintSpaces(indent + 2);
                cout << "." << endl;
                PrintSExp(cur->right, indent + 2);
                break;
            }
        }
        cur = cur->right;
    }
    PrintSpaces(indent);
    cout << ")" << endl;
}

bool LPRPCount() { // 計算括號數量
  if ( gLP == gRP )
    return true ;

  else
    return false ;




} // LPRPCount()

// type 1 atom type 2 function

void HandleCons(Node* node, Node*& result) {
  // 計算參數數量
  Node* argnum = node;
  int argcount = 0;
  while (argnum->right != NULL) {
    argcount++;
    argnum = argnum->right;
  }

  if (argcount != 2) {
    result = NULL;
    cout << "ERROR (incorrect number of arguments) : cons" << endl;
    return;
  }

  node = node->right;
  Node* arg1 = node->left;
  Node* arg2 = node->right->left;

  Node* eval1 = NULL;
  Node* eval2 = NULL;

  EvalSexp(arg1, eval1,false);
  EvalSexp(arg2, eval2,false);

  result = new Node();
  result->left = eval1;
  result->right = eval2;

}

void HandleIsAtom( Node* node,Node* &result ) {
  Node *argnum = node->right ;
  int argcount = 0 ;
  while ( argnum != NULL ) {
    argcount ++ ;
    argnum = argnum->right ;
  } // while

  if ( argcount == 0 || argcount > 1 ) {
    result = NULL ;
    cout << "ERROR (incorrect number of arguments) : atom?" << endl ;
    return ;
  } // if

  node = node->right ;
  Node *eval ;
  if ( node != NULL ) {
    EvalSexp( node->left, eval,false ) ;
    if ( IsAtom( eval->token ) ) {
      result = new Node() ;
      result->token.content = "#t" ;
      result->token.type = T ;
      result->left = NULL ;
      result->right = NULL ;
    } // if

    else {
      result = new Node() ;
      result->token.content = "nil" ;
      result->token.type = NIL ;
      result->left = NULL ;
      result->right = NULL ;

    } // else


  } // if




}

void HandleBegin( Node* node, Node* &result ) {
  node = node->right ;
  Node *NeedPrint ; // 存最後不是null的節點要印
  while ( node != NULL ) {
    NeedPrint = node->left ;
    node = node->right ;
  } // WHILE

  EvalSexp( NeedPrint, result,false ) ;
  return ;

}

void HandleAndOr(Node* node, Node* &result) {
  Node *argnum = node ;
  int argcount = 0 ;

  while ( argnum->right != NULL ) {
    argcount++ ;
    argnum = argnum->right ;
  } // while

  if ( argcount < 2 ) {
    result = NULL;
    if ( node->left->token.content == "and" )
      cout << "ERROR (incorrect number of arguments) : and" << endl ;
    else if ( node->left->token.content == "or" )
      cout << "ERROR (incorrect number of arguments) : or" << endl ;
    return ;
  } // if

  Node *stored = node->right;
  Node *ans = NULL;

  if ( node->left->token.content == "and" ) {
    while ( stored != NULL ) {
      EvalSexp( stored->left, ans,false ) ;

      if ( ans->token.type == NIL ) {
        result = ans ;
        return ;
      } // if

      result = ans ;
      stored = stored->right ;
    } // while

  } // if

  else if ( node->left->token.content == "or" ) {
    while ( stored != NULL ) {
      EvalSexp( stored->left, ans,false ) ;

      if ( ans->token.type != NIL ) {
        result = ans ;
        return ;
      } // if

      result = ans ;
      stored = stored->right ;
    } // while

  } // else if

  return ;

}

void HandleIf(Node *node, Node* &result) {
  node = node->right;

  Node *testNode = node->left;
  Node *thenNode = node->right->left;
  Node *elseNode = NULL;

  if (node->right->right != NULL)
    elseNode = node->right->right->left;

  Node *testResult = NULL;
  EvalSexp(testNode, testResult,false);

  if (testResult == NULL || testResult->token.type == NIL) {
    if (elseNode != NULL)
      EvalSexp(elseNode, result,false);
    else
      result = NULL;
  } else {
    EvalSexp(thenNode, result,false);
  }
}

void HandleClean( Node *node, Node* &result ) {
  Node *argnum = node ;
  int argcount = 0 ;
  while ( argnum->right != NULL ) {
    argcount ++ ;
    argnum = argnum->right ;
  } // while

  if ( argcount > 0 ) {
    result = NULL ;
    cout << "ERROR (incorrect number of arguments) : clean-environment" << endl ;
    return ;
  } // if

  bool insideCompound = true;
  Node* cur = node;

  if ( cur->left != NULL && cur->left->token.content == "clean-environment" &&
      ( cur->right == NULL || cur->right->left == NULL ) ) {
    insideCompound = false;
  } // if

  if ( insideCompound == true ) {
    result = new Node() ;
    result->token.content = "ERROR (level of CLEAN-ENVIRONMENT)" ;
    result->left = NULL ;
    result->right = NULL ;
    return;
  } // if

  Symbols.clear() ;
  cout << "environment cleaned" ;

} // HandleClean()

Node* CopyNode( Node *src ) {
  if ( src == NULL ) return NULL ;
  Node *copy = new Node() ;
  copy->token = src->token ;
  copy->left = CopyNode( src->left ) ;
  copy->right = CopyNode( src->right ) ;
  return copy ;
}

void HandleCond(Node *node, Node* &result) {
  node = node->right;

  while (node != NULL) {
    Node *clause = node->left; // clause 是一個 list：比方 ((> a b) 1 2) 之類的

    if (clause != NULL ) {
      Node *testNode = clause->left ;
      Node *exprList = clause->right ;

      if (testNode != NULL &&
          testNode->token.type == SYMBOL &&
          testNode->token.content == "else" &&
          node->right == NULL) { // 也就是最後一個條件的else要視為else而不是symbol
        Node *lastExpr = exprList;
        while ( lastExpr != NULL && lastExpr->right != NULL )
          lastExpr = lastExpr->right;

        if ( lastExpr != NULL )
          EvalSexp( lastExpr->left, result,false ) ;
        return;
      }

      Node *testResult = NULL ;
      EvalSexp( testNode, testResult,false ) ;

      if ( testResult->token.type != NIL ) {
        Node *lastExpr = exprList ;
        while ( lastExpr != NULL && lastExpr->right != NULL )
          lastExpr = lastExpr->right ;

        if ( lastExpr != NULL )
          EvalSexp( lastExpr->left, result,false ) ;
        return;
      } // if00

    } // if

    node = node->right; // 處理下一個 clause
  }

  result = NULL;
}

void HandleDefine(Node* node) {
  // 檢查參數個數是否正確
  Node* arg = node;
  int argcount = 0;
  while (arg->right != NULL) {
    argcount++;
    arg = arg->right;
  }

  if (argcount != 2) {
    result = node;
    cout << "ERROR (DEFINE format) : ";
    return;
  }

  Node* defineNode = node->right;         // define 目標
  Node* valueNode = defineNode->right;    // 被定義的值

  if (defineNode->left == NULL || defineNode->left->token.content.empty()) {
    result = node;
    cout << "ERROR (DEFINE format) : ";
    return;
  }

  string name = defineNode->left->token.content;

  Node* evaluated = NULL;
  Node* valueToStore = NULL;

  if (valueNode->left->left == NULL && valueNode->left->right == NULL &&
      FunctionName(valueNode->left->token)) {
    valueToStore = new Node();
    valueToStore->token = valueNode->left->token;
    valueToStore->left = NULL;
    valueToStore->right = NULL;
  }
  else if (valueNode->left->token.type == SYMBOL &&
           SymbolIdentity.count(valueNode->left->token.content)) {
    valueToStore = SymbolIdentity[valueNode->left->token.content];
  }
  else {
    EvalSexp(valueNode->left, evaluated,false);
    valueToStore = evaluated;
  }

  // 看是否是重新定義
  bool redefined = false;
  int index = -1;

  for (int i = 0; i < Symbols.size(); i++) {
    if (Symbols[i].name == name) {
      redefined = true;
      index = i;
      break;
    } // if
  } // for

  if (!redefined) {
    Variable var;
    var.name = name;
    var.node = valueToStore;
    Symbols.push_back(var);
  } // if

  else {
    delete Symbols[index].node;
    Symbols[index].node = valueToStore;
  } // else

  SymbolIdentity[name] = valueToStore;

  cout << name << " defined";
} // HandleDefine()

void HandleList(Node* node, Node*& result) {
  Node* arg = node->right;

  if (arg == NULL) {
    result = new Node();
    result->token.type = NIL;
    result->token.content = "nil";
    result->left = NULL;
    result->right = NULL;
    return;
  }

  result = new Node();
  Node* current = result;

  while (arg != NULL) {
    Node* evaled = NULL;
    EvalSexp(arg->left, evaled,false);

    current->left = evaled;

    if (arg->right != NULL) {
      current->right = new Node(); 
      current = current->right;
    }

    else {
      current->right = new Node();
      current->right->token.type = NIL;
      current->right->token.content = "nil";
      current->right->left = NULL;
      current->right->right = NULL;
    }

    arg = arg->right;
  }
}

void HandleQuote( Node *node, Node* &result ) {
  Node *argnum = node ;
  int argcount = 0 ;
  while ( argnum->right != NULL ) {
    argcount ++ ;
    argnum = argnum->right ;
  } // while

  if ( argcount == 0 || argcount > 1 ) {
    result = NULL ;
    cout << "ERROR (incorrect number of arguments) : quote" << endl ;
    return ;
  } // if

  if ( node->right != NULL ) {
    node = node->right ;
    if ( node->left != NULL )
      result = node->left ;
  } // if

  else
    result = NULL ;

} // HandleQuote()

bool NodeIsAtom(Node* node) { // 因應需要檢查節點是否是atom的進階版，原來僅能檢查單一token
  return node != NULL &&
         (node->token.type == INT || node->token.type == FLOAT ||
          node->token.type == STRING || node->token.type == SYMBOL ||
          node->token.type == T || node->token.type == NIL);
}

void HandleCar(Node* node, Node*& result) {
  int argcount = 0;
  Node* argnum = node;
  while (argnum->right != NULL) {
    argcount++;
    argnum = argnum->right;
  }


  node = node->right;
  Node* arg = node->left;
  Node* evalArg = NULL;

  EvalSexp(arg, evalArg);

  if (evalArg == NULL || NodeIsAtom(evalArg)) {
    result = NULL;
    cout << "ERROR (car with non-cons value) : ";
    PrintSExp(evalArg);
    cout << endl;
    return;
  }

  if (evalArg->left != NULL) {
    result = evalArg->left;
  } else {
    result = NULL;
    cout << "ERROR (car: missing left part)" << endl;
  }
}

void HandleCdr(Node* node, Node*& result) {
  int argcount = 0;
  Node* argnum = node;
  while (argnum->right != NULL) {
    argcount++;
    argnum = argnum->right;
  }


  node = node->right;
  Node* arg = node->left;
  Node* evalArg = NULL;

  EvalSexp(arg, evalArg,false);

  if (evalArg == NULL || NodeIsAtom(evalArg) ) {
    result = NULL;
    cout << "ERROR (cdr with non-cons value) : ";
    PrintSExp(evalArg);
    cout << endl;
    return;
  }

  if (evalArg->right != NULL) {
    result = evalArg->right;
  } else {
    result = NULL;
    cout << "ERROR (cdr: missing right part)" << endl;
  }
}


void HandleIsPair( Node* node, Node* &result ) {
  Node *argnum = node ;
  int argcount = 0 ;
  while ( argnum->right != NULL ) {
    argcount ++ ;
    argnum = argnum->right ;
  } // while

  if ( argcount == 0 || argcount > 1 ) {
    result = NULL ;
    cout << "ERROR (incorrect number of arguments) : pair?" << endl ;
    return ;
  } // if

  Node* arg = NULL;
  Node* evalArg = NULL;

  if ( node->right != NULL && node->right->left != NULL ) {
    arg = node->right->left;
    EvalSexp(arg, evalArg,false);

    if ( evalArg && (evalArg->left != NULL || evalArg->right != NULL ) ) {
      result = new Node();
      result->token.type = T;
      result->token.content = "#t";
    }
    else {
      result = new Node();
      result->token.type = NIL;
      result->token.content = "nil";
    }
  }
  else {
    result = new Node();
    result->token.type = NIL;
    result->token.content = "nil";
  }
}

void HandleIsNullList( Node* node, Node* &result ) {
  Node *argnum = node ;
  int argcount = 0 ;
  while ( argnum->right != NULL ) {
    argcount ++ ;
    argnum = argnum->right ;
  } // while

  if ( argcount == 0 || argcount > 1 ) {
    result = NULL ;
    cout << "ERROR (incorrect number of arguments) : null?" << endl ;
    return ;
  } // if
  Node* arg = NULL;
  Node* evalArg = NULL;

  if ( node->right != NULL && node->right->left != NULL ) {
    arg = node->right->left;
    EvalSexp(arg, evalArg,false);

    if ( evalArg == NULL || ( IsAtom(evalArg->token) && evalArg->token.content == "nil") ) { // null 或是nil都視為空
      result = new Node();
      result->token.type = T;
      result->token.content = "#t";
      return;
    }

    result = new Node();
    result->token.type = NIL;
    result->token.content = "nil";
  }
  else { //右邊是null，或是右邊往下沒東西了
    result = new Node();
    result->token.type = NIL;
    result->token.content = "nil";
  }


}



void HandleIsInteger( Node* node, Node* &result ) {
  Node *argnum = node ;
  int argcount = 0 ;
  while ( argnum->right != NULL ) {
    argcount ++ ;
    argnum = argnum->right ;
  } // while

  if ( argcount == 0 || argcount > 1 ) {
    result = NULL ;
    cout << "ERROR (incorrect number of arguments) : integer?" << endl ;
    return ;
  } // if

  if ( node->right != NULL ) {
    node = node->right ;
    if ( node->right == NULL && node->left != NULL ) {

      Node* evaled = NULL ;
      EvalSexp( node->left, evaled,false ) ;

      if ( evaled != NULL && evaled->token.type == INT ) {
        result = new Node() ;
        result->token.content = "#t" ;
        result->token.type = T ;
        result->left = NULL ;
        result->right = NULL ;
      } // if
      else {
        result = new Node() ;
        result->token.content = "nil" ;
        result->token.type = NIL ;
        result->left = NULL ;
        result->right = NULL ;
      } // else

    } // if
    else {
      result = new Node() ;
      result->token.content = "nil" ;
      result->token.type = NIL ;
      result->left = NULL ;
      result->right = NULL ;
    } // else
  } // if
}

void HandleIsReal( Node* node, Node* &result ) {
  Node *argnum = node ;
  int argcount = 0 ;
  while ( argnum->right != NULL ) {
    argcount ++ ;
    argnum = argnum->right ;
  } // while

  if ( argcount == 0 || argcount > 1 && node->left->token.content == "real?" ) {
    result = NULL ;
    cout << "ERROR (incorrect number of arguments) : real?" << endl ;
    return ;
  } // if

  if ( argcount == 0 || argcount > 1 && node->left->token.content == "number?" ) {
    result = NULL ;
    cout << "ERROR (incorrect number of arguments) : number?" << endl ;
    return ;
  } // if

  if ( node->right != NULL ) {
    node = node->right ;
    if ( node->right == NULL && node->left != NULL ) {

      Node* evaled = NULL ;
      EvalSexp( node->left, evaled,false ) ; // 將( real? 後面的這串再作eval取值)

      if ( evaled != NULL &&
           ( evaled->token.type == INT || evaled->token.type == FLOAT ) ) { // 取值後為int或float均視為real
        result = new Node() ;
        result->token.content = "#t" ;
        result->token.type = T ;
        result->left = NULL ;
        result->right = NULL ;
      } // if
      else { // 其他情況都是nil
        result = new Node() ;
        result->token.content = "nil" ;
        result->token.type = NIL ;
        result->left = NULL ;
        result->right = NULL ;
      } // else

    } // if
    else {
      result = new Node() ;
      result->token.content = "nil" ;
      result->token.type = NIL ;
      result->left = NULL ;
      result->right = NULL ;
    } // else
  } // if
}

void HandleIsString( Node* node, Node* &result ) {
  Node *argnum = node ;
  int argcount = 0 ;
  while ( argnum->right != NULL ) {
    argcount ++ ;
    argnum = argnum->right ;
  } // while

  if ( argcount == 0 || argcount > 1 ) {
    result = NULL ;
    cout << "ERROR (incorrect number of arguments) : string?" << endl ;
    return ;
  } // if
  if ( node->right != NULL ) {
    node = node->right ;
    if ( node->right == NULL && node->left != NULL ) {

      Node* evaled = NULL ;
      EvalSexp( node->left, evaled,false ) ;

      if ( evaled != NULL && evaled->token.type == STRING ) {
        result = new Node() ;
        result->token.content = "#t" ;
        result->token.type = T ;
        result->left = NULL ;
        result->right = NULL ;
        return ;
      }
      else {
        result = new Node() ;
        result->token.content = "nil" ;
        result->token.type = NIL ;
        result->left = NULL ;
        result->right = NULL ;
        return ;
      }

    } // if
    else {
      result = new Node() ;
      result->token.content = "nil" ;
      result->token.type = NIL ;
      result->left = NULL ;
      result->right = NULL ;
      return ;
    }
  }
}

void HandleIsBoolean( Node* node, Node* &result ) {
  Node *argnum = node ;
  int argcount = 0 ;
  while ( argnum->right != NULL ) {
    argcount ++ ;
    argnum = argnum->right ;
  } // while

  if ( argcount == 0 || argcount > 1 ) {
    result = NULL ;
    cout << "ERROR (incorrect number of arguments) : boolean?" << endl ;
    return ;
  } // if


  if ( node->right != NULL ) {
    node = node->right ;
    if ( node->right == NULL && node->left != NULL ) {

      Node* evaled = NULL ;
      EvalSexp( node->left, evaled,false ) ;

      if ( evaled != NULL && ( evaled->token.type == T || evaled->token.type == NIL ) ) {
        result = new Node() ;
        result->token.content = "#t" ;
        result->token.type = T ;
        result->left = NULL ;
        result->right = NULL ;
      } // if

      else {
        result = new Node() ;
        result->token.content = "nil" ;
        result->token.type = NIL ;
        result->left = NULL ;
        result->right = NULL ;
      } // else

    } // if

    else {
      result = new Node() ;
      result->token.content = "nil" ;
      result->token.type = NIL ;
      result->left = NULL ;
      result->right = NULL ;
    } // else
  } // if

}

void HandleIsNot(Node* node, Node* &result) {
  Node *argList = node->right;


  int argCount = 0;
  Node *countNode = argList;
  while ( countNode != NULL ) {
    argCount++;
    countNode = countNode->right;
  }

  if ( argCount != 1 ) {
    result = NULL;
    cout << "ERROR (incorrect number of arguments) : not" << endl;
    return;
  }

  Node *arg = argList->left;
  Node *eval = NULL;
  EvalSexp(arg, eval,false);

  result = new Node();
  result->left = NULL;
  result->right = NULL;

  if ( eval->token.type == NIL ) {
    result->token.content = "#t";
    result->token.type = T;
  } // if

  else {
    result->token.content = "nil";
    result->token.type = NIL;
  } // else

}

void HandleIsSymbol( Node* node, Node*& result ) {
  Node *argnum = node ;
  int argcount = 0 ;
  while ( argnum->right != NULL ) {
    argcount ++ ;
    argnum = argnum->right ;
  } // while

  if ( argcount == 0 || argcount > 1 ) {
    result = NULL ;
    cout << "ERROR (incorrect number of arguments) : symbol?" << endl ;
    return ;
  } // if
  Node* evalResult = NULL;

  EvalSexp( node->right->left, evalResult,false );

  if ( evalResult != NULL && evalResult->left == NULL && evalResult->right == NULL && evalResult->token.type == SYMBOL ) {
    result = new Node();
    result->left = NULL;
    result->right = NULL;
    result->token.type = T;
    result->token.content = "#t";
  }

  else {
    result = new Node();
    result->left = NULL;
    result->right = NULL;
    result->token.type = NIL;
    result->token.content = "nil";
  }
}

bool IsPlus( Token token ) {
  if ( token.content == "+" )
    return true ;
  else
    return false ;
}

bool IsMinus( Token token ) {
  if ( token.content == "-" )
    return true ;
  else
    return false ;
}

bool IsMulti( Token token ) {
  if ( token.content == "*" )
    return true ;
  else
    return false ;
}

bool IsDivide( Token token ) {

  if ( token.content == "/" )
    return true ;
  else
    return false ;
}


void HandleArith(Node* node, Node*& result) {
  Node* argnum = node;
  int argcount = 0;
  while (argnum->right != NULL) {
    argcount++;
    argnum = argnum->right;
  }

  Node* run = node;
  double ans = 0;
  bool one = false;
  bool hasfloat = false;

  auto getValue = [&](Node* n) -> double {
    Node* evalArg = NULL;
    EvalSexp(n, evalArg,false);

    if (evalArg == NULL) {
      result = NULL;
      cout << "ERROR (eval failed): ";
      PrintSExp(n);
      cout << endl;
      return 0;
    }

    if (evalArg->token.type == INT || evalArg->token.type == FLOAT) {
      if (evalArg->token.type == FLOAT)
        hasfloat = true;
      return stod(evalArg->token.content);
    } else if (evalArg->token.type == SYMBOL) {
      // 如果符號沒有被eval成數字，當錯誤處理
      cout << "ERROR (non-number used in arithmetic): ";
      PrintSExp(evalArg);
      cout << endl;
      result = NULL;
      return 0;
    } else {
      cout << "ERROR (non-number used in arithmetic): ";
      PrintSExp(evalArg);
      cout << endl;
      result = NULL;
      return 0;
    }
  };

  if (IsPlus(node->left->token)) {
    while (run->right != NULL) {
      run = run->right;
      if (run->left != NULL) {
        double val = getValue(run->left);
        ans += val;
      }
    }
  } else if (IsMinus(node->left->token)) {
    while (run->right != NULL) {
      run = run->right;
      if (run->left != NULL) {
        double val = getValue(run->left);
        if (one == false)
          ans = val;
        else
          ans -= val;
        one = true;
      }
    }
  } else if (IsMulti(node->left->token)) {
    while (run->right != NULL) {
      run = run->right;
      if (run->left != NULL) {
        double val = getValue(run->left);
        if (one == false)
          ans = val;
        else
          ans *= val;
        one = true;
      }
    }
  } else if (IsDivide(node->left->token)) {
    while (run->right != NULL) {
      run = run->right;
      if (run->left != NULL) {
        double val = getValue(run->left);
        if (one == false)
          ans = val;
        else {
          bool ans_is_int = (floor(ans) == ans);
          bool val_is_int = (floor(val) == val);
          if (val == 0) {
            cout << "ERROR (division by zero) : /" << endl;
            result = NULL;
            return;
          }
          if (ans_is_int && val_is_int && !hasfloat)
            ans = (int)ans / (int)val;  // 整數除法
          else
            ans = ans / val;            // 浮點除法
        }
        one = true;
      }
    }
  }

  result = new Node();
  result->left = NULL;
  result->right = NULL;
  ostringstream oss;

  if (hasfloat) {
    oss << fixed << setprecision(6) << ans;
    result->token.type = FLOAT;
  } else {
    if (floor(ans) == ans)
      oss << (int)ans;
    else {
      oss << fixed << setprecision(6) << ans;
      result->token.type = FLOAT;
    }
  }

  result->token.content = oss.str();
  bool dot = (result->token.content.find('.') != string::npos);  // 判斷是否有小數點

  if (dot)
    result->token.type = FLOAT;
  else
    result->token.type = INT;
}

bool SymbolExit( vector <Variable> symbols, Token token, int &x ) { // 確認該symbol是否存在(被define過)
  int i = 0 ;
  while ( i < symbols.size() ) {
    if ( symbols[i].name == token.content ) {
       x = i ;
       return true ;
    } // if

    i += 1 ;
  } // while

  return false ;


}



void HandleLogical(Node* node, Node* &result) {
  Node* argnum = node;
  int argcount = 0;
  while (argnum->right != NULL) {
    argcount++;
    argnum = argnum->right;
  }



  string op = node->left->token.content;
  bool ans = true;
  double last = 0;
  bool firstnumpass = false;

  while (node->right != NULL) {
    node = node->right;
    Node* current = node->left;

    if (current == NULL) continue;

    if (current->token.type == SYMBOL) {
      int x = -1;
      if (!SymbolExit(Symbols, current->token, x)) {
        result = NULL;
        return;
      }
      current = Symbols[x].node;
    }

    if (current->token.type == QUOTE && current->right != NULL) {
      current = current->right->left; // 取quote
    }
    else if (current->left != NULL || current->right != NULL) {
      Node* sub_result = NULL;
      EvalSexp(current, sub_result,false);
      if (sub_result == NULL || (sub_result->token.type != INT && sub_result->token.type != FLOAT)) {
        result = NULL;
        return;
      }
      current = sub_result;
    }

    if (current->token.type != INT && current->token.type != FLOAT) {
      result = NULL;
      return;
    }

    double current_val = stod(current->token.content);

    if (!firstnumpass) {
      last = current_val;
      firstnumpass = true;
      continue;
    }

    if (op == "=") {
      if (last != current_val) {
        ans = false ;
        break ;
      } // if

    }  // if

    else if (op == ">") {
      if (last > current_val) {
        last = current_val ;
      }

      else {
        ans = false ;
        break ;

      } // else

    } // else if

    else if (op == "<") {
      if (last < current_val) {
        last = current_val ;
      } // if

      else {
        ans = false;
        break ;
      } // else

    } // else if

    else if (op == ">=") {
      if (last >= current_val) {
        last = current_val ;
      } // if

      else {
        ans = false ;
        break ;
      } // else
    } // else if

    else if (op == "<=") {
      if (last <= current_val) {
        last = current_val;
      }

      else {
        ans = false ;
        break;
      } // else
    } // else if

    else {
      result = NULL;
      return;
    } // else

  } // while

  result = new Node();
  result->left = NULL;
  result->right = NULL;

  if (ans) {
    result->token.content = "#t";
    result->token.type = T;
  } // if

  else {
    result->token.content = "nil";
    result->token.type = NIL;
  } // else
}

void HandleStringLogical(Node* node, Node* &result) {
  int argcount = 0 ;
  Node* argnum = node ;
  while ( argnum->right != NULL ) {
    argcount ++ ;
    argnum = argnum->right ;
  } // while


  string buffer;
  string last ;
  bool firststrpass = false ;
  bool ans = false ;


  if ( node->left->token.content == "string-append" ) {
    while ( node->right != NULL ) {
      node = node->right;
      if ( node->left != NULL ) {

        Node* evaled = NULL;
        EvalSexp( node->left, evaled,false );  // 先展開參數

        if ( evaled != NULL && evaled->token.type == STRING ) {
          string content = evaled->token.content;
          // 移除前後的引號
          if ( content.length() >= 2 && content.front() == '\"' && content.back() == '\"' )
            content = content.substr( 1, content.length() - 2 );
          buffer.append( content );
        }
      }
    }

    result = new Node();
    result->left = NULL;
    result->right = NULL;
    result->token.content = "\"" + buffer + "\"";
    result->token.type = STRING;
    return;
  }


  else if ( node->left->token.content == "string=?" ) {
    while ( node->right != NULL ) {
      node = node->right ;
      if ( node->left->token.type == STRING ) {
        if ( firststrpass == false ) {
          firststrpass = true ;
          last = node->left->token.content ;

        } // if

        else {
          string current = node->left->token.content ;
          if ( strcmp( last.c_str(), current.c_str() ) == 0 )
            ans = true ;

          else {
            ans = false ;
            node->right = NULL ;
          } // else

          last = current ;


        } // else



      } // if

      else if ( node->left->token.type == SYMBOL) {
        int x = -1 ;
        if ( SymbolExit( Symbols, node->left->token, x ) ) {
          if ( firststrpass == false ) {
            firststrpass = true ;
            last = Symbols[x].node->token.content ;

          } // if

          else {
            string current = Symbols[x].node->token.content ; ;
            if ( strcmp( last.c_str(), current.c_str() ) == 0 )
              ans = true ;

            else {
              ans = false ;
              node->right = NULL ;
            } // else

            last = current ;

          } // else



        } // else if

      } // else if

      else {
        Node* evaluated = NULL;
        Node *temp = node->left ;
        EvalSexp( temp, evaluated,false); //


        if (evaluated != NULL && evaluated->token.type == STRING) {
          if ( firststrpass == false ) {
            firststrpass = true;
            last = evaluated->token.content;
          }

          else {
            string current = evaluated->token.content;
            if ( strcmp(last.c_str(), current.c_str()) == 0 )
              ans = true;
            else {
              ans = false;
              node->right = NULL ;
            }

            last = current;
          } // else
        } // if

      } // else



    } // WHILE


  } // else if

  else if ( node->left->token.content == "string<?" ) {
    while ( node->right != NULL ) {
      node = node->right ;
      if ( node->left->token.type == STRING ) {
        if ( firststrpass == false ) { // 第一個要先記住，方便跟下一個比較
          firststrpass = true ;
          last = node->left->token.content ;

        } // if

        else {
          string current = node->left->token.content ;
          if ( strcmp( last.c_str(), current.c_str() ) < 0 )
            ans = true ;

          else {
            ans = false ;
            node->right = NULL ;
          } // else

          last = current ;

        } // else



      } // if

      else if ( node->left->token.type == SYMBOL) {
        int x = -1 ;
        if ( SymbolExit( Symbols, node->left->token, x ) ) {
          if ( firststrpass == false ) {
            firststrpass = true ;
            last = Symbols[x].node->token.content ;

          } // if

          else {
            string current = Symbols[x].node->token.content ; ;
            if ( strcmp( last.c_str(), current.c_str() ) < 0 )
              ans = true ;

            else {
              ans = false ;
              node->right = NULL ;
            } // else

            last = current ;

          } // else



        } // if

      } // else if

      else {
        Node* evaluated = NULL;
        Node *temp = node->left ;
        EvalSexp( temp, evaluated,false); //


        if (evaluated != NULL && evaluated->token.type == STRING) {
          if ( firststrpass == false ) {
            firststrpass = true;
            last = evaluated->token.content;
          }

          else {
            string current = evaluated->token.content;
            if ( strcmp(last.c_str(), current.c_str()) < 0 )
              ans = true;
            else {
              ans = false;
              node->right = NULL ;
            }

            last = current;
          } // else
        } // if

      } // else


    } // WHILE
  } // else if

  else if ( node->left->token.content == "string>?" ) {
    while ( node->right != NULL ) {
      node = node->right ;
      if ( node->left->token.type == STRING ) {
        if ( firststrpass == false ) {
          firststrpass = true ;
          last = node->left->token.content ;

        } // if

        else {
          string current = node->left->token.content ;
          if ( strcmp( last.c_str(), current.c_str() ) > 0 )
            ans = true ;

          else {
            ans = false ;
            node->right = NULL ;
          } // else

          last = current ;

        } // else



      } // if

      else if ( node->left->token.type == SYMBOL) {
        int x = -1 ;
        if ( SymbolExit( Symbols, node->left->token, x ) ) {
          if ( firststrpass == false ) {
            firststrpass = true ;
            last = Symbols[x].node->token.content ;

          } // if

          else {
            string current = Symbols[x].node->token.content ; ;
            if ( strcmp( last.c_str(), current.c_str() ) > 0 )
              ans = true ;

            else {
              ans = false ;
              node->right = NULL ;
            } // else

            last = current ;

          } // else



        } // if

      } // else if

      else {
        Node* evaluated = NULL;
        Node *temp = node->left ;
        EvalSexp( temp, evaluated,false); //


        if (evaluated != NULL && evaluated->token.type == STRING) {
          if ( firststrpass == false ) {
            firststrpass = true;
            last = evaluated->token.content;
          }

          else {
            string current = evaluated->token.content;
            if ( strcmp(last.c_str(), current.c_str()) > 0 )
              ans = true;
            else {
              ans = false;
              node->right = NULL ;
            }

            last = current;
          } // else
        } // if

      } // else


    } // WHILE
  } // else if

  if ( ans == true ) {
    result = new Node() ;
    result->left == NULL ;
    result->right == NULL ;
    result->token.content = "#t" ;
    result->token.type = T ;
  } // if

  else {
    result = new Node() ;
    result->left == NULL ;
    result->right == NULL ;
    result->token.content = "nil" ;
    result->token.type = NIL ;
  } //else









} // HandleStringLogical()

bool IsEqual( Node* a, Node* b ) {
  if ( a == NULL && b == NULL )
    return true ;

  if ( a == NULL && b != NULL && b->token.type == NIL )
    return true ;
  if ( b == NULL && a != NULL && a->token.type == NIL )
    return true ;

  if ( a == NULL || b == NULL )
    return false ;

  bool aIsAtom = ( a->left == NULL && a->right == NULL ) ;
  bool bIsAtom = ( b->left == NULL && b->right == NULL ) ;

  if ( aIsAtom && bIsAtom ) {
    if ( ( a->token.type == INT && b->token.type == FLOAT ) ||
         ( a->token.type == FLOAT && b->token.type == INT ) ) {
      double firstnum = stod( a->token.content ) ;
      double secondnum = stod( b->token.content ) ;
      return firstnum == secondnum ;
    }

    return a->token.type == b->token.type &&
           a->token.content == b->token.content ;
  }

  if ( aIsAtom != bIsAtom )
    return false ;

  return IsEqual( a->left, b->left ) && IsEqual( a->right, b->right ) ;
}

bool IsEqv( Node* a, Node* b ) {
  if ( a == NULL || b == NULL )
    return false ;

  if ( a == b )
    return true ;

  bool aIsAtom = ( a->left == NULL && a->right == NULL ) ;
  bool bIsAtom = ( b->left == NULL && b->right == NULL ) ;

  // 如果是 atom
  if ( aIsAtom && bIsAtom ) {
    if ( ( a->token.type == INT && b->token.type == FLOAT ) ||
         ( a->token.type == FLOAT && b->token.type == INT ) ) {
      double firstnum = stod( a->token.content ) ;
      double secondnum = stod( b->token.content ) ;
      return firstnum == secondnum ;
    }

    if ( a->token.type == SYMBOL && b->token.type == SYMBOL &&
         a->token.content == b->token.content )
      return true ;

    // 其他 atom，型別與內容需完全相同（string除外）
    if ( a->token.type == b->token.type &&
         a->token.content == b->token.content &&
         a->token.type != STRING )
      return true ;

    return false ;
  }

  return false ;
}



void HandleEqualAndEqv( Node *node, Node* &result ) {
  Node *argnum = node ;
  int argcount = 0 ;
  while ( argnum->right != NULL ) {
    argcount ++ ;
    argnum = argnum->right ;
  } // while

  if ( argcount == 0 || argcount == 1 || argcount > 2 ) {
    result = NULL ;
    if ( node->left->token.content == "eqv?")
      cout << "ERROR (incorrect number of arguments) : eqv?" << endl ;
    if ( node->left->token.content == "equal?")
      cout << "ERROR (incorrect number of arguments) : equal?" << endl ;
    return ;
  } // if
  bool atom = false ;
  bool symbol = false ;
  bool ans = false ;
  bool firstonepass = false ;
  Node *first = NULL ;
  Node *second = NULL ;

  if ( node->left->token.content == "eqv?" || node->left->token.content == "equal?" ) {

    if ( node->right != NULL && node->right->right != NULL ) {
      Node *arg1 = node->right->left ;
      Node *arg2 = node->right->right->left ;
      Node *stored1 = arg1 ;
      Node *stored2 = arg2 ;
      

      EvalSexp( arg1, first,false ) ;
      EvalSexp( arg2, second,-false ) ;

      if ( node->left->token.content == "eqv?" ) {
        if ( first == NULL || second == NULL ) {
          ans = false ;

        } // if



        else if ( first->left == NULL && first->right == NULL &&
                  second->left == NULL && second->right == NULL ) {
          if ( first->token.type == second->token.type &&
               first->token.content == second->token.content && first->token.type != STRING ) // 字串是eqv的例外狀況，要單獨拿出來

            ans = true ;
          else if ( ( first->token.type == INT && second->token.type == FLOAT ) || ( first->token.type == FLOAT && second->token.type == INT ) ) { // ( eqv 3 3.0 ) 為#t
            double firstnum = stod( first->token.content ) ;
            double secondnum = stod( second->token.content ) ;
            if ( firstnum == secondnum )
              ans = true ;
          } // else if


          else if (first->token.type == SYMBOL && second->token.type == SYMBOL) {
            string name1 = first->token.content;
            string name2 = second->token.content;
            if (SymbolIdentity.count(name1) && SymbolIdentity.count(name2)) {
              if (SymbolIdentity[name1] == SymbolIdentity[name2]) {
                ans = true;
              } // if

            } // if

          } // else if

          else {
            //cout << "ss" ;
            if ( ( arg1->token.type == SYMBOL && arg2->token.type == SYMBOL ) && ( arg1->token.content == arg2->token.content ) ) // symbol相同
              ans = true ;
            else {
              //cout << "ss" ;
              ans = false ;
            } // else

          } // else

        } // else if

        else {

          if ( first->left == NULL && first->right == NULL &&
               second->left == NULL && second->right == NULL ) {
            if ( first->token.type == second->token.type &&
                 first->token.content == second->token.content &&
                 first->token.type != STRING ) // string 是例外
              ans = true;
            else if ( (first->token.type == INT && second->token.type == FLOAT) ||
                (first->token.type == FLOAT && second->token.type == INT) ) {
              double num1 = stod(first->token.content);
              double num2 = stod(second->token.content);
              if ( num1 == num2 )
                ans = true;
            } // else if

          } // if

          else if (first == second) {
            ans = true;
          }

          else {
            if ( stored1->token.type == SYMBOL && stored2->token.type == SYMBOL && ( stored1->token.content == stored2->token.content ) )
              ans = true ;

            else if (  stored1->token.type == SYMBOL && stored2->token.type == SYMBOL && ( stored1->token.content != stored2->token.content ) ) {
              if (first->identity != NULL && second->identity != NULL) {
                if (first->identity == second->identity)
                  ans = true;
              }

            } // else if

            else
              ans = false;
          } // else

        } // else

      } // if


      else

        ans = IsEqual( first, second ) ;


    } // if

  } // if

  result = new Node() ;
  result->left = NULL ;
  result->right = NULL ;

  if ( ans ) {
    result->token.content = "#t" ;
    result->token.type = T ;
  }

  else {

    result->token.content = "nil" ;
    result->token.type = NIL ;
  }
}

Node* DeepCopy(Node* node) {
  if (node == NULL) return NULL;

  Node* copy = new Node();
  copy->token = node->token; 
  copy->left = DeepCopy(node->left);  
  copy->right = DeepCopy(node->right); 
  return copy;
}

void PrintSymbolTable() {
  cout << "=== Symbol Table ===" << endl;
  for (int i = 0; i < Symbols.size(); i++) {
    cout << Symbols[i].name << " -> ";
    Node* val = Symbols[i].node;

    if (val == NULL) {
      cout << "NULL" << endl;
    } else if (val->left == NULL && val->right == NULL) {
      if (val->token.type == SYMBOL)
        cout << "[SYMBOL] " << val->token.content << endl;
      else if (val->token.type == STRING)
        cout << "[STRING] \"" << val->token.content << "\"" << endl;
      else
        cout << "[ATOM] " << val->token.content << endl;
    } else {
      cout << "[COMPOUND]" << endl;
      PrintSExp(val, 2);
    }
  }
}

Node* ResolveSymbolToFunction(Node* node) {
  while (node != NULL && node->left == NULL && node->right == NULL && IsSymbol(node->token)) {
    int x = 0;
    if (!SymbolExit(Symbols, node->token, x))
      break;
    node = Symbols[x].node;
  }
  return node;
}





void EvalSexp( Node *node, Node* &result, bool IsToplevel ) {

  if (node->left == NULL && node->right == NULL) { // 純 atom
    if (!IsSymbol(node->token)) {
      if (node->token.type != QUOTE) {
        result = node;
        return;
      } else {
        result = node->right;
        return;
      }
    }

    else {
      int i = 0;
      bool match = false;

      while (i < Symbols.size() && !match) {
        if (Symbols[i].name == node->token.content)
          match = true;
        else
          i++;
      }

      if (match) {
        result = DeepCopy(Symbols[i].node);
        result->identity = Symbols[i].node;
        return;
      }

      else if (FunctionName(node->token)) {
        result = new Node();
        result->left = NULL;
        result->right = NULL;
        result->token = node->token;
        return;
      }

      else {
        cout << "ERROR (unbound symbol) : " << node->token.content << endl;
        result = NULL;
        return;
      }
    }
  }

  else if (node->left != NULL) {
    if (FunctionName(node->left->token)) {
      if (node->left->token.content == "cons")
        HandleCons(node, result);
      else if (node->left->token.content == "clean-environment") {
          HandleClean(node,result ) ;
          return ;
      } // else if

      else if (node->left->token.content == "define")
        HandleDefine(node);
      else if (node->left->token.content == "list")
        HandleList(node, result);
      else if (node->left->token.content == "quote")
        HandleQuote(node, result);
      else if (node->left->token.content == "car")
        HandleCar(node, result);
      else if (node->left->token.content == "cdr")
        HandleCdr(node, result);
      else if (node->left->token.content == "pair?")
        HandleIsPair(node, result);
      else if (node->left->token.content == "null?")
        HandleIsNullList(node, result);
      else if (node->left->token.content == "integer?")
        HandleIsInteger(node, result);
      else if (node->left->token.content == "real?")
        HandleIsReal(node, result);
      else if (node->left->token.content == "number?")
        HandleIsReal(node, result);
      else if (node->left->token.content == "string?")
        HandleIsString(node, result);
      else if (node->left->token.content == "boolean?")
        HandleIsBoolean(node, result);
      else if (node->left->token.content == "symbol?")
        HandleIsSymbol(node, result);
      else if (node->left->token.content == "+" || node->left->token.content == "-" ||
               node->left->token.content == "*" || node->left->token.content == "/")
        HandleArith(node, result);
      else if (node->left->token.content == "not")
        HandleIsNot(node, result);
      else if (node->left->token.content == "=" || node->left->token.content == ">" ||
               node->left->token.content == "<" || node->left->token.content == ">=" ||
               node->left->token.content == "<=")
        HandleLogical(node, result);
      else if (node->left->token.content == "string-append" || node->left->token.content == "string>?" ||
               node->left->token.content == "string<?" || node->left->token.content == "string=?")
        HandleStringLogical(node, result);
      else if (node->left->token.content == "equal?" || node->left->token.content == "eqv?")
        HandleEqualAndEqv(node, result);
      else if (node->left->token.content == "and" || node->left->token.content == "or")
        HandleAndOr(node, result);
      else if (node->left->token.content == "atom?")
        HandleIsAtom(node, result);
      else if (node->left->token.content == "if")
        HandleIf(node, result);
      else if (node->left->token.content == "cond")
        HandleCond(node, result);
      else if (node->left->token.content == "begin")
        HandleBegin(node, result);
    }

    else if (node->left->token.type == SYMBOL) {
      int x = 0;
      if (SymbolExit(Symbols, node->left->token, x)) {
        Node* resolvedFn = NULL;
        EvalSexp(Symbols[x].node, resolvedFn);

        if (resolvedFn == NULL) {
          result = NULL;
          return;
        }

        Node* actualFn = ResolveSymbolToFunction(resolvedFn);

        if (actualFn->left == NULL && actualFn->right == NULL &&
            FunctionName(actualFn->token)) {
          Node* newCall = new Node();
          newCall->left = actualFn;
          newCall->right = node->right;
          EvalSexp(newCall, result);
          return;
        }

        else {
          //cout << "ss" ;
          cout << "ERROR (attempt to apply non-function) : " << actualFn->token.content << endl;
          result = NULL;
          return;
        }
      }


      else {
        cout << "ERROR (unbound symbol) : " << node->left->token.content << endl;
        result = NULL;
        return;
      }
    }

    else if (node->left->token.type == QUOTE) {
      HandleQuote(node, result);
      return;
    }

    else {
      Node* eval = node->left;
      while (eval != NULL && eval->token.content.size() == 0) {
        eval = eval->left;
      }

      Node* funcResolved = NULL;
      EvalSexp(node->left, funcResolved);

      if (funcResolved == NULL) {
        result = NULL;
        return;
      }

      // 如果解出來是 function，就執行
      Node* actualFn = funcResolved;
      while (actualFn->left == NULL && actualFn->right == NULL &&
             IsSymbol(actualFn->token)) {
        int y = 0;
        if (SymbolExit(Symbols, actualFn->token, y)) {
          actualFn = Symbols[y].node;
        }
        else break;
      }

      if (actualFn->left == NULL && actualFn->right == NULL &&
          FunctionName(actualFn->token)) {
        Node* newCall = new Node();
        newCall->left = actualFn;
        newCall->right = node->right;
        EvalSexp(newCall, result);
        return;
      }

      cout << "ERROR (attempt to apply non-function) : " << actualFn->token.content << endl;
      result = NULL;
      return;
    }
  }

  else {
    cout << "ERROR (attempt to apply non-function) : " << node->left->token.content << endl;
    result = NULL;
    return;
  }
}



int main() {
    cout << "Welcome to OurScheme!" << endl << endl ;
    int test = 0 ;
    if ( cin.eof() ) {
       cout << endl ;
       cout << "> " << "ERROR (no more input) : END-OF-FILE encountered" ;
       return 0;
    } // if
    cin >> test ;


    while ( gend == false ) {
      cout << "> " ;
      if ( cin.eof() ) {
        cout << "ERROR (no more input) : END-OF-FILE encountered" ;
        gend = true ;
      } // if


      else {
        Parser( groot ) ;
        if ( groot == NULL ) {
          if ( cin.eof() ) {
            cout << "ERROR (no more input) : END-OF-FILE encountered" ;
            gend = true ;
          } // if

          else if (Doterror || Unexpected || Noquote) {
            //cout << endl ;
            //Unexpected = false ;
            gend = true ;
          }

          else {
            gcolumn = 1 ;
            gline = 1 ;
            Noquote = false ;
            Unexpected = false ;
            Doterror = false ;
            //cout << "ss" << endl ;

          } // else

        } // if

        else if ( groot != NULL ) {


          if (Doterror || Unexpected || Noquote) {
            //cout << endl ;
            gend = true ;
          }

          if ( cin.eof() ) {
            cout << "ERROR (no more input) : END-OF-FILE encountered" ;
            gend = true ;
          } // if


          if ( groot->left != NULL && groot->right == NULL ) { // 處理( exit )
            if ( groot->left->token.content == "exit" )
              gend = true ;


          } // if

          //if ( groot->left != NULL && groot->right != NULL ) { // 處理 ( exit . nil )
            //if ( groot->left->token.content == "exit" && groot->right->token.type == NIL )
              //gend = true ;


          //} // if

        } // else if



        if ( gend == false ) { // 沒有error開始evaluate
          EvalSexp( groot, result,true ) ;
          if ( printdefine == false )
            PrintSExp( result, 0 );
          //delete result ;
          result = new Node() ;
          bool comment = false ;
          gcolumn = 1 ;
          gline = 1 ;
          while ( cin.peek() != '\n' && !cin.eof() && comment == false && Istoken == false ) { //處理sexp後面同行殘留的輸出
            if ( isspace( cin.peek() ) || cin.peek() == ';' ) {// 是空白或分號，讀掉
              if ( cin.peek() == ';' )
                comment = true ;

               if ( isspace( cin.peek() ) ) // 空白的話要先記錄column
                 gcolumn += 1 ;
            cin.get() ;

            } // if

            else // 不是註解 不讀 跳出迴圈
              Istoken = true ;

          } // while

          if ( cin.peek() == '\n' && !comment && !Istoken ) // 不是註解也不是token，純空白
            ;
            //cin.get() ;

          else if ( comment ) {
            while ( cin.peek() != '\n' && !cin.eof() )
              cin.get() ;

            //cin.get() ;

          } //else if

          else if ( Istoken )
            ;


          cout << endl ;
          delete groot ;
          groot = new Node() ;
          Noquote = false ;


          if ( !Istoken ) {
            gcolumn = 1 ;
            gline = 0 ;
          } // if

          else { // 33 44 ，column保留，行數設為0，因為換行並未讀掉且也不須reset
              gline = 1 ;

          } // else

          Forprint.clear() ;
          //first = false ;
          Unexpected = false ;

        } // if

        else
          ;


        gLP = 0 ;
        gRP = 0 ;
        if ( Unexpected || Doterror || Noquote ) {
          Unexpected = false ;
          cout << endl ;
          gend = false ;
          Noquote = false ;
          delete groot ;
          groot = new Node() ;
        } // if


      } // else

      if ( !Istoken ) {
        gcolumn = 1 ;

        gline = 0 ;
      } // if


      //first = false ;

      Doterror = false ;

      //cout << "ss" << endl ;

      Istoken = false ;
      printdefine = false ;
      gpairs = false ;

    } // while

    cout << endl << "Thanks for using OurScheme!" ;

} // main()
