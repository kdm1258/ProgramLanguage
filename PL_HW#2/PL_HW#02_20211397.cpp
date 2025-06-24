#include <bits/stdc++.h>
using namespace std;

#define MAX_TOKEN_LEN 20
#define MAX_TOKENS 10000
#define MAX_LINE_LEN 100000

/* Token Code*/
#define INT_LIT 10
#define IDENT 11
#define ASSIGN_OP 20
#define ADD_OP 21
#define SUB_OP 22
#define MULT_OP 23
#define LEFT_PAREN 31
#define RIGHT_PAREN 32
#define SEMICOLON 33
#define END 34
#define PRINT 41

#define EQ_OP 50       // ==
#define NEQ_OP 51      // !=
#define LT_OP 52       // <
#define GT_OP 53       // >

#define IF 60
#define ELSE 61
#define WHILE 62
#define DO 63
#define LBRACE 64      // {
#define RBRACE 65      // }
#define TYPE_INT 66    // integer

/*Token Type*/
#define LETTER 1
#define DIGIT 2
#define UNKNOWN 3

/* Token */
typedef struct _Token{
    int tokenCode;
    char lexeme[MAX_TOKEN_LEN];
}Token;

typedef struct _WhileLoopContext {
    int conditionStartIndex;    // 조건문의 시작 토큰 인덱스
    int bodyStartIndex;         // 루프 본문의 시작 토큰 인덱스 (LBRACE 다음)
    int closingBraceIndex;      // 루프 본문의 닫는 중괄호 '}' 토큰의 인덱스
    int afterLoopIndex;         // 루프 전체가 끝난 후 이동할 토큰 인덱스 (세미콜론 다음)
} WhileLoopContext;

typedef struct _IfElseContext {
    int conditionStartIndex;      // 조건문 시작
    int thenBlockStartIndex;      // then 블록 시작 (LBRACE 다음)
    int thenBlockClosingBraceIndex; // then 블록 끝 (RBRACE)
    int elseBlockStartIndex;      // else 블록 시작 (LBRACE 다음, else 키워드 뒤)
    int elseBlockClosingBraceIndex; // else 블록 끝 (RBRACE)
    int afterIfElseIndex;         // if-else 전체가 끝난 후 (세미콜론 다음)
} IfElseContext;

// Global declarations에 추가
vector<IfElseContext> ifElseStack;

/* Global declararions */
char buffer[MAX_LINE_LEN];      //한줄씩 입력받기 버퍼
int tokenType;                  //토큰 타입 : 만약 토큰타입이 달라지면 Syntax Error ex)ab3
int tokenIndex;                 //현재 토큰 인덱스
int lexIndex;                   //lexeme 인덱스 
Token token[MAX_TOKENS];        //token 저장 전역 배열열
map <string, long long> symbolTable;  //변수 저장용 
int currentTokenIndex;          //parser에서 토큰 추적 index
int printed;                    //프린트호출여부(개행관리리)
bool errorFlag = false;         //에러 여부 확인 전역 변수
vector<long long> printBuffer;        //프린트 버퍼
vector<WhileLoopContext> whileStack; // 이 줄을 추가


/* Function declarations */
int lookup(char);
int lexer(char*);
int findTokenType(char);

void program(void);
int statement(void);
long long term(void);
long long factor(void);
long long number(void);
string var(void);
bool digit(char);
bool alphabet(char);
int declaration(void);
int type(void);
int relop(void);
bool bexpr(void);
long long aexpr(void);
bool validateBexpr(void);
int validateStatement(void);

Token currentToken();
void nextToken(void);
bool match(int);
int expect(int);


//=====================================================================================================
// lexer

/* -----------------------------------------------------------------------------------
 * 설명 : char을 입력으로 받아 해당 symbol이 정의되었는지 확인 하는 함수
 * 매계 : 확인하고자 하는 char c
 * 반환 : 성공 = 해당 symbol의 Token code , 실패 = -1
 * -----------------------------------------------------------------------------------
 */
int lookup(char c){
    switch(c){
        case '=':
            return ASSIGN_OP;
        case '+':
            return ADD_OP;
        case '-':
            return SUB_OP;
        case '*':
            return MULT_OP;
        case '(':
            return LEFT_PAREN;
        case ')':
            return RIGHT_PAREN;
        case '{':
            return LBRACE;
        case '}':
            return RBRACE;
        case ';':
            return SEMICOLON;
        case '<':
            return LT_OP;
        case '>':
            return GT_OP;
        default:
            return -1;
    }

}

/* -----------------------------------------------------------------------------------
 * 설명 : 토큰타입 구분 함수
 * 매계 : char c
 * 반환 : Token Type
 * -----------------------------------------------------------------------------------
 */
int findTokenType(char c){
    if(alphabet(c)){
        return LETTER;
    }
    else if(digit(c)){
        return DIGIT;
    }
    else{
        return UNKNOWN;
    }
}

/* -----------------------------------------------------------------------------------
 * 설명 : line을 입력으로 받아 tokenize, Token배열에 저장
 * 매계 : 입력받은 line
 * 반환 : 성공 = 0 , 실패 = -1
 * -----------------------------------------------------------------------------------
 */
int lexer(char* input) {
    int len = strlen(input);
    tokenIndex = 0;

    for (int i = 0; i < len;) {
        //공백제거
        while (i < len && isspace(input[i])) i++;
        if (i >= len) break;

        //"==" "!="처리 
        if(input[i] == '=' && i+1 < len && input[i+1] == '='){
            Token new_token;
            new_token.tokenCode = EQ_OP;
            strcpy(new_token.lexeme, "==");
            i += 2;
            token[tokenIndex++] = new_token;
            continue;
        }
        if (input[i] == '!' && i + 1 < len && input[i + 1] == '=') {
            Token new_token;
            new_token.tokenCode = NEQ_OP;
            strcpy(new_token.lexeme, "!=");
            i += 2;
            token[tokenIndex++] = new_token;
            continue;
        }

        int currentType = findTokenType(input[i]);

        // UNKNOWN → symbol 처리
        if (currentType == UNKNOWN) {
            int code = lookup(input[i]);

            // 정의되지 않은 기호
            if (code == -1) {
                printf("Syntax Error!\n");
                return -1;
            }

            // 다음 글자도 공백 없이 붙으면 에러 (예: ++, =+ 등)
            if (i + 1 < len && !isspace(input[i + 1])) {
                printf("Syntax Error!\n");
                return -1;
            }

            // 단독 기호로 처리
            Token new_token;
            new_token.tokenCode = code;
            new_token.lexeme[0] = input[i++];
            new_token.lexeme[1] = '\0';
            token[tokenIndex++] = new_token;
            continue;
        }

        // 식별자 또는 숫자 처리
        Token new_token;
        lexIndex = 0;

        while (i < len && !isspace(input[i])) {
            int thisType = findTokenType(input[i]);
            if (thisType != currentType) {
                printf("Syntax Error!\n");
                return -1;
            }
            new_token.lexeme[lexIndex++] = input[i++];
        }

        new_token.lexeme[lexIndex] = '\0';

        // 토큰코드 매핑
        if (currentType == LETTER) {
            if (strcmp(new_token.lexeme, "print") == 0) new_token.tokenCode = PRINT;
            else if(strcmp(new_token.lexeme, "integer") ==0) new_token.tokenCode = TYPE_INT;
            else if(strcmp(new_token.lexeme, "if") ==0) new_token.tokenCode = IF;
            else if(strcmp(new_token.lexeme, "else") ==0) new_token.tokenCode = ELSE;
            else if(strcmp(new_token.lexeme, "while") ==0) new_token.tokenCode = WHILE;
            else if(strcmp(new_token.lexeme, "do") ==0) new_token.tokenCode = DO;
            else
                new_token.tokenCode = IDENT;  // 일반 식별자도 IDENT 
        } else if (currentType == DIGIT) {
            new_token.tokenCode = INT_LIT;
        } else {
            printf("Syntax Error!\n");
            return -1;
        }

        token[tokenIndex++] = new_token;
    }
    // token이 끝남을 표시시
    token[tokenIndex].tokenCode = END;
    token[tokenIndex].lexeme[0] = '\0';

    return 0;
}

//=====================================================================================================
//parser

/*-----------------------------------------------------------------------------------
 * 설명 : 현재 토큰을 반환함
 * 반환 : 현재 토큰
 * -----------------------------------------------------------------------------------
 */
Token currentToken(void){
    return token[currentTokenIndex];
}

/*-----------------------------------------------------------------------------------
 * 설명 : tokenIndex 증가
 * -----------------------------------------------------------------------------------
 */
void nextToken(void){
    currentTokenIndex++;
}

/*-----------------------------------------------------------------------------------
 * 설명 : expected 값과 token Code를 비교
 *        ADD_OP 처럼 있어도 되도 없어도 되는경우에 사용하기 위해 에러메시지를 출력하는 expect와 분리
 * 반환 : 성공 : true, 실패 : false
 * -----------------------------------------------------------------------------------
 */
bool match(int expected){
    if(currentToken().tokenCode == expected){
        nextToken();
        return true;
    }
    return false;
}

/*-----------------------------------------------------------------------------------
 * 설명 : expected 값과 token Code를 비교 후 false 시 에러 처리
 * 반환 : 성공 : 0, 실패 : -1
 * -----------------------------------------------------------------------------------
 */
int expect(int expected){
    if(!match(expected)){
        errorFlag = true;
        return -1;
    }
    return 0;
}

/* -----------------------------------------------------------------------------------
 * 설명 : <program> -> {<declaration>} {<statement>}
 * -----------------------------------------------------------------------------------
 */
void program(void){
    symbolTable.clear();
    errorFlag = false;
    printed = 0;
    
    // 입력인 END가 아니고 에러가 없다면 declaration()호출
    while(currentToken().tokenCode == TYPE_INT && !errorFlag){
        declaration();
    }

    // 입력인 END가 아니고 에러가 없다면 statement()호출
    while(currentToken().tokenCode != END && !errorFlag){
        statement();
    }
    if(errorFlag){
        printf("Syntax Error!\n");
    }
    else if (printed) {
        if (!errorFlag) {
            for (long long val : printBuffer) {
                printf("%lld ", val);
            }
            putchar('\n');
        } 
        printBuffer.clear(); // 다음 줄 입력 대비
    }
}

/* -----------------------------------------------------------------------------------
 * 설명 : <declaration> -> <type> <var>;
 * 반환 : 성공 :0, 실패 : -1
 * -----------------------------------------------------------------------------------
 */
int declaration(void){
    if(type() <0){
        errorFlag = true;
        return -1;
    }

    // 선언되지 않은 변수인지 확인
    // if (symbolTable.count(currentToken().lexeme) != 0) {
    //     errorFlag = true;
    //     return -1;
    // }
    string varName = var();
    
    if(expect(SEMICOLON) == -1){
        errorFlag = true;
        return -1;
    }

    symbolTable[varName] = 0;
    return 0;
}

/* -----------------------------------------------------------------------------------
 * 설명 : <type> -> integer
 * 반환 : 성공 :0, 실패 : -1
 * -----------------------------------------------------------------------------------
 */

int type(void){
    if(currentToken().tokenCode == TYPE_INT){
        nextToken();
        return 0;
    }
    return -1;
}
/* -----------------------------------------------------------------------------------
 * 설명 : 문법 검사용 bexpr - 실제로 값을 계산하지 않고 문법만 검사
 * 반환 : 문법이 올바르면 true, 아니면 false
 * -----------------------------------------------------------------------------------
 */
bool validateBexpr(void){
    // 첫 번째 <var> 파싱
    if (currentToken().tokenCode != IDENT) {
        errorFlag = true;
        return false;
    }
    // 선언되지 않은 변수인지 확인
    if (symbolTable.count(currentToken().lexeme) == 0) {
        errorFlag = true;
        return false;
    }
    nextToken(); // var 소비

    // <relop> 파싱
    int op = currentToken().tokenCode;
    if(op != EQ_OP && op != NEQ_OP && op != LT_OP && op != GT_OP){
        errorFlag = true;
        return false;
    }
    nextToken(); // relop 소비

    // 두 번째 <var> 파싱
    if (currentToken().tokenCode != IDENT) {
        errorFlag = true;
        return false;
    }
    // 선언되지 않은 변수인지 확인
    if (symbolTable.count(currentToken().lexeme) == 0) {
        errorFlag = true;
        return false;
    }
    nextToken(); // var 소비

    return true;
}

/* -----------------------------------------------------------------------------------
 * 설명 : 문법 검사용 statement - 실제로 실행하지 않고 문법만 검사
 * 반환 : 성공 :0, 실패 : -1
 * -----------------------------------------------------------------------------------
 */
int validateStatement(void){
    //print
    if(currentToken().tokenCode == PRINT){
        nextToken();
        long long val = aexpr();
        if(expect(SEMICOLON)==-1) return -1;
    }
    else if(currentToken().tokenCode == IDENT){
         // 선언된 변수인지 확인
        if (symbolTable.count(currentToken().lexeme) == 0) {
            errorFlag = true;
            return -1;
        }
        nextToken(); // var 소비
        if(expect(ASSIGN_OP) == -1) return -1;
        long long res_expr = aexpr();
        if (errorFlag) return -1;
        if(expect(SEMICOLON) == -1) return -1;
    }
    else if(currentToken().tokenCode == WHILE){
        nextToken(); 
        if(expect(LEFT_PAREN) == -1) return -1;
        if(!validateBexpr()) return -1;
        if(expect(RIGHT_PAREN) == -1) return -1;
        if(expect(DO) == -1) return -1;
        if(expect(LBRACE) == -1) return -1;

        // 루프 본문 문법 검사만 수행
        while(currentToken().tokenCode != RBRACE && !errorFlag && currentToken().tokenCode != END){
            if(validateStatement() == -1) {
                return -1;
            }
        }
        
        if(expect(RBRACE) == -1) return -1;
        if(expect(SEMICOLON) == -1) return -1;
    }
    else if(currentToken().tokenCode == IF) {
        nextToken();
        if (expect(LEFT_PAREN) == -1) return -1;
        if(!validateBexpr()) return -1;
        if (expect(RIGHT_PAREN) == -1) return -1;
        if (expect(LBRACE) == -1) return -1;
        
        // then 블록 문법 검사
        while(currentToken().tokenCode != RBRACE && !errorFlag && currentToken().tokenCode != END){
            if(validateStatement() == -1) {
                return -1;
            }
        }
        
        if (expect(RBRACE) == -1) return -1;
        if (expect(ELSE) == -1) return -1;
        if (expect(LBRACE) == -1) return -1;

        // else 블록 문법 검사
        while(currentToken().tokenCode != RBRACE && !errorFlag && currentToken().tokenCode != END){
            if(validateStatement() == -1) {
                return -1;
            }
        }
        
        if (expect(RBRACE) == -1) return -1;
        if (expect(SEMICOLON) == -1) return -1;
    }
    else{
        errorFlag = true;
        nextToken();
        return -1;
    }
    return 0;
}

/* -----------------------------------------------------------------------------------
 * 설명 :<statement> -> <var> = <aexpr> ; | print <aexpr> ;  |
                        while ( <bexpr> ) do ‘ { ’ {<statement>} ‘ } ’  ;  |
                        if ( <bexpr> ) ‘ { ’ {<statement>} ‘ } ’ else ‘ { ’ {<statement>} ‘ } ’ ;
 * 반환 : 성공 :0, 실패 : -1
 * 전역변수 errorFlag로 에러여부 판단으로 수정함
 * -----------------------------------------------------------------------------------
 */

int statement(void){

    //print
    if(currentToken().tokenCode == PRINT){
        nextToken();
        //print 다음 <aexpr>이 오지 않으면 에러
        long long val = aexpr();

        //print가 ';'로 끝나지 않을 시 에러
        if(expect(SEMICOLON)==-1) return -1;
        printed = 1;
        printBuffer.push_back(val);
    }
    else if(currentToken().tokenCode == IDENT){
         // 선언된 변수인지 확인
        if (symbolTable.count(currentToken().lexeme) == 0) {
            errorFlag = true;
            return -1;
        }
        string varName = var(); 
        // '='기호 처리
        if(expect(ASSIGN_OP) == -1) return -1;
        
        //<expr> 처리 
        long long res_expr = aexpr();
        if (errorFlag) return -1;

        // ';'기호 처리
        if(expect(SEMICOLON) == -1) return -1;

        //변수에 값 할당
        symbolTable[varName] = res_expr;
    }
    else if(currentToken().tokenCode == WHILE){
        nextToken(); 

        // 1. 먼저 전체 구조를 문법 검사만 수행
        int savedTokenIndex = currentTokenIndex;
        
        if(expect(LEFT_PAREN) == -1) return -1;
        int conditionStartIndex = currentTokenIndex;
        
        if(!validateBexpr()) return -1;
        if(expect(RIGHT_PAREN) == -1) return -1;
        if(expect(DO) == -1) return -1;
        if(expect(LBRACE) == -1) return -1;
        
        int bodyStartIndex = currentTokenIndex;
        
        // 본문 문법 검사
        while(currentToken().tokenCode != RBRACE && !errorFlag && currentToken().tokenCode != END){
            if(validateStatement() == -1) {
                return -1;
            }
        }
        
        if(errorFlag) return -1;
        
        int closingBraceIndex = currentTokenIndex;
        if(expect(RBRACE) == -1) return -1;
        if(expect(SEMICOLON) == -1) return -1;
        int afterLoopIndex = currentTokenIndex;
        
        // 2. 문법 검사가 완료되면 실제 실행
        // 실행을 위해 토큰 인덱스를 조건문 시작으로 되돌림
        currentTokenIndex = savedTokenIndex;
        if(expect(LEFT_PAREN) == -1) return -1;
        
        // 실제 while 루프 실행
        while(true){
            int beforeConditionIndex = currentTokenIndex;
            bool cond = bexpr();
            
            if (errorFlag) return -1;
            if (!cond) break;
            
            // ')' 'do' '{' 건너뛰기
            if(expect(RIGHT_PAREN) == -1) return -1;
            if(expect(DO) == -1) return -1;
            if(expect(LBRACE) == -1) return -1;
            
            // 본문 실행
            while(currentToken().tokenCode != RBRACE && !errorFlag && currentToken().tokenCode != END){
                if(statement() == -1) return -1;
            }
            if(errorFlag) return -1;
            
            if(expect(RBRACE) == -1) return -1;
            
            // 다음 반복을 위해 조건문으로 되돌아감
            currentTokenIndex = beforeConditionIndex;
        }
        
        // 루프 종료 후 전체 while문 이후로 이동
        currentTokenIndex = afterLoopIndex;
        return 0;
    }
    else if(currentToken().tokenCode == IF) {
        nextToken(); // 'if' 소비

        // 1. 먼저 전체 구조를 문법 검사만 수행
        int savedTokenIndex = currentTokenIndex;
        
        if (expect(LEFT_PAREN) == -1) return -1;
        int conditionStartIndex = currentTokenIndex;

        if(!validateBexpr()) return -1;
        if (expect(RIGHT_PAREN) == -1) return -1;
        if (expect(LBRACE) == -1) return -1;
        int thenBlockStartIndex = currentTokenIndex;

        // then 블록 문법 검사
        while(currentToken().tokenCode != RBRACE && !errorFlag && currentToken().tokenCode != END){
            if(validateStatement() == -1) {
                return -1;
            }
        }
        if(errorFlag) return -1;
        
        int thenBlockClosingBraceIndex = currentTokenIndex;
        if (expect(RBRACE) == -1) return -1;
        if (expect(ELSE) == -1) return -1;
        if (expect(LBRACE) == -1) return -1;
        int elseBlockStartIndex = currentTokenIndex;

        // else 블록 문법 검사
        while(currentToken().tokenCode != RBRACE && !errorFlag && currentToken().tokenCode != END){
            if(validateStatement() == -1) {
                return -1;
            }
        }
        if(errorFlag) return -1;

        int elseBlockClosingBraceIndex = currentTokenIndex;
        if (expect(RBRACE) == -1) return -1;
        if (expect(SEMICOLON) == -1) return -1;
        int afterIfElseIndex = currentTokenIndex;

        // 2. 문법 검사가 완료되면 실제 실행
        // 실행을 위해 토큰 인덱스를 조건문 시작으로 되돌림
        currentTokenIndex = savedTokenIndex;
        if (expect(LEFT_PAREN) == -1) return -1;
        
        // 조건 평가
        bool cond = bexpr();
        if (errorFlag) return -1;
        
        if (expect(RIGHT_PAREN) == -1) return -1;
        if (expect(LBRACE) == -1) return -1;

        if (cond) { // 조건이 true: then 블록 실행
            while(currentToken().tokenCode != RBRACE && !errorFlag && currentToken().tokenCode != END){
                if(statement() == -1) return -1;
            }
            if (expect(RBRACE) == -1) return -1;
            
            // else 블록은 건너뛰기
            if (expect(ELSE) == -1) return -1;
            if (expect(LBRACE) == -1) return -1;
            
            // else 블록의 내용을 건너뛰기 위해 중괄호 매칭
            int braceCount = 1;
            while(braceCount > 0 && currentToken().tokenCode != END) {
                if(currentToken().tokenCode == LBRACE) braceCount++;
                else if(currentToken().tokenCode == RBRACE) braceCount--;
                nextToken();
            }
        }
        else { // 조건이 false: then 블록 건너뛰고 else 블록 실행
            // then 블록 건너뛰기
            int braceCount = 1;
            while(braceCount > 0 && currentToken().tokenCode != END) {
                if(currentToken().tokenCode == LBRACE) braceCount++;
                else if(currentToken().tokenCode == RBRACE) braceCount--;
                nextToken();
            }
            
            if (expect(ELSE) == -1) return -1;
            if (expect(LBRACE) == -1) return -1;
            
            // else 블록 실행
            while(currentToken().tokenCode != RBRACE && !errorFlag && currentToken().tokenCode != END){
                if(statement() == -1) return -1;
            }
            if (expect(RBRACE) == -1) return -1;
        }
        
        if (expect(SEMICOLON) == -1) return -1;
        return 0;
    }
    else{
        errorFlag = true;
        nextToken();
        return -1;
    }
    return 0;
}
 
/* -----------------------------------------------------------------------------------
 * 설명 : <bexpr> -> <var> <relop> <var> 
 * 반환 : bexpr 값 에러인 경우 false를 리턴하지만 errorFlag로 에러 관리
 * ----------------------------------------------------------------------------------
 */bool bexpr(void){
    // 첫 번째 <var> 파싱
    // 선언되지 않은 변수인지 확인
    if (symbolTable.count(currentToken().lexeme) == 0) {
        errorFlag = true;
        return false;
    }
    string left = var(); // var() 내부에서 nextToken() 호출됨

    // <relop> 파싱 (토큰 코드만 얻음)
    int op = relop();
    if(op == -1){ // relop에서 오류 발생 시
        errorFlag = true;
        return false;
    }
    nextToken(); // 여기서 <relop> 토큰을 소비

    // 두 번째 <var> 파싱
    // 선언되지 않은 변수인지 확인
    if (symbolTable.count(currentToken().lexeme) == 0) {
        errorFlag = true;
        return false;
    }
    string right = var(); // var() 내부에서 nextToken() 호출됨

    if(errorFlag) return false;
    long long lval = symbolTable[left];
    long long rval = symbolTable[right];

    // 관계 연산 수행
    switch(op){
        case EQ_OP: return lval == rval;
        case NEQ_OP: return lval != rval;
        case LT_OP: return lval < rval;
        case GT_OP: return lval > rval;
        default: // 이 경우는 발생하지 않아야 하지만, 안전을 위해 남겨둠
            errorFlag = true;
            return false;
    }
}
/* -----------------------------------------------------------------------------------
 * 설명 : <relop> → == | != | < | >  
 * 반환 : 성공 : 해당 심볼의 token code, 실패 : -1
 * -----------------------------------------------------------------------------------
 */
int relop(void){
    int op = currentToken().tokenCode;
    if(op != EQ_OP && op != NEQ_OP && op != LT_OP && op != GT_OP){
        errorFlag = true;
        return -1;
    }
    return op;
}

/* -----------------------------------------------------------------------------------
 * 설명 : <aexpr> -> <term> {( + | - ) <term>}  
 * 반환 : aexpr 값
 * -----------------------------------------------------------------------------------
 */
long long aexpr(void){
    long long res_term = term();
    if (errorFlag) return -1;

    //'+' or '-'기호 처리 
    while(!errorFlag && (currentToken().tokenCode == ADD_OP || currentToken().tokenCode == SUB_OP)){
        int op = currentToken().tokenCode;
        nextToken();
        long long val = term();
        if(errorFlag) return -1;
        if(op == ADD_OP) res_term += val;
        else if(op == SUB_OP) res_term -= val;        
    }
    return res_term;
}

/* -----------------------------------------------------------------------------------
 * 설명 : <term> -> <factor> {* <factor>}
 * 반환 : term값 
 * -----------------------------------------------------------------------------------
 */
long long term(void){
    long long res_factor = factor();
    if(errorFlag) return -1;

    // '*'기호 처리
    while(!errorFlag && (currentToken().tokenCode == MULT_OP)){
        nextToken();
        long long val = factor();
        if(errorFlag) return -1;

        res_factor *= val;
    }
    return res_factor;
}

/* -----------------------------------------------------------------------------------
 * 설명 : <factor> -> [ - ] ( <number> | <var> | '('<aexpr>')')
 * 반환 : factor값
 * -----------------------------------------------------------------------------------
 */
long long factor(void){
    long long sign = 1;

    if(currentToken().tokenCode == SUB_OP){
        sign = -1;
        nextToken();

        //연속된 - 에러처리
        if (currentToken().tokenCode == SUB_OP){
            errorFlag = true;
            nextToken();
            return -1;
        }
    }
    
    // number
    if(currentToken().tokenCode == INT_LIT){
        long long val = number();
        return sign*val;
    }
    //var
    else if(currentToken().tokenCode == IDENT){
        // 선언된 변수인지 확인
        if (symbolTable.count(currentToken().lexeme) == 0) {
            errorFlag = true;
            return -1;
        }
        string varName = var();
        return sign*symbolTable[varName];
    }
    //괄호
    else if(currentToken().tokenCode == LEFT_PAREN){
        nextToken();
        long long val = aexpr();
        if(expect(RIGHT_PAREN) == -1){
            nextToken();
            return -1;
        }
        return sign * val;
    }
    else{
        errorFlag = true;
        nextToken();
        return -1;
    }
}

/* -----------------------------------------------------------------------------------
 * 설명 : <number> -> <digit> {<digit>}
 * -----------------------------------------------------------------------------------
 */
long long number(void){
    long long val = atoll(currentToken().lexeme);
    nextToken();
    return val;
}

/* -----------------------------------------------------------------------------------
 * 설명 : <var> -> <alphabet>{<alphabet>}
 * -----------------------------------------------------------------------------------
 */
string var(void){
    if (currentToken().tokenCode != IDENT){
        errorFlag = true;
        return "";
    }
    string varName = currentToken().lexeme;

    nextToken();
    return varName;
}

/* -----------------------------------------------------------------------------------
 * 설명 : <alphabet> -> a | b | c | d | e | f | g | h | i | j | k | l | m | n | o | p | q | r | s | t | u | v | w | x | y | z
 * -----------------------------------------------------------------------------------
 */
bool alphabet(char c){
    char al[26] = {'a','b','c','d','e','f','g','h',
                 'i','j','k','l','m','n','o','p',
                 'q','r','s','t','u','v','w','x','y','z'};
    for(int i = 0 ; i < 26 ; i++){
        if (c == al[i]){
            return true;
        }
    }
    return false;
}

/* -----------------------------------------------------------------------------------
 * 설명 : <digit> 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 
 * -----------------------------------------------------------------------------------
 */
bool digit(char c){
    if(isdigit(c)){
        return true;
    }
    return false;
}

int main(void){
    while(true){
        //함수 시작 전 global var 초기화
        tokenIndex = 0;
        tokenType = 0;
        printBuffer.clear();

        lexIndex = 0;
        currentTokenIndex = 0;
        cout << ">> ";
        cin.getline(buffer, MAX_LINE_LEN);
        //sprintf(buffer, "integer i ; i = 1 ; print i ;");
        if (strlen(buffer) == 0) break;
        if (lexer(buffer) != -1){
            program();
            errorFlag = false;
        }
    }
}
