import sys

operators = {
    '+',
    '-',
    '*',
    '/',
    '%',
    '=',
    '==',
}

class LexerState:
    def __init__(self, src):
        self.src = src
        self.line_no = 1
        self.col_no = 1
        self.pos = 0
        self.indentations = [0]
        self.parens = []
    
    @property
    def current(self):
        if self.pos >= len(self.src):
            return None
        return self.src[self.pos]
    
    @property
    def position(self):
        return (self.pos, self.line_no, self.col_no)
        
    def consume(self, consumable, exclude=False):
        cur = self.current
        if exclude:
            test = cur not in consumable
        else:
            test = cur in consumable
            
        if test:
            if cur == '\n':
                self.line_no += 1
                self.col_no = 1
            else:
                self.col_no += 1
            self.pos += 1
            return cur
        return None
            

def lexer(src):
    tokens = []
    
    def emit(*args):
        if args[0] == 'error':
            print('Token: ', *args, file=sys.stderr)
        tokens.append(args)
    
    
    def begin_of_line(state):
        indent = 0
        while state.consume(' '):
            indent += 1
        if indent > state.indentations[-1]:
            state.indentations.append(indent)
            emit('indent', state.position)
        elif indent < state.indentations[-1]:
            while indent < state.indentations[-1]:
                emit('dedent', state.position)
                state.indentations.pop()
            if indent != state.indentations[-1]:
                emit('error', state.position, 'indentation does not match any previous level.')
        return main_line
    
    def main_line(state):
        while state.consume(' '):
            pass
        if state.current == '#':
            return comment
        if state.current in '0123456789':
            return number
        if state.current in 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_':
            return identifier
        if state.current in '+-*/%=:':
            pos = state.position
            op = state.consume('+-*/%=:')
            while op + state.current in operators:
                op += state.consume(state.current)
            if op in operators:
                emit('operator', pos, op)
            else:
                emit('punctuation',pos, op)
            return main_line
        if state.current == '(':
            state.parens.append(('(', state.pos))
            emit('open-paren', state.position)
            state.consume('(')
            return main_line
        if state.current == ')':
            if state.parens and state.parens[-1][0] == '(':
                open_paren = state.parens.pop()
                emit('close-paren', state.position, open_paren[1])
            else:
                emit('error', state.position, 'unmatched )')
            state.consume(')')
            return main_line
        if state.consume('"'):
            return string
        if state.consume('\n'):
            emit('newline', state.position)
            return begin_of_line
        emit('error', state.position, 'unexpected character %r' % state.consume('', True))
        return main_line
    
    def identifier(state):
        pos = state.position
        while state.consume('abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789'):
            pass
        ident = state.src[pos[0]:state.pos]
        if ident in ('def', 'loop', 'break', 'continue', 'if', 'elif', 'else', 'return'):
            emit('keyword', pos, ident)
        else:
            emit('identifier', pos, ident)
        return main_line
    
    def number(state):
        pos = state.position
        while state.consume('0123456789'):
            pass
        emit('number', pos, state.src[pos[0]:state.pos])
        return main_line
                
    def comment(state):
        pos = state.position
        while state.consume('\n', True):
            pass
        emit('comment', pos)
        return main_line
            
    def string(state):
        pos = state.position
        while state.consume('"\n', True):
            pass
        emit('string', pos, state.src[pos[0]:state.pos])
        if not state.consume('"'):
            emit('error', state.position, 'string not terminated by "')
        return main_line

    src_state = LexerState(src)
    lex_state = begin_of_line
    
    while src_state.pos < len(src_state.src):
        lex_state = lex_state(src_state)
    
    while src_state.indentations.pop():
        emit('dedent', src_state.position)
    
    emit('newline', src_state.position)
    
    for (p, pos) in src_state.parens:
        emit('error', pos, 'unmatched ' + p)
        
    emit('end', src_state.position)
        
    return tokens

if __name__ == '__main__':
    with open('fizzbuzz.mil') as f:
        tokens = lexer(f.read())
    for token in tokens:
        print(token)