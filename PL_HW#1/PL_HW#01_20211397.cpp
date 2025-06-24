#include <bits/stdc++.h>
using namespace std;

#define MAX_TOKEN_LEN 100
#define MAX_TOKENS 100
#define MAX_LINE_LEN 1000

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

/*Token Type*/
#define LETTER 1
#define DIGIT 2
#define UNKNOWN 3

/* Token */
typedef struct _Token{
    int tokenCode;
    char lexeme[MAX_TOKEN_LEN];
}Token;

/* Global declararions */
char buffer[MAX_LINE_LEN];      //한줄씩 입력받기 버퍼
int tokenType;                  //토큰 타입 : 만약 토큰타입이 달라지면 Syntax Error ex)ab3
int tokenIndex;                 //현재 토큰 인덱스
int lexIndex;                   //lexeme 인덱스 
Token token[MAX_TOKENS];        //token 저장 전역 배열열
map <string, int> symbolTable;  //변수 저장용 
int currentTokenIndex;          //parser에서 토큰 추적 ind
int printed;                    //프린트호출여부(개행관리리)
bool errorFlag = false;         //에러 여부 확인 전역 변수
vector<int> printBuffer;        //프린트 버퍼


/* Function declarations */
int lookup(char);
int lexer(char*);
int findTokenType(char);

void program(void);
int statement(void);
int expr(void);
int term(void);
int factor(void);
int number(void);
string var(void);
bool digit(char);
bool alphabet(char);

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
        case ';':
            return SEMICOLON;
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
            if (strcmp(new_token.lexeme, "print") == 0)
                new_token.tokenCode = PRINT;
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
 * 설명 : <program> -> {<statement>}
 * -----------------------------------------------------------------------------------
 */
void program(void){
    symbolTable.clear();
    errorFlag = false;
    printed = 0;
    // 입력인 END가 아니고 에러가 없다면 statement()호출
    while(currentToken().tokenCode != END && !errorFlag){
        statement();
    }
    if(errorFlag){
        printf("Syntax Error!\n");
    }
    else if (printed) {
        if (!errorFlag) {
            for (int val : printBuffer) {
                printf("%d ", val);
            }
            putchar('\n');
        } 
        printBuffer.clear(); // 다음 줄 입력 대비
    }
}

/* -----------------------------------------------------------------------------------
 * 설명 : <statement> -> <var> = <expr> ; | print <var> ;
 * 반환 : 성공 :0, 실패 : -1
 * 전역변수 errorFlag로 에러여부 판단으로 수정함
 * -----------------------------------------------------------------------------------
 */
int statement(void){

    //print
    if(currentToken().tokenCode == PRINT){
        nextToken();
        //print 다음 <var>이 오지 않으면 에러
        if(currentToken().tokenCode != IDENT){
            errorFlag = true;
            return -1;
        }
            
        string varName = var();
        //print가 ';'로 끝나지 않을 시 에러
        if(expect(SEMICOLON)==-1) return -1;
        printed = 1;
        printBuffer.push_back(symbolTable[varName]);
    }
    else if(currentToken().tokenCode == IDENT){
        string varName = var(); 
        // '='기호 처리
        if(expect(ASSIGN_OP) == -1) return -1;
        
        //<expr> 처리 
        int res_expr = expr();
        if (errorFlag) return -1;

        // ';'기호 처리
        if(expect(SEMICOLON) == -1) return -1;

        //변수에 값 할당
        symbolTable[varName] = res_expr;
    }
    else{
        errorFlag = true;
        nextToken();
        return -1;
    }
    return 0;
}

/* -----------------------------------------------------------------------------------
 * 설명 : <expr> -> <term> {+ <term> | * <term>}
 * 반환 : expr 값
 * -----------------------------------------------------------------------------------
 */
int expr(void){
    int res_term = term();
    if (errorFlag) return -1;

    //'+' or '*'기호 처리 
    while(!errorFlag && (currentToken().tokenCode == ADD_OP || currentToken().tokenCode == MULT_OP)){
        int op = currentToken().tokenCode;
        nextToken();
        int val = term();
        if(errorFlag) return -1;
        if(op == ADD_OP) res_term += val;
        else if(op == MULT_OP) res_term *= val;        
    }
    return res_term;
}

/* -----------------------------------------------------------------------------------
 * 설명 : <term> -> <factor> {- <factor>}
 * 반환 : term값 
 * -----------------------------------------------------------------------------------
 */
int term(void){
    int res_factor = factor();
    if(errorFlag) return -1;

    // '-'기호 처리
    while(!errorFlag && (currentToken().tokenCode == SUB_OP)){
        nextToken();
        int val = factor();
        if(errorFlag) return -1;

        res_factor -= val;
    }
    return res_factor;
}

/* -----------------------------------------------------------------------------------
 * 설명 : <factor> -> [ - ] ( <number> | <var> | '('<expr>')')
 * 반환 : factor값
 * -----------------------------------------------------------------------------------
 */
int factor(void){
    int sign = 1;

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
        int val = number();
        return sign*val;
    }
    //var
    else if(currentToken().tokenCode == IDENT){
        string varName = var();
        return sign*symbolTable[varName];
    }
    //괄호
    else if(currentToken().tokenCode == LEFT_PAREN){
        nextToken();
        int val = expr();
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
int number(void){
    int val = atoi(currentToken().lexeme);
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
 * 설명 : <statement> -> a | b | c | d | e | f | g | h | i | j | k | l | m | n | o | p | q | r | s | t | u | v | w | x | y | z
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
        lexIndex = 0;
        currentTokenIndex = 0;

        cout << ">> ";
        cin.getline(buffer, MAX_LINE_LEN);

        if (strlen(buffer) == 0) break;
        if (lexer(buffer) != -1){
            program();
            errorFlag = false;
        }
    }
}
