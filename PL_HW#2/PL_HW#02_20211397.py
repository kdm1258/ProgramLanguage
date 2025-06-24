# Token Code
token_codes = {
    'INT_LIT': 10,
    'IDENT': 11,
    'ASSIGN_OP': 20,
    'ADD_OP': 21,
    'SUB_OP': 22,
    'MULT_OP': 23,
    'LEFT_PAREN': 31,
    'RIGHT_PAREN': 32,
    'SEMICOLON': 33,
    'END': 34,
    'PRINT': 41,
    'EQ_OP': 50,       # ==
    'NEQ_OP': 51,      # !=
    'LT_OP': 52,       # <
    'GT_OP': 53,       # >
    'IF': 60,
    'ELSE': 61,
    'WHILE': 62,
    'DO': 63,
    'LBRACE': 64,      # {
    'RBRACE': 65,      # }
    'TYPE_INT': 66     # integer
}

# Token class
class Token:
    def __init__(self, tokenCode, lexeme):
        self.tokenCode = tokenCode
        self.lexeme = lexeme

# While Loop Context class
class WhileLoopContext:
    def __init__(self, condition_start, body_start, closing_brace, after_loop):
        self.condition_start_index = condition_start
        self.body_start_index = body_start
        self.closing_brace_index = closing_brace
        self.after_loop_index = after_loop

# If Else Context class
class IfElseContext:
    def __init__(self, condition_start, then_block_start, then_block_closing_brace,
                 else_block_start, else_block_closing_brace, after_if_else):
        self.condition_start_index = condition_start
        self.then_block_start_index = then_block_start
        self.then_block_closing_brace_index = then_block_closing_brace
        self.else_block_start_index = else_block_start
        self.else_block_closing_brace_index = else_block_closing_brace
        self.after_if_else_index = after_if_else

# Global declaration
token = []             # 한줄씩 입력받기 버퍼
tokenIndex = 0         # 현재 토큰 인덱스
currentTokenIndex = 0  # parser에서 토큰 추적 index
symbol_table = {}      # 변수 저장용 심볼테이블
printed = False        # 프린트호출여부(개행관리)
errorFlag = False      # 에러여부확인
printBuffer = []       # 프린트 버퍼
whileStack = []        # while 루프 스택
ifElseStack = []       # if-else 스택

'''
-----------------------------------------------------------------------------------
설명 : 토큰타입 구분 함수
매개 : char c
반환 : Token Type
-----------------------------------------------------------------------------------
'''
def findTokenType(c):
    if alphabet(c): 
        return 'LETTER'
    if digit(c): 
        return 'DIGIT'
    return 'UNKNOWN'

'''
====================================================================================
lexer
'''

'''
-----------------------------------------------------------------------------------
설명 : char을 입력으로 받아 해당 symbol이 정의되었는지 확인 하는 함수
매개 : 확인하고자 하는 char c
반환 : 성공 = 해당 symbol의 Token code , 실패 = -1
-----------------------------------------------------------------------------------
'''
def lookup(c):
    return {
        '=': token_codes['ASSIGN_OP'],
        '+': token_codes['ADD_OP'],
        '-': token_codes['SUB_OP'],
        '*': token_codes['MULT_OP'],
        '(': token_codes['LEFT_PAREN'],
        ')': token_codes['RIGHT_PAREN'],
        '{': token_codes['LBRACE'],
        '}': token_codes['RBRACE'],
        ';': token_codes['SEMICOLON'],
        '<': token_codes['LT_OP'],
        '>': token_codes['GT_OP']
    }.get(c, -1)

'''
-----------------------------------------------------------------------------------
설명 : line을 입력으로 받아 tokenize, Token배열에 저장
매개 : 입력받은 line
반환 : 성공 = 0 , 실패 = -1
-----------------------------------------------------------------------------------
'''
def lexer(input_line):
    global token, tokenIndex
    token = []
    tokenIndex = 0
    i = 0
    length = len(input_line)

    while i < length:
        # 공백제거
        while i < length and input_line[i].isspace():
            i += 1
        if i >= length: 
            break

        c = input_line[i]
        
        # == 및 != 처리
        if c == '=' and i + 1 < length and input_line[i + 1] == '=':
            token.append(Token(token_codes['EQ_OP'], '=='))
            i += 2
            continue
        if c == '!' and i + 1 < length and input_line[i + 1] == '=':
            token.append(Token(token_codes['NEQ_OP'], '!='))
            i += 2
            continue
            
        currentType = findTokenType(c)

        # UNKNOWN -> symbol 처리
        if currentType == 'UNKNOWN':
            code = lookup(c)

            # 정의되지 않은 기호
            if code == -1:
                print("Syntax Error!")
                return -1

            # 다음글자가 공백없이 붙으면 에러(++, =+ 등)
            if i + 1 < length and not input_line[i+1].isspace():
                print("Syntax Error!")
                return -1
            token.append(Token(code, c))
            i += 1
        else:
            lexeme = ''
            start_type = currentType
            while i < length and not input_line[i].isspace():
                this_type = findTokenType(input_line[i])
                if this_type != start_type:
                    print("Syntax Error!")
                    return -1
                lexeme += input_line[i]
                i += 1

            # 토큰코드 매핑 
            if currentType == 'LETTER':
                keyword_map = {
                    'print': token_codes['PRINT'],
                    'integer': token_codes['TYPE_INT'],
                    'if': token_codes['IF'],
                    'else': token_codes['ELSE'],
                    'while': token_codes['WHILE'],
                    'do': token_codes['DO']  # 누락된 DO 토큰 추가
                }
                code = keyword_map.get(lexeme, token_codes['IDENT'])
            elif currentType == 'DIGIT':
                code = token_codes['INT_LIT']
            else:
                print("Syntax Error!")
                return -1

            token.append(Token(code, lexeme))

    token.append(Token(token_codes['END'], ''))
    return 0

'''
====================================================================================
parser
'''

'''
-----------------------------------------------------------------------------------
설명 : 현재 토큰을 반환함
반환 : 현재 토큰
-----------------------------------------------------------------------------------
'''
def currentToken():
    return token[currentTokenIndex]

'''
-----------------------------------------------------------------------------------
설명 : tokenIndex 증가
-----------------------------------------------------------------------------------
'''
def nextToken():
    global currentTokenIndex
    currentTokenIndex += 1

'''
-----------------------------------------------------------------------------------
설명 : expected 값과 token Code를 비교
       ADD_OP 처럼 있어도 되고 없어도 되는경우에 사용하기 위해 에러메시지를 출력하는 expect와 분리
반환 : 성공 : true, 실패 : false
-----------------------------------------------------------------------------------
'''
def match(expected):
    if currentToken().tokenCode == expected:
        nextToken()
        return True
    return False

'''
-----------------------------------------------------------------------------------
설명 : expected 값과 token Code를 비교 후 false 시 에러 처리
반환 : 성공 : 0, 실패 : -1
-----------------------------------------------------------------------------------
'''
def expect(expected):
    global errorFlag
    if not match(expected):
        errorFlag = True
        return -1
    return 0

# Grammar rules
'''
-----------------------------------------------------------------------------------
설명 : <program> -> {<declaration>} {<statement>}
-----------------------------------------------------------------------------------
'''
def program():
    global errorFlag, printed, printBuffer, symbol_table
    symbol_table = {}
    errorFlag = False
    printed = False
    
    # 변수 선언 처리
    while currentTokenIndex < len(token) and currentToken().tokenCode == token_codes['TYPE_INT'] and not errorFlag:
        declaration()
    
    # statement 처리
    while currentTokenIndex < len(token) and currentToken().tokenCode != token_codes['END'] and not errorFlag:
        statement()
        
    if errorFlag:
        print("Syntax Error!")
    elif printed:
        if not errorFlag:
            # C++처럼 공백으로 구분하여 출력
            for val in printBuffer:
                print(val, end=' ')
            print()  # 개행
    printBuffer = []
'''
-----------------------------------------------------------------------------------
설명 : <declaration> -> integer <var> ;
반환 : 성공 : 0, 실패 : -1
-----------------------------------------------------------------------------------
'''
def declaration():
    global errorFlag, symbol_table
    
    if typeDeclaration() < 0:
        errorFlag = True
        return -1
        
    varName = var()
    
    if expect(token_codes['SEMICOLON']) == -1:
        errorFlag = True
        return -1
        
    symbol_table[varName] = 0
    return 0

'''
-----------------------------------------------------------------------------------
설명 : <type> -> integer
반환 : 성공 : 0, 실패 : -1
-----------------------------------------------------------------------------------
'''
def typeDeclaration():
    if currentToken().tokenCode == token_codes['TYPE_INT']:
        nextToken()
        return 0
    return -1

def validateBexpr():
    global errorFlag, symbol_table
    
    # 첫 번째 <var> 파싱
    if currentToken().tokenCode != token_codes['IDENT']:
        errorFlag = True
        return False
    # 선언되지 않은 변수인지 확인
    if currentToken().lexeme not in symbol_table:
        errorFlag = True
        return False
    nextToken()  # var 소비
    
    # <relop> 파싱
    op = currentToken().tokenCode
    if op not in [token_codes['EQ_OP'], token_codes['NEQ_OP'], token_codes['LT_OP'], token_codes['GT_OP']]:
        errorFlag = True
        return False
    nextToken()  # relop 소비
    
    # 두 번째 <var> 파싱
    if currentToken().tokenCode != token_codes['IDENT']:
        errorFlag = True
        return False
    # 선언되지 않은 변수인지 확인
    if currentToken().lexeme not in symbol_table:
        errorFlag = True
        return False
    nextToken()  # var 소비
    
    return True

# 3. validateStatement() 함수 - 누락된 함수 구현
def validateStatement():
    global errorFlag
    
    # print
    if currentToken().tokenCode == token_codes['PRINT']:
        nextToken()
        val = aexpr()
        if expect(token_codes['SEMICOLON']) == -1:
            return -1
    elif currentToken().tokenCode == token_codes['IDENT']:
        # 선언된 변수인지 확인
        if currentToken().lexeme not in symbol_table:
            errorFlag = True
            return -1
        nextToken()  # var 소비
        if expect(token_codes['ASSIGN_OP']) == -1:
            return -1
        res_expr = aexpr()
        if errorFlag:
            return -1
        if expect(token_codes['SEMICOLON']) == -1:
            return -1
    elif currentToken().tokenCode == token_codes['WHILE']:
        nextToken()
        if expect(token_codes['LEFT_PAREN']) == -1:
            return -1
        if not validateBexpr():
            return -1
        if expect(token_codes['RIGHT_PAREN']) == -1:
            return -1
        if expect(token_codes['DO']) == -1:
            return -1
        if expect(token_codes['LBRACE']) == -1:
            return -1
        
        # 루프 본문 문법 검사만 수행
        while currentToken().tokenCode != token_codes['RBRACE'] and not errorFlag and currentToken().tokenCode != token_codes['END']:
            if validateStatement() == -1:
                return -1
        
        if expect(token_codes['RBRACE']) == -1:
            return -1
        if expect(token_codes['SEMICOLON']) == -1:
            return -1
    elif currentToken().tokenCode == token_codes['IF']:
        nextToken()
        if expect(token_codes['LEFT_PAREN']) == -1:
            return -1
        if not validateBexpr():
            return -1
        if expect(token_codes['RIGHT_PAREN']) == -1:
            return -1
        if expect(token_codes['LBRACE']) == -1:
            return -1
        
        # then 블록 문법 검사
        while currentToken().tokenCode != token_codes['RBRACE'] and not errorFlag and currentToken().tokenCode != token_codes['END']:
            if validateStatement() == -1:
                return -1
        
        if expect(token_codes['RBRACE']) == -1:
            return -1
        if expect(token_codes['ELSE']) == -1:
            return -1
        if expect(token_codes['LBRACE']) == -1:
            return -1
        
        # else 블록 문법 검사
        while currentToken().tokenCode != token_codes['RBRACE'] and not errorFlag and currentToken().tokenCode != token_codes['END']:
            if validateStatement() == -1:
                return -1
        
        if expect(token_codes['RBRACE']) == -1:
            return -1
        if expect(token_codes['SEMICOLON']) == -1:
            return -1
    else:
        errorFlag = True
        nextToken()
        return -1
    
    return 0


'''
-----------------------------------------------------------------------------------
설명 : <statement> -> <var> = <aexpr> ; | print <aexpr> ; | 
                      while ( <bexpr> ) do { <statement>* } ; |
                      if ( <bexpr> ) { <statement>* } else { <statement>* } ;
반환 : 성공 :0, 실패 : -1
전역변수 errorFlag로 에러여부 판단으로 수정함
-----------------------------------------------------------------------------------
'''
def statement():
    global errorFlag, printed, printBuffer, symbol_table, currentTokenIndex  

    # print 문 처리
    if currentToken().tokenCode == token_codes['PRINT']:
        nextToken()
        val = aexpr()
        if expect(token_codes['SEMICOLON']) == -1:
            return -1
        printed = True
        printBuffer.append(val)
    
    # 변수 할당문 처리
    elif currentToken().tokenCode == token_codes['IDENT']:
        # 선언된 변수인지 확인
        if currentToken().lexeme not in symbol_table:
            errorFlag = True
            return -1
        varName = var()
        if expect(token_codes['ASSIGN_OP']) == -1:
            return -1
        res_expr = aexpr()
        if errorFlag:
            return -1
        if expect(token_codes['SEMICOLON']) == -1:
            return -1
        symbol_table[varName] = res_expr
    
    # while 문 처리 (C++과 동일한 두 단계 처리)
    elif currentToken().tokenCode == token_codes['WHILE']:
        nextToken()
        
        # 1단계: 문법 검사
        savedTokenIndex = currentTokenIndex
        
        if expect(token_codes['LEFT_PAREN']) == -1:
            return -1
        conditionStartIndex = currentTokenIndex
        
        if not validateBexpr():
            return -1
        if expect(token_codes['RIGHT_PAREN']) == -1:
            return -1
        if expect(token_codes['DO']) == -1:
            return -1
        if expect(token_codes['LBRACE']) == -1:
            return -1
        
        bodyStartIndex = currentTokenIndex
        
        # 본문 문법 검사
        while currentToken().tokenCode != token_codes['RBRACE'] and not errorFlag and currentToken().tokenCode != token_codes['END']:
            if validateStatement() == -1:
                return -1
        
        if errorFlag:
            return -1
            
        closingBraceIndex = currentTokenIndex
        if expect(token_codes['RBRACE']) == -1:
            return -1
        if expect(token_codes['SEMICOLON']) == -1:
            return -1
        afterLoopIndex = currentTokenIndex
        
        # 2단계: 실제 실행
        currentTokenIndex = savedTokenIndex
        if expect(token_codes['LEFT_PAREN']) == -1:
            return -1
        
        # 실제 while 루프 실행
        while True:
            beforeConditionIndex = currentTokenIndex
            cond = bexpr()
            
            if errorFlag:
                return -1
            if not cond:
                break
            
            # ')' 'do' '{' 건너뛰기
            if expect(token_codes['RIGHT_PAREN']) == -1:
                return -1
            if expect(token_codes['DO']) == -1:
                return -1
            if expect(token_codes['LBRACE']) == -1:
                return -1
            
            # 본문 실행
            while currentToken().tokenCode != token_codes['RBRACE'] and not errorFlag and currentToken().tokenCode != token_codes['END']:
                if statement() == -1:
                    return -1
            if errorFlag:
                return -1
            
            if expect(token_codes['RBRACE']) == -1:
                return -1
            
            # 다음 반복을 위해 조건문으로 되돌아감
            currentTokenIndex = beforeConditionIndex
        
        # 루프 종료 후 전체 while문 이후로 이동
        currentTokenIndex = afterLoopIndex
        return 0
    
    # if-else 문 처리 (C++과 동일한 두 단계 처리)
    elif currentToken().tokenCode == token_codes['IF']:
        nextToken()
        
        # 1단계: 문법 검사
        savedTokenIndex = currentTokenIndex
        
        if expect(token_codes['LEFT_PAREN']) == -1:
            return -1
        conditionStartIndex = currentTokenIndex
        
        if not validateBexpr():
            return -1
        if expect(token_codes['RIGHT_PAREN']) == -1:
            return -1
        if expect(token_codes['LBRACE']) == -1:
            return -1
        thenBlockStartIndex = currentTokenIndex
        
        # then 블록 문법 검사
        while currentToken().tokenCode != token_codes['RBRACE'] and not errorFlag and currentToken().tokenCode != token_codes['END']:
            if validateStatement() == -1:
                return -1
        if errorFlag:
            return -1
        
        thenBlockClosingBraceIndex = currentTokenIndex
        if expect(token_codes['RBRACE']) == -1:
            return -1
        if expect(token_codes['ELSE']) == -1:
            return -1
        if expect(token_codes['LBRACE']) == -1:
            return -1
        elseBlockStartIndex = currentTokenIndex
        
        # else 블록 문법 검사
        while currentToken().tokenCode != token_codes['RBRACE'] and not errorFlag and currentToken().tokenCode != token_codes['END']:
            if validateStatement() == -1:
                return -1
        if errorFlag:
            return -1
        
        elseBlockClosingBraceIndex = currentTokenIndex
        if expect(token_codes['RBRACE']) == -1:
            return -1
        if expect(token_codes['SEMICOLON']) == -1:
            return -1
        afterIfElseIndex = currentTokenIndex
        
        # 2단계: 실제 실행
        currentTokenIndex = savedTokenIndex
        if expect(token_codes['LEFT_PAREN']) == -1:
            return -1
        
        # 조건 평가
        cond = bexpr()
        if errorFlag:
            return -1
        
        if expect(token_codes['RIGHT_PAREN']) == -1:
            return -1
        if expect(token_codes['LBRACE']) == -1:
            return -1
        
        if cond:  # 조건이 true: then 블록 실행
            while currentToken().tokenCode != token_codes['RBRACE'] and not errorFlag and currentToken().tokenCode != token_codes['END']:
                if statement() == -1:
                    return -1
            if expect(token_codes['RBRACE']) == -1:
                return -1
            
            # else 블록은 건너뛰기
            if expect(token_codes['ELSE']) == -1:
                return -1
            if expect(token_codes['LBRACE']) == -1:
                return -1
            
            # else 블록의 내용을 건너뛰기 위해 중괄호 매칭
            braceCount = 1
            while braceCount > 0 and currentToken().tokenCode != token_codes['END']:
                if currentToken().tokenCode == token_codes['LBRACE']:
                    braceCount += 1
                elif currentToken().tokenCode == token_codes['RBRACE']:
                    braceCount -= 1
                nextToken()
        else:  # 조건이 false: then 블록 건너뛰고 else 블록 실행
            # then 블록 건너뛰기
            braceCount = 1
            while braceCount > 0 and currentToken().tokenCode != token_codes['END']:
                if currentToken().tokenCode == token_codes['LBRACE']:
                    braceCount += 1
                elif currentToken().tokenCode == token_codes['RBRACE']:
                    braceCount -= 1
                nextToken()
            
            if expect(token_codes['ELSE']) == -1:
                return -1
            if expect(token_codes['LBRACE']) == -1:
                return -1
            
            # else 블록 실행
            while currentToken().tokenCode != token_codes['RBRACE'] and not errorFlag and currentToken().tokenCode != token_codes['END']:
                if statement() == -1:
                    return -1
            if expect(token_codes['RBRACE']) == -1:
                return -1
        
        if expect(token_codes['SEMICOLON']) == -1:
            return -1
        return 0
    
    else:
        errorFlag = True
        nextToken()
        return -1
    
    return 0

'''
-----------------------------------------------------------------------------------
설명 : <bexpr> -> <var> <relop> <var>
반환 : boolean 결과
-----------------------------------------------------------------------------------
'''
def bexpr():
    global errorFlag, symbol_table
    
    # 첫 번째 변수
    if currentToken().lexeme not in symbol_table:
        errorFlag = True
        return False
        
    left = var()
    
    # 관계 연산자
    op = relop()
    if op == -1:
        errorFlag = True
        return False
    nextToken()
    
    # 두 번째 변수
    if currentToken().lexeme not in symbol_table:
        errorFlag = True
        return False
        
    right = var()
    
    if errorFlag:
        return False
        
    lval = symbol_table[left]
    rval = symbol_table[right]
    
    # 관계 연산 수행
    if op == token_codes['EQ_OP']:
        return lval == rval
    elif op == token_codes['NEQ_OP']:
        return lval != rval
    elif op == token_codes['LT_OP']:
        return lval < rval
    elif op == token_codes['GT_OP']:
        return lval > rval
    else:
        errorFlag = True
        return False

'''
-----------------------------------------------------------------------------------
설명 : <relop> -> == | != | < | >
반환 : 연산자 토큰 코드
-----------------------------------------------------------------------------------
'''
def relop():
    global errorFlag
    op = currentToken().tokenCode
    if op not in [token_codes['EQ_OP'], token_codes['NEQ_OP'], token_codes['LT_OP'], token_codes['GT_OP']]:
        errorFlag = True
        return -1
    return op

'''
-----------------------------------------------------------------------------------
설명 : <aexpr> -> <term> {+ <term> | - <term>}
반환 : aexpr 값 실패시 -1
-----------------------------------------------------------------------------------
'''
def aexpr():
    res_term = term()
    if errorFlag:
        return -1
    while not errorFlag and currentToken().tokenCode in [token_codes['ADD_OP'], token_codes['SUB_OP']]:
        op = currentToken().tokenCode
        nextToken()
        val = term()
        if errorFlag: 
            return -1
        # ADD_OP이면 더하기
        if op == token_codes['ADD_OP']:
            res_term += val
        # SUB_OP이면 빼기
        elif op == token_codes['SUB_OP']:
            res_term -= val
    return res_term

'''
-----------------------------------------------------------------------------------
설명 : <term> -> <factor> {* <factor>}
반환 : term값 
-----------------------------------------------------------------------------------
'''
def term():
    res_factor = factor()
    if errorFlag:
        return -1

    # * 기호 처리
    while not errorFlag and currentToken().tokenCode == token_codes['MULT_OP']:
        nextToken()
        val = factor()
        if errorFlag:
            return -1
        res_factor *= val
    return res_factor

'''
-----------------------------------------------------------------------------------
설명 : <factor> -> [ - ] ( <number> | <var> | '('<aexpr>')')
       [-]처리를 위해 sign 변수를 선언해 곱해줌
반환 : factor값 실패시 -1
-----------------------------------------------------------------------------------
'''
def factor():
    global errorFlag, symbol_table
    sign = 1
    if currentToken().tokenCode == token_codes['SUB_OP']:
        sign = -1
        nextToken()

        # 연속된 - 에러 처리 (ex) - - )
        if currentToken().tokenCode == token_codes['SUB_OP']:
            errorFlag = True
            nextToken()
            return -1

    # number처리
    if currentToken().tokenCode == token_codes['INT_LIT']:
        return sign * number()

    # var 처리
    elif currentToken().tokenCode == token_codes['IDENT']:
        if currentToken().lexeme not in symbol_table:
            errorFlag = True
            return -1
        varName = var()
        return sign * symbol_table[varName]

    # 괄호 처리
    elif currentToken().tokenCode == token_codes['LEFT_PAREN']:
        nextToken()
        val = aexpr()
        if expect(token_codes['RIGHT_PAREN']) == -1:
            nextToken()
            return -1
        return sign * val
    else:
        errorFlag = True
        nextToken()
        return -1

'''
-----------------------------------------------------------------------------------
설명 : <number> -> <digit> {<digit>}
-----------------------------------------------------------------------------------
'''
def number():
    val = int(currentToken().lexeme)
    nextToken()
    return val

'''
-----------------------------------------------------------------------------------
설명 : <var> -> <alphabet>{<alphabet>}
-----------------------------------------------------------------------------------
'''
def var():
    global errorFlag
    if currentToken().tokenCode != token_codes['IDENT']:
        errorFlag = True
        return ""
    varName = currentToken().lexeme
    nextToken()
    return varName

'''
-----------------------------------------------------------------------------------
설명 : <alphabet> -> a | b | c | d | e | f | g | h | i | j | k | l | m | n | o | p | q | r | s | t | u | v | w | x | y | z
       대문자는 허용하지 않음
-----------------------------------------------------------------------------------
'''
def alphabet(c):
    al = ('a','b','c','d','e','f','g','h',
        'i','j','k','l','m','n','o','p',
        'q','r','s','t','u','v','w','x','y','z')
    if c in al:
        return True
    return False

'''
-----------------------------------------------------------------------------------
설명 : <digit> 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 
-----------------------------------------------------------------------------------
'''
def digit(c):
    digits = ('0', '1', '2', '3', '4', '5', '6', '7', '8', '9')
    if c in digits:
        return True
    return False

# Main loop
while True:
    try:
        line = input(">> ")
        if not line.strip():  # 아무것도 입력하지 않고 엔터를 치면 종료
            break
        currentTokenIndex = 0
        if lexer(line) != -1:
            program()
    except EOFError:
        break
    except KeyboardInterrupt:
        break