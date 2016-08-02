from pprint import pprint
from pip._vendor.distlib._backport.tarfile import BLKTYPE
def interesting_tokens(tokens):
    for tok in tokens:
        if tok[0] == 'comment':
            pass
        elif tok[0] == 'error':
            raise Exception(tok[2])
        else:
            yield tok

def debug_symbol(func):
    def wrapper():
        print('start', func.__name__)
        ret = func()
        print('end', func.__name__, '->', ret)
        return ret
    return wrapper

def parser(tokens):
    
    iter = interesting_tokens(tokens)
    current = next(iter)
    
    def consume(tok, keywd=None):
        nonlocal current
        if current[0] == tok and (keywd is None or current[2] == keywd):
            ret = current
            print('consuming token', current)
            current = next(iter, None)
            return ret
        else:
            raise Exception('Unexpected Token' + str(current))
    
    @debug_symbol
    def statement():
        if current[0] == 'keyword' and current[2] == 'def':
            return function_definition()
        if current[0] == 'keyword' and current[2] == 'if':
            return conditional()
        if current[0] == 'keyword' and current[2] == 'return':
            return return_stmt()
        if current[0] == 'keyword' and current[2] == 'loop':
            return loop()
        if current[0] == 'keyword' and current[2] == 'break':
            consume('keyword', 'break')
            consume('newline')
            return ('break',)
        return expression_stmt()
            #raise Exception('unexpected token: ' + str( current))
    
    @debug_symbol
    def expression_stmt():
        expr = expression()
        consume('newline')
        return ('expr-stmt', expr)
    
    @debug_symbol
    def loop():
        consume('keyword', 'loop')
        consume('punctuation', ':')
        consume('newline')
        consume('indent')
        blk = block()
        consume('dedent')
        return ('loop', blk)
    
    @debug_symbol
    def expression():
        expr = []
        ops = []
        prec = {'=':0, '==':10, '+':20, '-':20, '*':30, '/':30, '%':30}
        assoc = {'=':0, '==':10, '+':'L', '-':'L', '*':'L', '/':'L', '%':'L'}
        
        def must_pop_op(cur, tos):
            if assoc[cur[2]] == 'L':
                return prec[cur[2]] <= prec[tos[2]]
            else:
                return prec[cur[2]] < prec[tos[2]]
        
        prefix = True
        while current[2:3] != (':',) and current[0] != 'newline' and current[0] != 'end':
            if current[0] in ('number', 'identifier', 'string'):
                expr.append(current)
                prefix = False
            elif current[0] == 'operator':
                while ops and must_pop_op(current, ops[-1]):
                    rhs = expr.pop()
                    lhs = expr.pop()
                    node = ('operator', ops.pop(), lhs, rhs)
                    expr.append(node)
                ops.append(current)
                prefix = True
            elif current[0] == 'open-paren':
                if prefix:
                    raise NotImplementedError
                else:
                    # function call
                    ops.append(('call', []))
            elif current[0] == 'close-paren':
                while ops and ops[-1][0] != 'call':
                    rhs = expr.pop()
                    lhs = expr.pop()
                    node = ('operator', ops.pop(), lhs, rhs)
                    expr.append(node)
                assert ops[-1][0] == 'call'
                ops[-1][1].append(expr.pop())
                _, args = ops.pop()
                func = expr.pop()
                expr.append(('call', func, args))
            else:
                raise Exception('unexpected token in expression %r' % (current,))
            prev = consume(current[0])
        while ops:
            rhs = expr.pop()
            lhs = expr.pop()
            node = ('operator', ops.pop(), lhs, rhs)
            expr.append(node)
        if len(expr) != 1:
            raise Exception('Expression malformed')
        return ('expr', expr.pop())
    
    @debug_symbol
    def conditional():
        consume('keyword', 'if')
        condition = expression()
        consume('punctuation', ':')
        consume('newline')
        consume('indent')
        true_block = block()
        consume('dedent')
        #if current[0]=='keyword' and current[2] == 'else':
        
        return ('cond', condition, true_block)   
        
    @debug_symbol
    def return_stmt():
        consume('keyword', 'return')
        expr = expression()
        consume('newline')
        return ('return', expr)
        
        
    @debug_symbol
    def function_definition():
        consume('keyword', 'def')
        name = consume('identifier')
        consume('open-paren')
        arg = consume('identifier')
        consume('punctuation', ':')
        argtype = consume('identifier')
        consume('close-paren')
        consume('punctuation', ':')
        consume('newline')
        consume('indent')
        body = block()
        consume('dedent')
        return ('fn', name, [(arg,argtype)], body)
    
    @debug_symbol
    def block():
        statements = []
        while current[0] != 'dedent' and current[0] != 'end':
            if current[0] == 'newline':
                consume('newline')
            else:
                statements.append(statement())
        return statements
    
    @debug_symbol
    def module():
        blk = block()
        consume('end')
        return blk
    
    return module()

def main():
    from lexer import lexer
    with open('fizzbuzz.mil') as f:
        src = f.read()
    tokens = lexer(src)
    pprint(parser(tokens))
    
if __name__ == '__main__':
    import cProfile
    #cProfile.run('main()')
    main()