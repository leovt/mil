class Frame:
    def __init__(self, code, parent=None):
        self.code = code[0]
        self.names = code[1]
        self.pc = 0
        self.parent = parent
        self.labels = {instr[1]:pos for pos,instr in enumerate(self.code) if instr[0]=='label'}
        self.stack = {'int':[], 'str':[]}
        self.locals = {'int':{}, 'str':{}}

def run(code):
    frame = Frame(code)
    while True:
        instr = frame.code[frame.pc]
        frame.pc += 1
        if instr[0] == 'label':
            pass
        elif instr[0] == 'load_const':
            frame.stack[instr[1]].append(instr[2])
        elif instr[0] == 'store':
            frame.locals[instr[1]][instr[2]] = frame.stack[instr[1]].pop()
        elif instr[0] == 'load':
            frame.stack[instr[1]].append(frame.locals[instr[1]][instr[2]])
        elif instr[0] == 'jump-if-false':
            if not frame.stack['int'].pop():
                frame.pc = frame.labels[instr[1]]
        elif instr[0] == 'jump':
            frame.pc = frame.labels[instr[1]]
        elif instr[0]=='return':
            if frame.parent:
                typ = frame.names['*return']
                ret = frame.stack[typ].pop()
                frame = frame.parent
                frame.stack[typ].append(ret)
            else:
                break
        elif instr[0] == 'call':
            fn, args, ret, code = frame.names[instr[1]]
            assert fn == 'function'
            if code == 'builtin':
                if instr[1] == 'str':
                    frame.stack['str'].append(str(frame.stack['int'].pop()))
                elif instr[1] == 'writeln':
                    print(frame.stack['str'].pop())
                else:
                    raise Exception('unknown builtin %r' % (instr[1],))
            else:
                caller = frame
                frame = Frame(code, frame)
                for argtype in args:
                    frame.stack[argtype].append(caller.stack[argtype].pop())
        elif instr[0] == 'remainder':
            a = frame.stack['int'].pop()
            b = frame.stack['int'].pop()
            frame.stack['int'].append(b % a)
        elif instr[0] == 'add_int':
            a = frame.stack['int'].pop()
            b = frame.stack['int'].pop()
            frame.stack['int'].append(b + a)
        elif instr[0] == 'equal_int':
            a = frame.stack['int'].pop()
            b = frame.stack['int'].pop()
            frame.stack['int'].append(b == a)
        else:
            raise Exception('Unknown Instruction %r' % (instr,))

def main():
    from lexer import lexer
    from mlparser import parser
    from translate import translate
    from pprint import pprint
    with open('fizzbuzz.mil') as f:
        src = f.read()
    tokens = lexer(src)
    ast = parser(tokens)
    code = translate(ast, ())
    run(code)

if __name__ == '__main__':
    main()