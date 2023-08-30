#include "specific.h"

#define MAXNUMTOKENS 5000
#define MAXTOKENSIZE 5000
#define NIL NULL
#define LISTSTRLEN 1000 
#define STARTPARENS "("
#define ENDPARENS ")"
#define COMMENT '#'
#define NUMALPH 26
#define DOUBLEQUOTE '"'
#define SINGLEQUOTE '\''
#define CLEAR ""
#define DOUBLE 2

/*Store readfile as a Program*/
struct prog {
    char wds[MAXNUMTOKENS][MAXTOKENSIZE]; // original input
    char str[MAXNUMTOKENS]; // Store input as String (exclude comment lines)
    int firstLine; // line of begin statement
    int lastLine; // line of end statement
};
typedef struct prog Program;

/*Variable structure*/
struct Var {
    bool isSet; // if variable has been defined
    lisp* list; // store lisp structure 
};
typedef struct Var Var;


/*take from previous assignment - LISP*/
#define atom(X)       lisp_atom(X)
#define cons(X, Y)    lisp_cons(X, Y)
#define car(L)        lisp_car(L)
#define cdr(L)        lisp_cdr(L)
#define copy(L)       lisp_copy(L)
#define fromstring(L) lisp_fromstring(L)


/*Read input Function*/
Program* READFILE(char* argv);


/*Parser Function*/
bool startProgram(Program* p);
bool parseInstrcts(char* instrcts);
bool parseInstrct(char* instrct);
bool parseSetFunc(char* instrct_token, char* rest);
bool parsePrintFunc(char* instrct_token, char* rest);
bool parseListFunc(char* instrct_token, char* rest);
bool parseList(char* rest);
bool parseTwoList(char* rest);
bool parseCondition(char* cpyInstrct);
bool parseConditionInstrcts(char* type, char* instrcts);
/*Parser Extension Function*/
bool parseResetFunc(char* instrct_token, char* rest);
bool parseSwapFunc(char* instrct_token, char* rest);

/*Helper Function*/
bool isCompare(char* str);
bool isLiteral(char* str);
bool isVariable(char* str);
bool isLisp(char* str);
bool isOperation(char* str);
bool isCondition(char* str);
bool isConstString(char* str);
bool isBrace(char ch);
bool strsame(char* str1,char* str2);

int countFrequency(char* str, char ch);
int countValidBrace(char* str);

void getFirstToken(char* firstToken, char* rest);
void unParens(char* str);
void removeSpaceAtSides(char* str);
void substring(char* buffer, int from, int end);
void separateBraces(char* str);
void sepFirstInstrct(char* instrct, char* rest);
void getFirstLiteral(char* str);

void tests();
void readMultipleFiles();
void testReadFile();

/*Interp Function*/
bool interpInstrcts(Var* varInfo, char* instrcts);
bool interpInstrct(Var* varInfo, char* instrct);
bool interpSetFunc(Var* varInfo, char* instrct_token, char* rest);
bool interpWhileFunc(Var* varInfo, char* instrct_token, char* rest);
bool interpIfFunc(Var* varInfo, char* instrct_token, char* rest);
bool interpList(Var* varInfo, char* token);
bool interpPrintFunc(Var* varInfo, char* instrct_token, char* rest);

lisp* interpListFunc(Var* varInfo, char* instrct);
lisp* interpCar(Var* varInfo, char* token, char* rest);
lisp* interpCdr(Var* varInfo, char* token, char* rest);
lisp* interpConsNCat(Var* varInfo, char* token, char* rest);
lisp* getList(Var* varInfo, char* token);
int interpLength(Var* varInfo, char* token, char* rest);
int interpOpComp(Var* varInfo, char* token, char* rest);
int doCompareOrOperation(char* token, int val1, int val2);

bool doIf(Var* varInfo, char* type, char* cond_instrct, char* stm1, char* stm2);
void doSet(Var* varInfo, char* origin, char* target);
/*Extension Interp Function*/
int interpABS(Var* varInfo, char* token, char* rest);
bool interpResetFunc(Var* varInfo, char* instrct_token, char* rest);
bool interpSwap(Var* varInfo, char* token, char* rest);
void doReset(Var* varInfo, char* target);
void doSwap(Var* varInfo, char* instrct, char var1, char var2);
int interpAtom(Var* varInfo, char* token, char* rest);

