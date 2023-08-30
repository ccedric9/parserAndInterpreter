#include "specific.h"

// Returns element 'a' - this is not a list, and
// by itelf would be printed as e.g. "3", and not "(3)"
lisp* lisp_atom(const atomtype a) {
    lisp* lp = ncalloc(1, sizeof(lisp));
    lp->car = NIL;
    lp->cdr = NIL;
    lp->val = a;
    return lp;
}

// Returns a list containing the car as 'l1'
// and the cdr as 'l2'- data in 'l1' and 'l2' are reused,
// and not copied. Either 'l1' and/or 'l2' can be NULL
lisp* lisp_cons(const lisp* l1, const lisp* l2) {
    if (!l1 && !l2) {
        return NIL;
    }
    lisp* lp = ncalloc(1, sizeof(lisp));
    if (l1) {
        lp->car = (lisp*)l1;
    }
    if (l2) {
        lp->cdr = (lisp*)l2;
    }
    return lp;

}

// Returns the car (1st) component of the list 'l'.
// Does not copy any data.
lisp* lisp_car(const lisp* l) {
    if (l) {
        return l->car;
    }
    return NIL;
}

// Returns the cdr (all but the 1st) component of the list 'l'.
// Does not copy any data.
lisp* lisp_cdr(const lisp* l) {
    if (l) {
        return l->cdr;
    }
    return NIL;
}

// Returns the data/value stored in the cons 'l'
atomtype lisp_getval(const lisp* l) {
    if (!l) {
        return 0;
    }
    return l->val;
}

// Returns a deep copy of the list 'l'
lisp* lisp_copy(const lisp* l) {
    if (!l) {
        return NIL;
    }
    lisp* copy = ncalloc(1, sizeof(lisp));
    copy->val = l->val;
    copy->car = lisp_copy(l->car);
    copy->cdr = lisp_copy(l->cdr);

    return copy;
}

// Returns number of components in the list.
int lisp_length(const lisp* l) {
    int len = 0;
    if (!l || lisp_isatomic(l)) {
        return len;
    }
    while (l) {
        len++;
        l = l->cdr;
    }
    return len;
}
bool lisp_isatomic(const lisp* l) {
    if (!l) {
        return false;
    }
    if (!l->car && !l->cdr) {
        return true;
    }
    return false;
}
// Returns stringified version of list
void lisp_tostring(const lisp* l, char* str) {
    if (!l) {
        strcpy(str, BKT);
        return;
    }
    char holder[LISTSTRLEN] = "";
    if (lisp_isatomic(l)) {
        int len = sprintf(holder, "%d", lisp_getval(l));
        assert(len);
        strcpy(str, holder);
        return;
    }
    strcpy(str, EMPTY);
    strcat(str, LBKT);
    recursionToStr(l, str);
}

void recursionToStr(const lisp* l, char* str) {
    if (!l) {
        return;
    }
    char holder[LISTSTRLEN] = "";
    if (l->car) {
        if (l->car->car) {
            strcat(str, LBKT);
        }
    }
    if (lisp_isatomic(l->car)) {
        int len = sprintf(holder, "%d", lisp_getval(l->car));
        assert(len);
        strcat(str, holder);
        if (!l->cdr) {
            strcat(str, RBKT);
        }
    }
    if (l->car) {
        recursionToStr(l->car, str);
    }
    if (l->cdr) {
        strcat(str, SPACE);
        recursionToStr(l->cdr, str);
    }
}
// Clears up all space used
// Double pointer allows function to set 'l' to NULL on success
void lisp_free(lisp** l) {
    if (!*l) {
        return;
    }
    lisp_free(&(*l)->car);
    lisp_free(&(*l)->cdr);
    free(*l);
    *l = NIL;
}


/* ------------- Tougher Ones : Extensions ---------------*/

// Builds a new list bas    lisp *con1 = lisp_cons(NIL,NIL);ed on the string 'str'
lisp* lisp_fromstring(const char* str) {
    char cpyStr[LISTSTRLEN] = "";
    strcpy(cpyStr, str);
    if (str[0] == '\'' && str[strlen(str) - 1] == '\'') {
        subString(cpyStr, 1, strlen(cpyStr) - 1);
    }
    if (strcmp(cpyStr, BKT) == 0) {
        return NIL;
    }
    if (strIsAtomic(cpyStr)) {
        return lisp_atom(atoi(cpyStr));
    }
    if (isSingleElement(cpyStr)) {
        lisp* res = ncalloc(1, sizeof(lisp));
        char buffer[LISTSTRLEN] = "";
        for (int i = 1; i < (int)strlen(cpyStr) - 1;i++) {
            strcat(buffer, &cpyStr[i]);
        }
        res->car = lisp_atom(atoi(buffer));
        return res;
    }
    lisp* res = lisp_atom(DEFAULT);
    //store all atoms as cdr from string
    lisp* nums = strToList(cpyStr);
    //start loop target string from index 1 - skip first left bracket 
    int idx = 1;
    recursionFromString(res, cpyStr, nums, &idx);
    lisp_free(&nums);
    return res;
}
void recursionFromString(lisp* res, char* str, lisp* nums, int* idx) {
    int len = (int)strlen(str);
    if ((*idx >= len)) {
        return;
    }
    while (*idx < len) {
        char cur = str[*idx];
        if (cur == '(') {
            (*idx)++;
            res->car = lisp_atom(DEFAULT);
            recursionFromString(res->car, str, nums, idx);
        }
        else if (cur == ' ') {
            (*idx)++;
            res->cdr = lisp_atom(DEFAULT);
            recursionFromString(res->cdr, str, nums, idx);
            res = res->cdr;
        }
        else if (isdigit(str[*idx]) || str[*idx] == NEGATIVE) {
            res->car = lisp_atom(lisp_getval(nums));
            nums = nums->cdr;
            (*idx)++;
            while (isdigit(str[*idx])) {
                (*idx)++;
            }
        }
        else if (cur == ')') {
            (*idx)++;
            return;
        }
        else {
            printf("errrr[%s]\n", str);
            on_error("Invalid character in target string");
        }
    }

}

bool isSingleElement(const char* str) {
    int n = (int)strlen(str);
    // consider string length less than 1 
    if (n <= 1) {
        return false;
    }
    if (str[0] != '(' && str[n - 1] != ')') {
        return false;
    }
    if (!isdigit(str[1])) {
        if (str[1] != NEGATIVE) {
            return false;
        }
    }
    // consider string length less than 1 
    for (int i = SECONDCHAR; i < n - 1; i++) {
        if (!isdigit(str[i])) {
            return false;
        }
    }
    return true;
}

bool strIsAtomic(const char* str) {
    int len = strlen(str);
    if (len == 0) {
        return false;
    }
    if (!isdigit(str[0])) {
        if (str[0] != NEGATIVE) {
            return false;
        }
    }
    for (int i = 1; i < len; i++) {
        if (ispunct(str[i])) {
            return false;
        }
    }
    return true;
}

lisp* strToList(const char* str) {
    if (strcmp(str, BKT) == 0) {
        return NIL;
    }
    lisp* res = ncalloc(1, sizeof(lisp)), * tmp = res;
    char buffer[LISTSTRLEN] = EMPTY;
    for (int i = 1; i < (int)strlen(str); i++) {
        if (isdigit(str[i]) || str[i] == NEGATIVE) {
            strcat(buffer, &str[i]);
        }
        if (isspace(str[i])) {
            tmp->cdr = lisp_atom(atoi(buffer));
            strcpy(buffer, EMPTY);
            tmp = tmp->cdr;
        }
        tmp->cdr = lisp_atom(atoi(buffer));
    }
    return res->cdr;
}

// Returns a new list from a set of existing lists.
// A variable number 'n' lists are used.
// Data in existing lists are reused, and not copied.
// You need to understand 'varargs' for this.
lisp* lisp_list(const int n, ...) {

    va_list args;
    va_start(args, n);
    lisp* res = ncalloc(1, sizeof(lisp));
    lisp* tmp = res;
    for (int i = 0; i < n;i++) {
        tmp->car = va_arg(args, lisp*);
        tmp->cdr = ncalloc(1, sizeof(lisp));
        if (i < n - 1) {
            tmp = tmp->cdr;
        }
        else {
            tmp->cdr = NIL;
        }
    }
    va_end(args);
    return res;
}

// Allow a user defined function 'func' to be applied to
// each component of the list 'l'.
// The user-defined 'func' is passed a pointer to a cons,
// and will maintain an accumulator of the result.
void lisp_reduce(void (*func)(lisp* l, atomtype* n), lisp* l, atomtype* acc) {
    if (!l) {
        return;
    }
    if (lisp_isatomic(l)) {
        (*func)(l, acc);
    }
    lisp_reduce(func, l->car, acc);
    lisp_reduce(func, l->cdr, acc);

}

void subString(char* buffer, int from, int end) {
    int len = (int)strlen(buffer);
    if (from > end || len == 0) {
        return;
    }
    char tmp[LISTSTRLEN] = "";
    int start = 0;
    for (int i = from; i < end;i++) {
        char cur = buffer[i];
        tmp[start++] = cur;
    }
    strcpy(buffer, tmp);
}


void test() {

    char testStr[LISTSTRLEN] = "";
    /*check NIL case*/
    assert(!lisp_getval(NIL));
    assert(lisp_length(NIL) == 0);
    assert(!lisp_car(NIL));
    assert(!lisp_cdr(NIL));
    // no need to free as no space creation for NIL
    assert(!lisp_cons(NIL, NIL));

    /*check atomic element*/
    lisp* test1 = lisp_atom(10);
    assert(test1);
    assert(test1->val == 10);
    assert(lisp_getval(test1) == 10);
    assert(lisp_length(test1) == 0);
    assert(!lisp_car(test1));
    assert(!lisp_cdr(test1));
    assert(lisp_isatomic(test1));
    lisp_tostring(test1, testStr);
    assert(strcmp(testStr, "10") == 0);
    lisp_free(&test1);
    assert(!test1);

    lisp* test2 = lisp_atom(9);
    lisp* test3 = lisp_atom(5);
    lisp* con2 = lisp_cons(test2, NIL);
    lisp* con3 = lisp_cons(NIL, test3);
    assert(lisp_getval(con2->car) == 9);
    assert(lisp_getval(con3->cdr) == 5);
    assert(lisp_isatomic(con2) == false);
    assert(lisp_isatomic(con3) == false);

    lisp_free(&con2);
    lisp_free(&con3);
    assert(!con2);
    assert(!con3);

    //con1 is (9 5)
    lisp* con1 = lisp_cons(lisp_atom(9), lisp_atom(5));
    assert(lisp_length(con1) == 2);
    assert(lisp_getval(con1->car) == 9);
    assert(lisp_getval(con1->cdr) == 5);
    assert(lisp_isatomic(con1) == false);

    // /*check copy function*/
    lisp* test4 = lisp_copy(con1);
    assert(lisp_length(test4) == 2);
    assert(lisp_getval(test4->car) == 9);
    assert(lisp_getval(test4->cdr) == 5);
    assert(lisp_isatomic(test4) == false);

    lisp_free(&con1);
    assert(!con1);
    lisp_free(&test4);
    assert(!test4);

    // ((2 4) 9)
    lisp* test5 = lisp_cons(lisp_cons(lisp_atom(2), lisp_atom(4)), lisp_atom(9));
    assert(lisp_length(test5) == 2);
    assert(lisp_getval(test5->car->car) == 2);
    assert(lisp_getval(test5->car->cdr) == 4);
    assert(lisp_getval(test5->cdr) == 9);
    lisp* test6 = lisp_copy(test5);
    assert(lisp_length(test6) == 2);
    assert(lisp_getval(test6->car->car) == 2);
    assert(lisp_getval(test6->car->cdr) == 4);
    assert(lisp_getval(test6->cdr) == 9);

    lisp_free(&test6);
    assert(!test6);
    lisp_free(&test5);
    assert(!test5);

    lisp* from1 = lisp_fromstring("()");
    lisp_tostring(from1, testStr);
    assert(strcmp(testStr, "()") == 0);

    lisp* from2 = lisp_fromstring("-12");
    lisp_tostring(from2, testStr);
    assert(strcmp(testStr, "-12") == 0);
    lisp_free(&from2);
    assert(!from2);


    lisp* from3 = lisp_fromstring("(-11)");
    lisp_tostring(from3, testStr);
    assert(strcmp(testStr, "(-11)") == 0);
    lisp_free(&from3);
    assert(!from3);
}



