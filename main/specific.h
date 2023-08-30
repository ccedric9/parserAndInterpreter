#include <ctype.h>
#include "lisp.h"

#define NIL NULL
#define SPACE " "
#define LBKT "("
#define RBKT ")"
#define BKT "()"
#define EMPTY ""
#define LISTSTRLEN 1000
#define LISPIMPL "Linked"
#define NEGATIVE '-'
#define DEFAULT 0
#define SECONDCHAR 2

struct lisp
{
    struct lisp* car;
    struct lisp* cdr;
    atomtype val;
};
bool lisp_isatomic(const lisp* l);
void recursionToStr(const lisp* l, char* str);
bool strIsAtomic(const char* str);
lisp* strToList(const char* str);
void recursionFromString(lisp* res, char* str, lisp* nums, int* idx);
bool isSingleElement(const char* str);
void subString(char* buffer, int from, int end);




