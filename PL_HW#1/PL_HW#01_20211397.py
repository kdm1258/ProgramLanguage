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
    'PRINT': 41
}

# Token class
class Token:
    def __init__(self, tokenCode, lexeme):
        self.tokenCode = tokenCode
        self.lexeme = lexeme


#Global declararion
token = []             #한줄씩 입력받기 버퍼
tokenIndex = 0         #현재 토큰 인덱스
currentTokenIndex = 0  #parser에서 토큰 추적 index
symbol_table = {}      #변수 저장용 심볼테이블
printed = False        #프린트호출여부(개행관리)
errorFlag = False      #에러여부확인
printBuffer = []       #프린트 버퍼퍼

'''
-----------------------------------------------------------------------------------
설명 : 토큰타입 구분 함수
매계 : char c
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
매계 : 확인하고자 하는 char c
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
        ';': token_codes['SEMICOLON']
    }.get(c, -1)


'''
-----------------------------------------------------------------------------------
설명 : line을 입력으로 받아 tokenize, Token배열에 저장
매계 : 입력받은 line
반환 : 성공 = 0 , 실패 = -1
-----------------------------------------------------------------------------------
'''
def lexer(input_line):
    global token, tokenIndex
    token.clear()   #입력받을 버퍼(token) 초기화
    tokenIndex = 0
    i = 0
    length = len(input_line)

    while i < length:
        #공백제거
        while i < length and input_line[i].isspace():
            i += 1
        if i >= length: 
            break

        c = input_line[i]
        currentType = findTokenType(c)

        # UNKNOWN -> symbol 처리
        if currentType == 'UNKNOWN':
            code = lookup(c)

            #정의되지 않은 기호
            if code == -1:
                print("Syntax Error!")
                return -1

            #다음글자가 공백없이 붙으면 에러(++, =+ 등)
            if i + 1 < length and not input_line[i+1].isspace():
                print("Syntax Error!")
                return -1
            token.append(Token(code, c))
            i += 1
        else:
            lexeme = ''
            while i < length and findTokenType(input_line[i]) == currentType:
                lexeme += input_line[i]
                i += 1

        #토큰코드 매핑 
            #LETTER일 때 print 따로 처리
            if currentType == 'LETTER':
                code = token_codes['PRINT'] if lexeme == 'print' else token_codes['IDENT']
            #DIGIT는 INT_LIT 토큰
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
----------------------------------------------------------------------------------
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
       ADD_OP 처럼 있어도 되도 없어도 되는경우에 사용하기 위해 에러메시지를 출력하는 expect와 분리
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
설명 : <program> -> {<statement>}
-----------------------------------------------------------------------------------
'''
def program():
    global errorFlag, printed, printBuffer, symbol_table
    symbol_table.clear()
    errorFlag = False
    printed = False
    
    #입력인 END가 아니고 에러가 없다면 statement()호출
    while currentTokenIndex < len(token) and currentToken().tokenCode != token_codes['END'] and not errorFlag:
        statement()
    if errorFlag:
        print("Syntax Error!")
    elif printed:
        if not errorFlag:
            #버퍼에 있는 값 합쳐서 출력
            print(' '.join(map(str, printBuffer)))
    printBuffer.clear()

'''
-----------------------------------------------------------------------------------
설명 : <statement> -> <var> = <expr> ; | print <var> ;
반환 : 성공 :0, 실패 : -1
전역변수 errorFlag로 에러여부 판단으로 수정함
-----------------------------------------------------------------------------------
'''
def statement():
    global printed, errorFlag

    #print
    if currentToken().tokenCode == token_codes['PRINT']:
        nextToken()
        #print 뒤에 <var>이 아닌경우 error
        if currentToken().tokenCode != token_codes['IDENT']:
            errorFlag = True
            return -1
        varName = var()
        #print가 ;로 끝나지 않으면 error
        if expect(token_codes['SEMICOLON']) == -1: 
            return -1
        printed = True
        printBuffer.append(symbol_table.get(varName, 0))
        
    # <var> = <expr> ;
    elif currentToken().tokenCode == token_codes['IDENT']:
        varName = var()
        # '='기호 처리
        if expect(token_codes['ASSIGN_OP']) == -1: 
            return -1
        #<expr> 호출
        val = expr()
        if errorFlag: 
            return -1

        # ; 처리
        if expect(token_codes['SEMICOLON']) == -1: 
            return -1

        # 심볼테이블에 변수 값 할당
        symbol_table[varName] = val
    else:
        errorFlag = True
        nextToken()
        return -1
    return 0

'''
-----------------------------------------------------------------------------------
설명 : <expr> -> <term> {+ <term> | * <term>}
반환 : expr 값 실패시 -1
-----------------------------------------------------------------------------------
'''
def expr():
    res_term = term()
    if errorFlag:
        return -1
    while not errorFlag and currentToken().tokenCode in [token_codes['ADD_OP'], token_codes['MULT_OP']]:
        op = currentToken().tokenCode
        nextToken()
        val = term()
        if errorFlag: 
            return -1
        #ADD_OP이면 더하기
        if op == token_codes['ADD_OP']:
            res_term += val
        #MULT_OP이면 곱하기
        elif op == token_codes['MULT_OP']:
            res_term *= val
    return res_term

'''
-----------------------------------------------------------------------------------
설명 : <term> -> <factor> {- <factor>}
반환 : term값 
-----------------------------------------------------------------------------------
'''
def term():
    res_factor = factor()
    if errorFlag:
        return -1

    # - 기호 처리
    while not errorFlag and currentToken().tokenCode == token_codes['SUB_OP']:
        nextToken()
        val = factor()
        if errorFlag:
            return -1
        res_factor -= val
    return res_factor

'''
-----------------------------------------------------------------------------------
설명 : <factor> -> [ - ] ( <number> | <var> | '('<expr>')')
       [-]처리를 위해 sign 변수를 선언해 곱해줌
반환 : factor값 실패시 -1
-----------------------------------------------------------------------------------
'''
def factor():
    global errorFlag
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
        return sign * symbol_table.get(var(), 0)

    # 괄호 처리
    elif currentToken().tokenCode == token_codes['LEFT_PAREN']:
        nextToken()
        val = expr()
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
설명 : <statement> -> a | b | c | d | e | f | g | h | i | j | k | l | m | n | o | p | q | r | s | t | u | v | w | x | y | z
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
        if not line.strip():
            break
        currentTokenIndex = 0
        if lexer(line) != -1:
            program()
    except EOFError:
        break
