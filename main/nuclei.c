#include "nuclei.h"

int main(int argc, char* argv[]) {
    Program* p;
    p = READFILE(*&argv[argc - 1]);
    startProgram(p);
    free(p);
    tests();
    return 0;
}

Program* READFILE(char* argv) {
    FILE* fp;
    if (!(fp = fopen(argv, "r"))) {
        fprintf(stderr, "Cannot Read %s\n", argv);
        exit(EXIT_FAILURE);
    }
    Program* p = ncalloc(1, sizeof(Program));
    int start = 0;
    strcpy(p->str, "");
    char buffer[MAXNUMTOKENS] = "";
    bool getFirstLine = false;
    while ((fscanf(fp, "%[^\n]\n", buffer)) != EOF) {
        separateBraces(buffer);
        strcpy(p->wds[start], buffer);
        if (buffer[0] != COMMENT) {
            if (!strsame(p->str, "")) {
                strcat(p->str, SPACE);
            }
            strcat(p->str, buffer);
            if (!getFirstLine) {
                p->firstLine = start;
                getFirstLine = true;
            }
        }
        start++;
    }
    //check the last line of statement, return 0 if it's empty
    p->lastLine = start > 0 ? (start - 1) : 0;
    fclose(fp);
    return p;
}

bool startProgram(Program* p) {
    if (!strsame(p->wds[p->firstLine], STARTPARENS)) {
        on_error("Wrong start statement ; ");
    }
    if (!strsame(p->wds[p->lastLine], ENDPARENS)) {
        on_error("Wrong end statement ;  ");
    }
    char instrcts[MAXNUMTOKENS] = "";
    strcpy(instrcts, p->str);
    unParens(instrcts), removeSpaceAtSides(instrcts);
    if (instrcts[0] != '(') {
        on_error("Wrong instrcts begin statement ; ");
    }
#ifdef INTERP
    Var varInfo[NUMALPH] = { {false,NIL} };
    if (interpInstrcts(varInfo, instrcts)) {
        for (int i = 0; i < NUMALPH;i++) {
            varInfo[i].list = NIL;
            varInfo[i].isSet = false;
        }
        return true;
    }
#else 
    if (parseInstrcts(instrcts)) {
        printf("Parse OK\n");
        return true;
    }
#endif
    return false;
}

#ifdef INTERP
bool interpInstrcts(Var* varInfo, char* instrcts) {
    if (!instrcts) {
        on_error("Empty Instrcts Statement");
    }
    char instrct[MAXNUMTOKENS] = "", rest[MAXNUMTOKENS] = "";
    strcpy(instrct, instrcts), strcpy(rest, instrcts);
    sepFirstInstrct(instrct, rest);
    if (!interpInstrct(varInfo, instrct)) {
        return false;
    }
    if (strsame(rest, "")) {
        return true;
    }
    return interpInstrcts(varInfo, rest);
}

bool interpInstrct(Var* varInfo, char* instrct) {
    unParens(instrct), removeSpaceAtSides(instrct);
    char instrct_token[MAXNUMTOKENS] = "", rest[MAXNUMTOKENS] = "";
    strcpy(instrct_token, instrct), strcpy(rest, instrct);
    getFirstToken(instrct_token, rest);
    if (strsame(instrct_token, "SET")) {
        return interpSetFunc(varInfo, instrct_token, rest);
    }
    else if (strsame(instrct_token, "RESET")) {
        return interpResetFunc(varInfo, instrct_token, rest);
    }
    else if (strsame(instrct_token, "PRINT")) {
        return interpPrintFunc(varInfo, instrct_token, rest);
    }
    else if (strsame(instrct_token, "WHILE")) {
        return interpWhileFunc(varInfo, instrct_token, rest);
    }
    else if (strsame(instrct_token, "IF")) {
        return interpIfFunc(varInfo, instrct_token, rest);
    }
    else if (strsame(instrct_token, "SWAP")) {
        return interpSwap(varInfo, instrct_token, rest);
    }
    return false;
}

bool interpSetFunc(Var* varInfo, char* instrct_token, char* rest) {
    if (!strsame(instrct_token, "SET")) {
        on_error("Not a set function");
    }
    char token[MAXNUMTOKENS] = "";
    strcpy(token, rest);
    getFirstToken(token, rest);
    if (!isVariable(token)) {
        on_error("Interp : Wrong set variable");
    }
    if (interpList(varInfo, rest)) {
        doSet(varInfo, token, rest);
        return true;
    }
    else {
        on_error("Not a valid set target");
    }
    return false;
}

bool interpResetFunc(Var* varInfo, char* instrct_token, char* rest) {
    if (!strsame(instrct_token, "RESET")) {
        on_error("Not a reset function");
    }
    if (isVariable(rest)) {
        doReset(varInfo, rest);
        return true;
    }
    return false;

}

bool interpWhileFunc(Var* varInfo, char* instrct_token, char* rest) {
    if (!strsame(instrct_token, "WHILE")) {
        on_error("Not a while function");
    }
    /*Get conditional statement*/
    char cond_instrct[MAXNUMTOKENS] = "", statement[MAXNUMTOKENS] = "";
    strcpy(cond_instrct, rest), strcpy(statement, rest);
    sepFirstInstrct(cond_instrct, statement);
    unParens(cond_instrct), removeSpaceAtSides(cond_instrct);
    unParens(statement), removeSpaceAtSides(statement);
    char token[MAXNUMTOKENS] = "", rest_token[MAXNUMTOKENS] = "";
    strcpy(token, cond_instrct), strcpy(rest_token, cond_instrct);
    getFirstToken(token, rest_token);
    /*Check boolean function if do while */
    int checkCondition = interpOpComp(varInfo, token, rest_token);
    if (checkCondition > 0) {
        while (checkCondition > 0) {
            interpInstrcts(varInfo, statement);
            checkCondition = interpOpComp(varInfo, token, rest_token);
        }
        return true;
    }
    else {
        on_error("Not a valid while condtion");
    }
    return false;
}

bool interpIfFunc(Var* varInfo, char* instrct_token, char* rest) {
    if (!strsame(instrct_token, "IF")) {
        on_error("Not a if function");
    }
    /*Get conditional statement*/
    char cond_instrct[MAXNUMTOKENS] = "", statement[MAXNUMTOKENS] = "";
    strcpy(cond_instrct, rest), strcpy(statement, rest);
    sepFirstInstrct(cond_instrct, statement);
    unParens(cond_instrct), removeSpaceAtSides(cond_instrct);
    char token[MAXNUMTOKENS] = "", rest_token[MAXNUMTOKENS] = "";
    strcpy(token, cond_instrct), strcpy(rest_token, cond_instrct);
    getFirstToken(token, rest_token);
    /*Get if and else statement by store in stm1 & stm2 &*/
    char stm1[MAXNUMTOKENS] = "", stm2[MAXNUMTOKENS] = "";
    strcpy(stm1, statement), strcpy(stm2, statement);
    sepFirstInstrct(stm1, stm2);
    return doIf(varInfo, instrct_token, cond_instrct, stm1, stm2);
}

bool doIf(Var* varInfo, char* type, char* cond_instrct, char* stm1, char* stm2) {
    if (!strsame(type, "IF")) {
        on_error("Not an if statement");
    }
    unParens(stm1), removeSpaceAtSides(stm1);
    unParens(stm2), removeSpaceAtSides(stm2);
    char firstToken[MAXNUMTOKENS] = "", rest[MAXNUMTOKENS] = "";
    strcpy(firstToken, cond_instrct), strcpy(rest, cond_instrct);
    getFirstToken(firstToken, rest);
    int checkCondition = 0;
    if (strsame(firstToken, "ATOM")) {
        checkCondition = interpAtom(varInfo, firstToken, rest);
    }
    else {
        checkCondition = interpOpComp(varInfo, firstToken, rest);
    }
    if (checkCondition > 0) {
        return interpInstrcts(varInfo, stm1);
    }
    else {
        return interpInstrcts(varInfo, stm2);
    }
}

bool interpList(Var* varInfo, char* token) {
    if (strsame(token, "NIL")) {
        return true;
    }
    else if (isLiteral(token)) {
        char cpyToken[MAXNUMTOKENS] = "";
        strcpy(cpyToken, token);
        substring(cpyToken, 1, (int)strlen(token) - 1);
        if (countValidBrace(cpyToken) == 0) {
            return true;
        }
        else {
            fprintf(stderr, "Invalid lisp string:'%s'\n", cpyToken);
            exit(EXIT_FAILURE);
        }
    }
    /*check if variable has been defined*/
    else if (isVariable(token)) {
        char var = token[0];
        if (varInfo[var - 'A'].isSet) {
            return true;
        }
        else {
            on_error("Interp: Variable has not been defined");
        }
    }
    else if (token[0] == '(' && countValidBrace(token) == 0) {
        return true;
    }
    return false;
}

bool interpPrintFunc(Var* varInfo, char* instrct_token, char* rest) {
    char buffer[MAXNUMTOKENS] = "";
    if (!strsame(instrct_token, "PRINT")) {
        on_error("Not a print function");
    }
    if (isVariable(rest)) {
        if (varInfo[rest[0] - 'A'].isSet) {
            lisp_tostring(varInfo[rest[0] - 'A'].list, buffer);
            printf("%s\n", buffer);
            return true;
        }
        else {
            on_error("Variable has not been defined");
        }
    }
    else if (isConstString(rest)) {
        printf("%s\n", rest);
        return true;
    }
    else if (rest[0] == '(' && countValidBrace(rest) == 0) {
        lisp* target = interpListFunc(varInfo, rest);
        lisp_tostring(target, buffer);
        printf("%s\n", buffer);
        return true;
    }
    else {
        fprintf(stderr, "invalid %s\n", rest);
        exit(EXIT_FAILURE);
    }
    return false;
}

lisp* interpListFunc(Var* varInfo, char* instrct) {
    char cpyInstrct[MAXNUMTOKENS] = "";
    strcpy(cpyInstrct, instrct);
    unParens(cpyInstrct), removeSpaceAtSides(cpyInstrct);
    char token[MAXNUMTOKENS] = "", rest[MAXNUMTOKENS] = "";
    strcpy(token, cpyInstrct), strcpy(rest, cpyInstrct);
    getFirstToken(token, rest);
    if (strsame(token, "CAR")) {
        return interpCar(varInfo, token, rest);
    }
    else if (strsame(token, "CDR")) {
        return interpCdr(varInfo, token, rest);
    }
    else if (strsame(token, "CONS") || strsame(token, "CAT")) {
        return interpConsNCat(varInfo, token, rest);
    }
    else if (isOperation(token) || isCompare(token)) {
        return lisp_atom(interpOpComp(varInfo, token, rest));
    }
    else if (strsame(token, "LENGTH")) {
        return lisp_atom(interpLength(varInfo, token, rest));
    }
    else if (strsame(token, "ABS")) {
        return lisp_atom(interpABS(varInfo, token, rest));
    }
    return NIL;
}

int interpABS(Var* varInfo, char* token, char* rest) {
    if (!strsame(token, "ABS") || strsame(rest, "NIL")) {
        return 0;
    }
    int val = 0;
    if (isVariable(rest)) {
        int index = rest[0] - 'A';
        val = lisp_getval(varInfo[index].list);
    }
    else if (isLiteral(rest)) {
        val = lisp_getval(lisp_fromstring(rest));
    }
    else if (rest[0] == '(' && countValidBrace(rest) == 0) {
        val = lisp_getval(interpListFunc(varInfo, rest));
    }
    if (val < 0) {
        val = 0 - val;
    }
    return val;
}

bool interpSwap(Var* varInfo, char* token, char* rest) {
    if (!strsame(token, "SWAP")) {
        on_error("Not a swap function");
    }
    char var1[MAXNUMTOKENS] = "", var2[MAXNUMTOKENS] = "";
    strcpy(var1, rest), strcpy(var2, rest);
    getFirstToken(var1, var2);
    if (isVariable(var1) && isVariable(var2)) {
        char v1 = var1[0], v2 = var2[0];
        doSwap(varInfo, token, v1, v2);
        return true;
    }
    return false;
}

lisp* interpCar(Var* varInfo, char* token, char* rest) {
    if (!strsame(token, "CAR") || rest == NULL) {
        on_error("Not a car function");
    }
    if (isVariable(rest)) {
        int index = rest[0] - 'A';
        return lisp_car(varInfo[index].list);
    }
    else if (strsame(rest, "NIL")) {
        return NIL;
    }
    else if (isLiteral(rest) && countValidBrace(rest) == 0) {
        char cpyRest[MAXNUMTOKENS] = "";
        strcpy(cpyRest, rest);
        int len = strlen(cpyRest);
        substring(cpyRest, 1, len - 1);
        return lisp_car(lisp_fromstring(cpyRest));
    }
    else {
        return lisp_car(interpListFunc(varInfo, rest));
    }
}

lisp* interpCdr(Var* varInfo, char* token, char* rest) {
    if (!strsame(token, "CDR") || rest == NULL) {
        on_error("Not a cdr function");
    }
    if (isVariable(rest)) {
        int index = rest[0] - 'A';
        return lisp_cdr(varInfo[index].list);
    }
    else if (strsame(rest, "NIL")) {
        return NIL;
    }
    else if (isLiteral(rest) && countValidBrace(rest) == 0) {
        char cpyRest[MAXNUMTOKENS] = "";
        strcpy(cpyRest, rest);
        int len = strlen(cpyRest);
        substring(cpyRest, 1, len - 1);
        return lisp_cdr(lisp_fromstring(cpyRest));
    }
    else {
        return lisp_cdr(interpListFunc(varInfo, rest));
    }
}

lisp* interpConsNCat(Var* varInfo, char* token, char* rest) {
    if (!strsame(token, "CONS") && !strsame(token, "CAT")) {
        on_error("Not a cons function");
    }
    char l1[MAXNUMTOKENS] = "", l2[MAXNUMTOKENS] = "";
    strcpy(l1, rest), strcpy(l2, rest);
    lisp* res = lisp_atom(DEFAULT);
    if (rest[0] == '(') {
        sepFirstInstrct(l1, l2);
    }
    else if (rest[0] == '\'') {
        getFirstLiteral(l1);
        int l1_len = strlen(l1), len = strlen(rest);
        substring(l2, l1_len, len);
        removeSpaceAtSides(l2);
    }
    else {
        getFirstToken(l1, l2);
        if (!isVariable(l1) && !strsame(l1, "NIL")) {
            on_error("Wrong Cons statement");
        }
    }
    if (strsame(token, "CONS")) {
        res->car = getList(varInfo, l1);
    }
    else if (strsame(token, "CAT")) {
        res = getList(varInfo, l1);
    }
    res->cdr = getList(varInfo, l2);
    return res;
}

int interpAtom(Var* varInfo, char* token, char* rest) {
    if (!strsame(token, "ATOM")) {
        on_error("Not an check atomic function");
    }
    if (lisp_isatomic(getList(varInfo, rest))) {
        return 1;
    }
    else {
        return 0;
    }
}

int interpLength(Var* varInfo, char* token, char* rest) {
    if (!strsame(token, "LENGTH") || rest == NULL) {
        on_error("Not a get length function");
    }
    int result = 0;
    if (isVariable(rest)) {
        int index = rest[0] - 'A';
        result = lisp_length(varInfo[index].list);
    }
    else if (strsame(rest, "NIL")) {
        result = 0;
    }
    else if (isLiteral(rest) && countValidBrace(rest) == 0) {
        char cpyRest[MAXNUMTOKENS] = "";
        strcpy(cpyRest, rest);
        int len = strlen(cpyRest);
        substring(cpyRest, 1, len - 1);
        result = lisp_length(lisp_fromstring(cpyRest));
    }
    else {
        result = lisp_length(interpListFunc(varInfo, rest));
    }
    return result;
}

int interpOpComp(Var* varInfo, char* token, char* rest) {
    if (!isOperation(token) && !isCompare(token)) {
        on_error("Not a operation/compare function");
    }
    char l1[MAXNUMTOKENS] = "", l2[MAXNUMTOKENS] = "";
    strcpy(l1, rest), strcpy(l2, rest);
    int val1 = 0, val2 = 0;
    if (rest[0] == '(') {
        sepFirstInstrct(l1, l2);
    }
    else if (rest[0] == '\'') {
        getFirstLiteral(l1);
        int l1_len = strlen(l1), len = strlen(rest);
        substring(l2, l1_len, len);
        removeSpaceAtSides(l2);
    }
    else {
        getFirstToken(l1, l2);
        if (!strsame(l1, "NIL") && !isVariable(l1)) {
            on_error("Wrong compare / operation target");
        }
    }
    val1 = lisp_getval(getList(varInfo, l1));
    val2 = lisp_getval(getList(varInfo, l2));
    return doCompareOrOperation(token, val1, val2);
}

int doCompareOrOperation(char* token, int val1, int val2) {
    int result = 0;
    if (strsame(token, "LESS")) {
        result = val1 < val2 ? 1 : 0;
    }
    else if (strsame(token, "GREATER")) {
        result = val1 > val2 ? 1 : 0;
    }
    else if (strsame(token, "EQUAL")) {
        result = val1 == val2 ? 1 : 0;
    }
    else if (strsame(token, "PLUS")) {
        result = val1 + val2;
    }
    else if (strsame(token, "MINUS")) {
        result = val1 - val2;
    }
    else if (strsame(token, "MUL")) {
        result = val1 * val2;
    }
    else if (strsame(token, "DIV") && val2 != 0) {
        result = val1 / val2;
    }
    else if (strsame(token, "MOD") && val2 != 0) {
        result = val1 % val2;
    }
    else {
        on_error("Not a operation or compare function");
    }
    return result;
}

lisp* getList(Var* varInfo, char* token) {
    if (strsame(token, "NIL")) {
        return NIL;
    }
    else if (isVariable(token)) {
        int index = token[0] - 'A';
        if (varInfo[index].isSet) {
            return varInfo[index].list;
        }
        else {
            on_error("This variable is not defined");
        }
    }
    else if (isLiteral(token) && countFrequency(token, '\'') % 2 == 0) {
        int len = strlen(token);
        char cpyToken[MAXNUMTOKENS] = "";
        strcpy(cpyToken, token);
        substring(cpyToken, 1, len - 1);
        return lisp_fromstring(cpyToken);
    }
    else if ((token[0] == '(' && countValidBrace(token) == 0)) {
        return interpListFunc(varInfo, token);
    }
    else {
        on_error("Unable to interp this token");
    }
    return NIL;
}

void doSet(Var* varInfo, char* origin, char* target) {
    if (!isVariable(origin)) {
        on_error("Cannot set a non-variable");
    }
    if (!interpList(varInfo, target)) {
        on_error("Not a valid set target");
    }
    int index = origin[0] - 'A';
    varInfo[index].isSet = true;
    /*lisp from string / NIL / Variable */
    if (isVariable(target)) {
        int target_idx = target[0] - 'A';
        varInfo[index] = varInfo[target_idx];
    }
    else if (strsame(target, "NIL")) {
        varInfo[index].list = NIL;
    }
    else if (isLiteral(target)) {
        int len = strlen(target);
        substring(target, 1, len - 1);
        if (countValidBrace(target) != 0) {
            on_error("Wrong lisp literal");
        }
        varInfo[index].list = lisp_fromstring(target);
    }
    else if (target[0] == '(' && countValidBrace(target) == 0) {
        varInfo[index].list = interpListFunc(varInfo, target);
    }
}

void doReset(Var* varInfo, char* target) {
    if (!isVariable(target)) {
        return;
    }
    int index = target[0] - 'A';
    varInfo[index].isSet = false;
    varInfo[index].list = NIL;
}

void doSwap(Var* varInfo, char* instrct, char var1, char var2) {
    if (!strsame(instrct, "SWAP")) {
        on_error("Not a swap function");
    }
    int idx1 = var1 - 'A', idx2 = var2 - 'A';
    bool b_tmp = varInfo[idx1].isSet;
    lisp* tmp = varInfo[idx1].list;
    varInfo[idx1].isSet = varInfo[idx2].isSet;
    varInfo[idx1].list = varInfo[idx2].list;
    varInfo[idx2].isSet = b_tmp;
    varInfo[idx2].list = tmp;
}

#else
bool parseInstrcts(char* instrcts) {
    if (!instrcts) {
        on_error("Empty Instrcts Statement");
    }
    char instrct[MAXNUMTOKENS] = "", rest[MAXNUMTOKENS] = "";
    strcpy(instrct, instrcts);
    sepFirstInstrct(instrct, rest);
    if (!parseInstrct(instrct)) {
        return false;
    }
    if (strsame(rest, "")) {
        return true;
    }
    return parseInstrcts(rest);
}

bool parseInstrct(char* instrct) {
    if (instrct[0] != '(' && instrct[(int)strlen(instrct) - 1] != ')') {
        on_error("Not a valid instrct statement");
    }
    unParens(instrct), removeSpaceAtSides(instrct);
    char firstToken[MAXNUMTOKENS] = "", rest[MAXNUMTOKENS] = "";
    strcpy(firstToken, instrct), strcpy(rest, instrct);
    getFirstToken(firstToken, rest);
    if (strsame(firstToken, "SET")) {
        return parseSetFunc(firstToken, rest);
    }
    else if (strsame(firstToken, "PRINT")) {
        return parsePrintFunc(firstToken, rest);
    }
    else if (isCondition(firstToken)) {
        return parseConditionInstrcts(firstToken, rest);
    }
    else if (strsame(firstToken, "RESET")) {
        return parseResetFunc(firstToken, rest);
    }
    else if (strsame(firstToken, "SWAP")) {
        return parseSwapFunc(firstToken, rest);
    }
    else {
        printf("Wrong instrct statement :%s\n", instrct);
    }
    return false;
}

bool parseSetFunc(char* instrct_token, char* rest) {
    if (!strsame(instrct_token, "SET")) {
        on_error("Not a SET function");
    }
    char nextToken[MAXNUMTOKENS] = "";
    getFirstToken(nextToken, rest);
    /*check if has variable*/
    if (!isVariable(nextToken)) {
        on_error("Require a variable for SET function");
    }
    if (parseList(rest)) {
        return true;
    }
    else {
        on_error("Wrong SET Function");
    }
    return false;
}

bool parsePrintFunc(char* instrct_token, char* rest) {
    /*Print var / const string / a list*/
    if (!strsame(instrct_token, "PRINT")) {
        on_error("Not a Print Function");
    }
    /*Include var/list*/
    if (parseList(rest)) {
        return true;
    }
    if (isConstString(rest)) {
        return true;
    }
    else {
        fprintf(stderr, "Wrong Print target:%s\n", rest);
        exit(EXIT_FAILURE);
    }
    return false;
}

bool parseListFunc(char* instrct_token, char* rest) {
    if (strsame(instrct_token, "CAR") || strsame(instrct_token, "CDR")) {
        return parseList(rest);
    }
    else if (strsame(instrct_token, "LENGTH") || strsame(instrct_token, "ABS")) {
        return parseList(rest);
    }
    else if (isOperation(instrct_token) || isCompare(instrct_token)) {
        if (parseTwoList(rest)) {
            return true;
        }
    }
    else if (strsame(instrct_token, "CONS") || strsame(instrct_token, "CAT")) {
        if (parseTwoList(rest)) {
            return true;
        }
    }
    else {
        on_error("Wrong LISTFUNC statement");
    }
    return false;
}

bool parseConditionInstrcts(char* type, char* instrcts) {
    if (!isCondition(type)) {
        on_error("Wrong if/while statement");
    }
    /*First Statement must be a condtional statement*/
    char cond_instrct[MAXNUMTOKENS] = "", rest[MAXNUMTOKENS] = "";
    strcpy(cond_instrct, instrcts), strcpy(rest, instrcts);
    sepFirstInstrct(cond_instrct, rest);
    if (parseCondition(cond_instrct)) {
        char instrcts1[MAXNUMTOKENS] = "", instrcts2[MAXNUMTOKENS] = "";
        strcpy(instrcts1, rest), strcpy(instrcts2, rest);
        if (strsame(type, "IF")) {
            sepFirstInstrct(instrcts1, instrcts2);
            unParens(instrcts1), removeSpaceAtSides(instrcts1);
            unParens(instrcts2), removeSpaceAtSides(instrcts2);
            return parseInstrcts(instrcts1) && parseInstrcts(instrcts2);
        }
        else if (strsame(type, "WHILE")) {
            unParens(instrcts1), removeSpaceAtSides(instrcts1);
            return parseInstrcts(instrcts1);
        }
    }
    else {
        on_error("Wrong Condition statement");
    }
    return false;
}

bool parseCondition(char* cpyInstrct) {
    char instrct[MAXNUMTOKENS] = "";
    strcpy(instrct, cpyInstrct);
    unParens(instrct), removeSpaceAtSides(instrct);
    char firstToken[MAXNUMTOKENS] = "", rest[MAXNUMTOKENS] = "";
    strcpy(firstToken, instrct), strcpy(rest, instrct);
    getFirstToken(firstToken, rest);
    if (strsame(firstToken, "ATOM")) {
        return parseList(rest);
    }
    if (!isCompare(firstToken)) {
        on_error("Wrong condtional statement");
    }
    return parseTwoList(rest);
}

bool parseList(char* rest) {
    if (rest[0] == '(' && rest[(int)strlen(rest) - 1] == ')') {
        unParens(rest), removeSpaceAtSides(rest);
        char list[MAXNUMTOKENS] = "", instrct_token[MAXNUMTOKENS] = "";
        strcpy(list, rest), strcpy(instrct_token, rest);
        getFirstToken(instrct_token, list);
        if (isLisp(instrct_token) || isOperation(instrct_token) || isCompare(instrct_token)) {
            return parseListFunc(instrct_token, list);
        }
        if (strsame(instrct_token, "CAT") || strsame(instrct_token, "LENGTH") || strsame(instrct_token, "ABS")) {
            return parseListFunc(instrct_token, list);
        }
    }
    else if (strsame(rest, "NIL") || isLiteral(rest) || isVariable(rest)) {
        return true;
    }
    return false;
}

bool parseTwoList(char* rest) {
    char list1[MAXNUMTOKENS] = "", list2[MAXNUMTOKENS] = "";
    strcpy(list1, rest), strcpy(list2, rest);
    getFirstToken(list1, list2);
    if (isVariable(list1) || strsame(list1, "NIL")) {
        return parseList(list2);
    }
    else if (rest[0] == '\'' || rest[0] == '(') {
        char l1[MAXNUMTOKENS] = "", l2[MAXNUMTOKENS] = "";
        strcpy(l1, rest), strcpy(l2, rest);
        if (rest[0] == '\'') {
            getFirstLiteral(l1);
            int len = strlen(rest);
            int l1_len = strlen(l1);
            substring(l2, l1_len, len);
            removeSpaceAtSides(l2);
        }
        else {
            sepFirstInstrct(l1, l2);
        }
        return parseList(l1) && parseList(l2);
    }
    return false;
}

bool parseResetFunc(char* instrct_token, char* rest) {
    if (!strsame(instrct_token, "RESET")) {
        on_error("NOT a reset function");
    }
    if (!isVariable(rest)) {
        return false;
    }
    return true;
}

bool parseSwapFunc(char* instrct_token, char* rest) {
    if (!strsame(instrct_token, "SWAP")) {
        on_error("NOT a SWAP function");
    }
    char first[MAXNUMTOKENS] = "", second[MAXNUMTOKENS] = "";
    strcpy(first, rest), strcpy(second, rest);
    getFirstToken(first, second);
    if (isVariable(first) && isVariable(second)) {
        return true;
    }
    return false;
}

#endif

void sepFirstInstrct(char* instrct, char* rest) {
    if (instrct == NULL || rest == NULL) {
        return;
    }
    removeSpaceAtSides(instrct);
    if (instrct[0] != '(') {
        return;
    }
    strcpy(rest, instrct);
    int len = strlen(instrct);
    for (int i = 0; i < len; i++) {
        if (instrct[i] == ')') {
            substring(instrct, 0, i + 1);
            if (countValidBrace(instrct) == 0) {
                int b_len = strlen(instrct);
                substring(rest, b_len, len);
                removeSpaceAtSides(rest);
                i = len;
            }
            else {
                strcpy(instrct, rest);
            }
        }
    }
}

bool isConstString(char* str) {
    int len = strlen(str);
    if (len == 0) {
        return false;
    }
    if (str[0] == DOUBLEQUOTE && str[len - 1] == DOUBLEQUOTE) {
        return countFrequency(str, '"') == DOUBLE;
    }
    return false;
}

void separateBraces(char* str) {
    char tmp[MAXNUMTOKENS] = "";
    int len = strlen(str);
    for (int i = 0; i < len; i++) {
        strncat(tmp, &str[i], 1);
        if (i < len - 1 && isBrace(str[i]) && isBrace(str[i + 1])) {
            strcat(tmp, SPACE);
        }
    }
    strcpy(str, tmp);
}

bool isBrace(char ch) {
    if (ch == '(' || ch == ')') {
        return true;
    }
    return false;
}

bool isCondition(char* str) {
    if (strsame(str, "IF") || strsame(str, "WHILE")) {
        return true;
    }
    return false;
}

bool isCompare(char* str) {
    if (strsame(str, "LESS") || strsame(str, "GREATER") || strsame(str, "EQUAL")) {
        return true;
    }
    return false;
}

bool isOperation(char* str) {
    if (strsame(str, "PLUS") || strsame(str, "MINUS")) {
        return true;
    }
    else if (strsame(str, "MUL") || strsame(str, "MOD") || strsame(str, "DIV")) {
        return true;
    }
    return false;
}

bool isLiteral(char* str) {
    int len = strlen(str);
    if (str[0] == SINGLEQUOTE && str[len - 1] == SINGLEQUOTE) {
        return true;
    }
    return false;
}

int countFrequency(char* str, char ch) {
    int len = strlen(str);
    int count = 0;
    for (int i = 0; i < len; i++) {
        if (str[i] == ch) {
            count++;
        }
    }
    return count;
}

int countValidBrace(char* str) {
    int len = strlen(str);
    if (len == 0) {
        return 0;
    }
    int singleQ = 0, doubleQ = 0, countValid = 0;
    bool inDQ = false;
    for (int i = 0; i < len; i++) {
        if (str[i] == SINGLEQUOTE && !inDQ) {
            singleQ++;
        }
        else if (str[i] == DOUBLEQUOTE) {
            inDQ = inDQ == true ? false : true;
            doubleQ++;
        }
        if (singleQ % 2 == 0 && doubleQ % 2 == 0) {
            if (str[i] == '(') {
                countValid++;
            }
            else if (str[i] == ')') {
                countValid--;
            }
        }
    }
    return countValid;
}

bool isVariable(char* str) {
    int len = strlen(str);
    if (len != 1) {
        return false;
    }
    if (!isupper(str[0])) {
        return false;
    }
    return true;
}

bool isLisp(char* str) {

    if (strsame(str, "CAR") || strsame(str, "CDR") || strsame(str, "CONS")) {
        return true;
    }
    return false;
}

bool strsame(char* str1,char* str2){
    if(strcmp(str1,str2) == 0){
        return true;
    }
    return false;
}

void unParens(char* str) {
    if (!str) {
        return;
    }
    int len = strlen(str);
    int from = str[0] == '(' ? 1 : 0;
    int end = str[len - 1] == ')' ? len - 1 : len;
    substring(str, from, end);
    if (str[0] == '(' || str[strlen(str) - 1] == ')') {
        unParens(str);
    }
}

void getFirstToken(char* firstToken, char* rest) {
    if (!firstToken || !rest) {
        return;
    }
    strcpy(firstToken, rest);
    int len = strlen(firstToken);
    int i = 0, firstSpaceIndex = 0, countSpace = 0;
    for (int i = 0; i < len; i++) {
        if (firstToken[i] == ' ') {
            countSpace++;
        }
    }
    if (countSpace == 0) {
        strcpy(rest, "");
        return;
    }
    bool findSpace = false;
    while (i < len && !findSpace) {
        if (firstToken[i] == ' ') {
            findSpace = true;
            firstSpaceIndex = i;
        }
        i++;
    }
    substring(firstToken, 0, firstSpaceIndex);
    int ft_len = strlen(firstToken);
    substring(rest, ft_len, len);
    removeSpaceAtSides(rest);
    }

/*Remove space at head and tail*/
void removeSpaceAtSides(char* str) {
    int len = strlen(str);
    if (len == 0) {
        return;
    }
    if (str[0] == ' ') {
        substring(str, 1, len);
        removeSpaceAtSides(str);
    }
    else if (str[len - 1] == ' ') {
        substring(str, 0, len - 1);
        removeSpaceAtSides(str);
    }
}

void substring(char* buffer, int from, int end) {
    int len = (int)strlen(buffer);
    if (from > end || len == 0) {
        return;
    }
    char tmp[MAXNUMTOKENS] = "";
    int start = 0;
    for (int i = from; i < end;i++) {
        char cur = buffer[i];
        tmp[start++] = cur;
    }
    strcpy(buffer, tmp);
}

void getFirstLiteral(char* str) {
    if (!str) {
        return;
    }
    int len = strlen(str);
    bool hasStart = false;
    int start = -1, end = -1, count = 0;
    for (int i = 0; i < len;i++) {
        if (str[i] == '\'') {
            count++;
            if (hasStart) {
                end = i;
                i = len;
            }
            else {
                start = i;
                hasStart = true;
            }
        }
    }
    if (count != DOUBLE) {
        return;
    }
    if (start < 0 || end < 0) {
        return;
    }
    substring(str, start, end + 1);
}

void readMultipleFiles() {
    Program* pTest1 = READFILE("test1.ncl");
    Program* pTest2 = READFILE("test2.ncl");
    Program* pTest3 = READFILE("test3.ncl");
    Program* pTest4 = READFILE("test4.ncl");
    Program* pTest5 = READFILE("testl.ncl");
    Program* pTest6 = READFILE("simploop.ncl");
    Program* pTest7 = READFILE("printset.ncl");
    Program* pTest8 = READFILE("fib.ncl");
    Program* pTest9 = READFILE("basicprint.ncl");
    Program* pTest10 = READFILE("triv.ncl");
    Program* pTest11 = READFILE("demo1.ncl");
    Program* pTest12 = READFILE("demo2.ncl");
    Program* pTest13 = READFILE("demo3.ncl");
    assert(startProgram(pTest1));
    assert(startProgram(pTest2));
    assert(startProgram(pTest3));
    assert(startProgram(pTest4));
    assert(startProgram(pTest5));
    assert(startProgram(pTest6));
    assert(startProgram(pTest7));
    assert(startProgram(pTest8));
    assert(startProgram(pTest9));
    assert(startProgram(pTest10));
    assert(startProgram(pTest11));
    assert(startProgram(pTest12));
    assert(startProgram(pTest13));
    free(pTest1);
    free(pTest2);
    free(pTest3);
    free(pTest4);
    free(pTest5);
    free(pTest6);
    free(pTest7);
    free(pTest8);
    free(pTest9);
    free(pTest10);
    free(pTest11);
    free(pTest12);
    free(pTest13);
}

void testReadFile() {
    /*Below is the test for readfile*/
    printf("\n---TEST CASE 1---\n");
    Program* pTest1 = READFILE("test1.ncl");
    char buffer[MAXNUMTOKENS] = "";
    assert(pTest1->firstLine == 1);
    assert(pTest1->lastLine == 12);
    assert(strsame(pTest1->wds[3], "(WHILE (GREATER A '2') ("));
    assert(strsame(pTest1->wds[8], "(SET A (MINUS A '1') )"));
    assert(strsame(pTest1->wds[11], ") )"));
    assert(startProgram(pTest1));
    strcpy(buffer, pTest1->str);
    unParens(buffer);
#ifdef INTERP
    Var testInfo[NUMALPH] = { {false,NIL} };
#else 
    char rest[MAXNUMTOKENS] = "";
    assert(parseInstrcts(buffer));
    assert(parseInstrcts(pTest1->wds[10]));
    assert(parseInstrct(pTest1->wds[4]));
    assert(parseInstrct(pTest1->wds[5]));
    assert(parseInstrct(pTest1->wds[7]));
    strcpy(buffer, pTest1->wds[10]);
    unParens(buffer), removeSpaceAtSides(buffer);
    strcpy(rest, buffer);
    getFirstToken(buffer, rest);
    assert(parseConditionInstrcts(buffer, rest));
#endif
    printf("\n---TEST CASE 2---\n");
    Program* pTest2 = READFILE("test2.ncl");
    assert(pTest2->firstLine == 4);
    assert(pTest2->lastLine == 13);
    assert(startProgram(pTest2));
    strcpy(buffer, pTest2->str);
    unParens(buffer), removeSpaceAtSides(buffer);
#ifdef INTERP
    for (int i = 0; i < NUMALPH; i++) {
        testInfo[i].isSet = false;
        testInfo[i].list = NIL;
    }
    assert(interpInstrcts(testInfo, buffer));
#else
    assert(parseInstrcts(buffer));
    sepFirstInstrct(buffer, rest);
    strsame(buffer, "(SET A (LENGTH NIL))");
    strcpy(buffer, rest);
    sepFirstInstrct(buffer, rest);
    strsame(buffer, ("SET B '(-1)'"));
    strcpy(buffer, rest);
    unParens(buffer), removeSpaceAtSides(buffer);
    strcpy(rest, buffer);
    getFirstToken(buffer, rest);
    assert(parseConditionInstrcts("WHILE", rest));
#endif
    free(pTest1);
    free(pTest2);
}

void tests() {
    /*Comment these two function as they will interp PrintFunction will cause confusion in output*/

    /* ReadMultipleFiles() helps to build code coverage report
    while testReadfile() helps to check details of each input file*/

    // readMultipleFiles();
    // testReadFile();
    char buffer[MAXNUMTOKENS] = "", rest[MAXNUMTOKENS] = "";
#ifdef INTERP
    /*Below IS TEST OF INTERP PRINT FUNCTION*/
    Var testInfo[NUMALPH] = { {false,NIL} };
    assert(interpInstrcts(testInfo, "(SET A '5') (SET X (LENGTH A) )"));
    assert(interpInstrcts(testInfo, "(SET H (CONS NIL '(1 2)') ) (SET T (LESS H '10') )"));
    strcpy(buffer, "A (CDR B)");
    assert(interpSetFunc(testInfo, "SET", buffer));
    strcpy(buffer, "C (CONS A (CDR (CAR B) ) )");
    assert(interpSetFunc(testInfo, "SET", buffer));
    strcpy(buffer, "D (EQUAL A (CDR (CDR '(1 2 -3)') ) )");
    assert(interpSetFunc(testInfo, "SET", buffer));
    // assert(interpPrintFunc(testInfo, "PRINT", "A"));
    // assert(interpPrintFunc(testInfo, "PRINT", "\"Testing interp PrintFunc\""));
    // assert(interpPrintFunc(testInfo, "PRINT", "(CDR B)"));
    assert(interpListFunc(testInfo, "(CDR '(2 3 -8)')"));
    assert(interpListFunc(testInfo, "(CONS NIL (CAR H) )"));
    assert(interpListFunc(testInfo, "(LENGTH '(2 3 -8)')"));
    assert(interpList(testInfo, "NIL"));
    lisp* l1 = interpCar(testInfo, "CAR", "'(2 8 (9 7))'");
    assert(lisp_getval(l1) == 2);
    assert(lisp_length(l1) == 0);
    lisp* l2 = interpCdr(testInfo, "CDR", "'(2 8 (9 7) )'");
    assert(lisp_getval(l2) == 0);
    assert(lisp_length(l2) == 2);
    assert(!interpCdr(testInfo, "CDR", "NIL"));
    lisp* l3 = interpConsNCat(testInfo, "CONS", "C NIL");
    assert(lisp_getval(l3) == 0);
    assert(lisp_length(l3) == 1);
    assert(interpLength(testInfo, "LENGTH", "H") == 3);
    assert(interpLength(testInfo, "LENGTH", "'(2 -3 (2 6) )'") == 3);
    assert(interpLength(testInfo, "LENGTH", "'-283'") == 0);
    assert(interpOpComp(testInfo, "MINUS", "(CAR H) (LENGTH H)"));
    assert(interpOpComp(testInfo, "PLUS", "(CAR NIL) (LENGTH '(2 -9)')") == 2);
    assert(interpOpComp(testInfo, "LESS", "'5' (LENGTH A)") == 0);
    assert(doCompareOrOperation("EQUAL", 5, 5) == 1);
    assert(doCompareOrOperation("EQUAL", -5, 5) == 0);
    assert(doCompareOrOperation("GREATER", -5, 5) == 0);
    assert(doCompareOrOperation("MINUS", -5, 5) == -10);
    assert(doCompareOrOperation("PLUS", -5, 5) == 0);
    lisp* l4 = getList(testInfo, "NIL");
    assert(lisp_getval(l4) == 0);
    assert(lisp_length(l4) == 0);
    lisp* l5 = getList(testInfo, "'(1 -1 -2)'");
    assert(lisp_getval(l5) == 0);
    assert(lisp_length(l5) == 3);
    lisp* l6 = getList(testInfo, "(CONS A H)");
    assert(lisp_getval(l6) == 0);
    assert(lisp_length(l6) == 4);
    assert(!lisp_isatomic(l6));
    lisp* l7 = getList(testInfo, "'-100'");
    assert(lisp_getval(l7) == -100);
    assert(lisp_length(l7) == 0);
    assert(lisp_isatomic(l7));
    lisp* l8 = interpConsNCat(testInfo, "CAT", "'(1)' '(2)'");
    lisp_tostring(l8, buffer);
    assert(strsame(buffer, "(1 2)"));
    lisp* l9 = interpConsNCat(testInfo, "CAT", "(CDR H) '(2 5 -5 1)'");
    lisp_tostring(l9, buffer);
    assert(strsame(buffer, "(1 2 5 -5 1)"));
    assert(interpABS(testInfo, "ABS", "'-101'") == 101);
    assert(interpABS(testInfo, "ABS", "NIL") == 0);
    testInfo['Z' - 'A'].isSet = true;
    testInfo['Z' - 'A'].list = lisp_atom(-8);
    testInfo['U' - 'A'].isSet = true;
    testInfo['U' - 'A'].list = lisp_atom(3);
    assert(interpABS(testInfo, "ABS", "Z") == 8);
    assert(interpABS(testInfo, "ABS", "(CAR '(-7 2)')") == 7);
    assert(interpABS(testInfo, "ABS", "(CAR (CDR '(7 -2)') )") == 2);
    assert(interpABS(testInfo, "ABS", "(CAR (CAR (CONS '(-7 2)' U)))") == 7);
    testInfo['H' - 'A'].list = lisp_atom(9);
    testInfo['H' - 'A'].isSet = true;
    /*Testing Swap Function*/
    assert(lisp_getval(testInfo['H' - 'A'].list) == 9);
    assert(lisp_getval(testInfo['Z' - 'A'].list) == -8);
    assert(interpSwap(testInfo, "SWAP", "Z H"));
    assert(lisp_getval(testInfo['H' - 'A'].list) == -8);
    assert(lisp_getval(testInfo['Z' - 'A'].list) == 9);
    testInfo['P' - 'A'].list = lisp_atom(-88);
    testInfo['P' - 'A'].isSet = true;
    assert(interpSwap(testInfo, "SWAP", "P H"));
    assert(!interpSwap(testInfo, "SWAP", "P HI"));
    assert(lisp_getval(testInfo['H' - 'A'].list) == -88);
    assert(lisp_getval(testInfo['P' - 'A'].list) == -8);
    doSwap(testInfo, "SWAP", 'H', 'P');
    assert(lisp_getval(testInfo['H' - 'A'].list) == -8);
    assert(lisp_getval(testInfo['P' - 'A'].list) == -88);
    /*B is not set, so P will swap to default status */
    testInfo['B' - 'A'].list = NIL;
    testInfo['B' - 'A'].isSet = false;
    doSwap(testInfo, "SWAP", 'B', 'P');
    assert(lisp_getval(testInfo['B' - 'A'].list) == -88);
    assert(lisp_getval(testInfo['P' - 'A'].list) == 0);
    assert(testInfo['B' - 'A'].isSet);
    assert(!testInfo['P' - 'A'].isSet);
    assert(interpResetFunc(testInfo, "RESET", "A"));
    assert(!testInfo['A' - 'A'].isSet);
    assert(lisp_getval(testInfo['A' - 'A'].list) == 0);
    assert(interpResetFunc(testInfo, "RESET", "Z"));
    assert(!testInfo['Z' - 'A'].isSet);
    assert(lisp_getval(testInfo['Z' - 'A'].list) == 0);
    /*l9 has been freed by others*/
    lisp_free(&l1);
    lisp_free(&l2);
    lisp_free(&l3);
    lisp_free(&l4);
    lisp_free(&l5);
    lisp_free(&l6);
    lisp_free(&l7);
    lisp_free(&l8);
    for (int i = 0; i < NUMALPH; i++) {
        testInfo[i].isSet = false;
        testInfo[i].list = NIL;
    }
#else 
    /*test parseSetFunction*/
    strcpy(buffer, "B C");
    assert(parseSetFunc("SET", buffer));
    strcpy(buffer, "Z '((1 2) 4 5 (6 7))'");
    assert(parseSetFunc("SET", buffer));
    strcpy(buffer, "Z (LENGTH A)");
    assert(parseSetFunc("SET", buffer));
    strcpy(buffer, "K NIL");
    assert(parseSetFunc("SET", buffer));
    strcpy(buffer, "T '())))(**('");
    assert(parseSetFunc("SET", buffer));
    /*test parsePrintFunction*/
    strcpy(buffer, "\"Current doing testing print function\"");
    assert(parsePrintFunc("PRINT", buffer));
    strcpy(buffer, "H");
    assert(parsePrintFunc("PRINT", buffer));
    strcpy(buffer, "(LENGTH H)");
    assert(parsePrintFunc("PRINT", buffer));
    strcpy(buffer, "(MINUS A B)");
    assert(parsePrintFunc("PRINT", buffer));
    strcpy(buffer, "'(1 2 -5 (5 60))'");
    assert(parsePrintFunc("PRINT", buffer));
    /*parser considers below case as literal*/
    strcpy(buffer, "\"'(((((()'\"");
    assert(parsePrintFunc("PRINT", buffer));
    /*parser considers below case as literal*/
    strcpy(buffer, "\"'*&&$*(&$W)'\"");
    assert(parsePrintFunc("PRINT", buffer));
    /*test parseListFunction/List*/
    strcpy(buffer, "(MINUS A B)");
    assert(parseListFunc("CAR", buffer));
    strcpy(buffer, "'(1 2 -5 (5 60))'");
    assert(parseListFunc("CDR", buffer));
    assert(parseList(buffer));
    strcpy(buffer, "(LENGTH A) (CONS A B)");
    assert(parseTwoList(buffer));
    assert(parseListFunc("CONS", buffer));
    assert(parseListFunc("GREATER", buffer));
    strcpy(buffer, "NIL NIL");
    assert(parseListFunc("CONS", buffer));
    assert(parseListFunc("LESS", buffer));
    assert(parseTwoList(buffer));
    strcpy(buffer, "NIL");
    assert(parseListFunc("LENGTH", buffer));
    assert(parseList(buffer));
    /*parser considers below case as literal*/
    strcpy(buffer, "'00000)('");
    assert(countFrequency(buffer, '0') == 5);
    assert(isLiteral(buffer));
    assert(parseListFunc("LENGTH", buffer));
    assert(parseList(buffer));
    strcpy(buffer, "C ')()('");
    assert(countValidBrace(buffer) == 0);
    assert(parseListFunc("EQUAL", buffer));
    assert(parseTwoList(buffer));
    getFirstLiteral(buffer);
    assert(strsame(buffer, "')()('"));
    strcpy(buffer, "'(-1 8 (8))");
    assert(!isLiteral(buffer));

    strcpy(buffer, "NIL");
    assert(parseList("NIL"));
    strcpy(buffer, "k");
    assert(!parseList(buffer));
    strcpy(buffer, "NIl");
    assert(!parseList(buffer));
    /*parser considers below case as literal*/
    strcpy(buffer, "'N'");
    assert(isLiteral(buffer));
    assert(parseList(buffer));
    strcpy(buffer, "'@@@@@'");
    getFirstLiteral(buffer);
    strsame(buffer, "'@@@@@'");
    assert(countFrequency(buffer, '@') == 5);
    assert(parseList(buffer));
    strcpy(buffer, "(GREATER A B)");
    assert(parseList(buffer));
    assert(!parseList("HELLO"));
    assert(parseTwoList("H H"));
    assert(parseTwoList("H NIL"));
    assert(!parseTwoList("HI NIL"));
    /*parser considers below case as two literals*/
    assert(parseTwoList("'**&' ')))(('"));
    assert(countValidBrace(buffer) == 0);
    assert(parseTwoList("(CONS A B) (LENGTH I)"));
    assert(parseCondition("(GREATER A '(5 5 5 (3 4))')"));
    assert(parseCondition("(EQUAL '(2 3)' '(5 5 5 (3 4))')"));
    assert(parseCondition("(LESS H NIL"));
    /*can only reset variable*/
    assert(parseResetFunc("RESET", "A"));
    assert(!parseResetFunc("RESET", "NIL"));
    assert(!parseResetFunc("RESET", "'(2 4 6 8)'"));
    /*can only swap between variables*/
    assert(parseSwapFunc("SWAP", "X Y"));
    assert(parseSwapFunc("SWAP", "X       Z"));
    assert(parseSwapFunc("SWAP", "X Z"));
    assert(!parseSwapFunc("SWAP", "X '1'"));
    assert(!parseSwapFunc("SWAP", "NIL N"));
    assert(!parseSwapFunc("SWAP", "(5 7 (1 2)) N"));

#endif
    /*helper function testing*/
    char str[MAXNUMTOKENS] = "";
    strcpy(buffer, "(((S)))");
    unParens(buffer);
    assert(strsame(buffer, "S"));
    strcpy(str, "WR_UITEH((*)");
    getFirstToken(buffer, str);
    assert(strsame(buffer, "WR_UITEH((*)"));
    assert(strsame(str, ""));
    strcpy(buffer, "(2345)"), strcpy(str, buffer);
    getFirstToken(buffer, str);
    assert(strsame(buffer, "(2345)"));
    assert(strsame(rest, ""));
    assert(!isCompare("BIGGER"));
    assert(!isCompare("smaller"));
    assert(!isCompare("Less"));
    assert(!isCompare("NIL"));
    assert(isCompare("GREATER") && isCompare("LESS") && isCompare("EQUAL"));
    assert(isBrace('(') && isBrace(')'));
    assert(!isBrace(' '));
    assert(!isBrace('}'));
    strcpy(buffer, "(PRINT '(5 9)') (SET H (CONS A B)) (PRINT \"NUCLEI LANGUAGE\")");
    sepFirstInstrct(buffer, rest);
    assert(strsame(buffer, "(PRINT '(5 9)')"));
    getFirstLiteral(buffer);
    strsame(buffer, "'(5 9)'");
    assert(strsame(rest, "(SET H (CONS A B)) (PRINT \"NUCLEI LANGUAGE\")"));
    strcpy(buffer, rest);
    sepFirstInstrct(buffer, rest);
    assert(strsame(buffer, "(SET H (CONS A B))"));
    assert(countFrequency(buffer, '(') == 2);
    assert(strsame(rest, "(PRINT \"NUCLEI LANGUAGE\")"));
    strcpy(buffer, rest);
    sepFirstInstrct(buffer, rest);
    assert(strsame(buffer, "(PRINT \"NUCLEI LANGUAGE\")"));
    assert(strsame(rest, ""));
    assert(countFrequency(buffer, '"') == 2);
    assert(isConstString("\"NIL\""));
    assert(!isConstString(""));
    assert(isConstString("\"INTERPING LISP...\""));
    assert(!isConstString("'INTERPING LISP...'"));
    assert(!isConstString("(INTERPING LISP...)"));
    assert(isCondition("IF") && isCondition("WHILE"));
    assert(!isCondition("if"));
    assert(!isCondition("While"));
    assert(isOperation("PLUS") && isOperation("MOD"));
    assert(isOperation("MUL") && isOperation("DIV"));
    assert(!isOperation("Minus") && !isOperation("mul"));
    assert(isLiteral("'A'") && isLiteral("' '") && isLiteral("'()'"));
    assert(!isLiteral("' (1 2 5 ( 9 8 ))"));
    assert(countValidBrace("' (1 2 5 ( 9 8 ))") == 0);
    assert(countValidBrace("") == 0);
    assert(isLiteral("'NIL'") && !isLiteral("NULL"));
    assert(countValidBrace("(1 2 5 '( 9 8 )'") == 1);
    assert(countValidBrace("'( 9 8 )' ))))") == -4);
    assert(!isVariable("a"));
    assert(!isVariable("A_"));
    assert(!isVariable("A "));
    assert(!isVariable(" A "));
    assert(isLisp("CDR") && isLisp("CAR") && isLisp("CONS"));
    assert(!isLisp("CON"));
    assert(!isLisp("Car"));
    assert(!isLisp("cdr"));

}

