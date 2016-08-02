from itertools import count
from pprint import pprint

builtins = {'str': ('function', ['int'], 'str', 'builtin'),
            'writeln': ('function', ['str'], 'null', 'builtin')}


def translate(ast, arguments):
    code = []
    
    names = {}
    names.update(builtins)
    
    break_labels = []
    
    def emit(*args):
        code.append(args)
    labels = ('lbl%d' % i for i in count())
    varids = ('var%d' % i for i in count()) 
    
    def get_op(operator, ltype, rtype):
        if operator == '==':
            if ltype == rtype == 'int':
                return 'equal_int', 'int'
        if operator == '%':
            if ltype == rtype == 'int':
                return 'remainder', 'int'
        if operator == '=':
            raise Exception('can not assign in an expression')
        if operator == '+':
            if ltype == rtype == 'int':
                return 'add_int', 'int'            
        else:
            raise Exception('illegal operation %s for types %s and %s' % (operator, ltype, rtype))

    def trans_expr(node):
        if node[0] == 'number':
            code = [('load_const', 'int', int(node[2]))]
            type = 'int'
        elif node[0] == 'string':
            code = [('load_const', 'str', node[2])]
            type = 'str'
        elif node[0] == 'identifier':
            try:
                type, vid, loc = names[node[2]]
            except KeyError:
                pprint(names)
                pprint(node)
                raise Exception('Undefined variable %s' % node[2])
            code = [('load', type, vid)]      
        elif node[0] == 'operator':
            lhs, ltype = trans_expr(node[2])
            rhs, rtype = trans_expr(node[3])
            operation, type = get_op(node[1][2], ltype, rtype)
            code = lhs + rhs + [(operation,)]
        elif node[0] == 'call':
            function = names[node[1][2]]
            if len(node[2]) != len(function[1]):
                raise Exception('number of arguments wrong')
            code = []
            for arg, extype in zip(node[2], function[1]):
                acode, type = trans_expr(arg)
                if type != extype:
                    raise Exception('wrong type of argument')
                code.extend(acode)
            code.append(('call', node[1][2]))
            type = function[2]
        else:
            assert False, 'Unknown node %s in expression' % (node,)
        return (code, type)

    def trans(node):
        if node[0] == 'cond':
            else_label = next(labels)
            end_label = next(labels)
            trans(node[1])
            emit('jump-if-false', else_label)
            trans_block(node[2])
            if len(node)>3:
                emit('jump', end_label)
                emit('label', else_label)
                trans_block(node[3])
                emit('label', end_label)
            else:
                emit('label', else_label)
            
        elif node[0] == 'fn':
            arguments = [(nnode[2], tnode[2]) for (nnode, tnode) in node[2]]
            function = translate(node[3], arguments)
            names[node[1][2]] = ('function', 
                                 [arg[1] for arg in arguments],
                                 function[1]['*return'],
                                 function
                                 )
            
        elif node[0] == 'expr':
            code, type = trans_expr(node[1])
            for instr in code:
                emit(*instr)
                
        elif node[0] == 'expr-stmt':
            if node[1][1][0] == 'operator' and node[1][1][1][2] == '=':
                lhs_node = node[1][1][2]
                rhs_node = node[1][1][3]
                code, type = trans_expr(rhs_node)
                if lhs_node[0] != 'identifier':
                    raise Exception('can only assign to identifiers')
                name = lhs_node[2]
                if name in names and names[name][0] != type:
                    raise Exception('redefining %s with another type' % name)
                elif name not in names:
                    names[name] = (type, next(varids), lhs_node[1])
                
                for instr in code:
                    emit(*instr)
                emit('store', type, names[name][1])
            else:
                trans(node[1])
        
        elif node[0] == 'loop':
            label = next(labels)
            end_label = next(labels)
            emit('label', label)
            break_labels.append(end_label)
            trans_block(node[1])
            popped = break_labels.pop()
            assert popped == end_label
            emit('jump', label)
            emit('label', end_label)
        
        elif node[0] == 'return':
            assert node[1][0] == 'expr'
            code, type = trans_expr(node[1][1])
            if '*return' in names:
                if type != names['*return']:
                    raise Exception('ambiguous return type')
            else:
                names['*return'] = type
            for instr in code:
                emit(*instr)
            emit('return')
            
        elif node[0] == 'break':
            emit('jump', break_labels[-1])
        
        else:
            assert False, 'Unknown node %s' % (node,)
            
    def trans_block(block):
        for node in block:
            trans(node)
    
    for name, type in reversed(arguments):
        vid = next(varids)
        names[name] = (type, vid, None)
        emit('store', type, vid)       
    trans_block(ast)
    
    return code, names             

def main():
    from lexer import lexer
    from mlparser import parser
    from pprint import pprint
    with open('fizzbuzz.mil') as f:
        src = f.read()
    tokens = lexer(src)
    ast = parser(tokens)
    pprint(ast)
    pprint(translate(ast, ()))
    
if __name__ == '__main__':
    import cProfile
    #cProfile.run('main()')
    main()