from pyparsing import *
from object import Object 

class parseLogs:
    def __init__(self):
        self.m_obj = []
        pass

    def __str__(self):
        pass
    
    # All Variables are finally parsed here and added to the object
    def __parseVariables(self, fname, pvar, prefix=''):
        #print(pvar)
        var_t = Word(printables) + Suppress(Literal('=')) + OneOrMore(Word(printables))
        var = var_t.parseString(pvar)
        dot = None

        #print(var[0], var[1])
        dot = f'.' if prefix else f''
        k_var = f'{prefix}{dot}{var[0]}'
        d_var = {k_var:var[1]}
        # Update the varaible list of object
        x = self.m_obj[-1]
        #for x in self.m_obj:
        if x.getfname() == fname:
            x.addVar(d_var)
            #print(x.getVar(k_var))
            #print(k_var)
        else:
            print("[ERROR]: OUT OF ORDER")

    def __parseArray(self, fname, v, prefix):
        LPAR,RPAR = map(Suppress, "{}")
        value = (originalTextFor(OneOrMore(Word(printables, excludeChars="{},"))))
        aname_t = Word(printables) + Suppress(Literal('='))
        arr_t = Suppress(aname_t) + LPAR + delimitedList(value) + RPAR
        aname = aname_t.parseString(v)[0]
        avalue = arr_t.parseString(v)
        val = '0x' + ''.join([format(int(c, 0), '02X') for c in reversed(avalue)])
        arr_str = aname + ' = ' + val
        self.__parseVariables(fname, arr_str, prefix)



    def __flattenStruct(self, sname_t, struct_var):
        # extract struct elements (except nested structs)
        LPAR,RPAR = map(Suppress, "{}")
        value = (originalTextFor(OneOrMore(Word(printables, excludeChars="{},") | nestedExpr('{','}'))))
        expr = Suppress(sname_t + SkipTo(Literal('{'))) + LPAR + delimitedList(value) + RPAR
        vlist = expr.parseString(struct_var).asList()
        return vlist

    def __parseStruct(self, fname, struct_var, prefix = ''):
        #print(struct_var[0])
        # extract struct name
        sname_t = Word(printables) + (Suppress(Literal('=')) | Suppress(LineEnd()))
        sname = sname_t.parseString(struct_var)[0]
        #print(sname)

        try:
            vlist = self.__flattenStruct(sname_t, struct_var)
        except Exception as e:
            print(e)
            return

        nested = False
        dot = f'.' if prefix else f''
        prefix = f'{prefix}{dot}{sname}'
        # insert all extracted variables in Object
        # recursively find structures
        braces = ['{', '}']
        for v in vlist:
            if '=' not in v:
                continue
            # TODO add support for Arrays of struct
            if '{{' in v:
                continue
            # Parse array of ints
            if re.match(r'(.*){(.*)}',v):
                self.__parseArray(fname, v, prefix)
            # Parse nested struct
            elif '{' in v:
                self.__parseStruct(fname, v, prefix)
            else:
                self.__parseVariables(fname, v, prefix)
            pass

        '''
        svar = svars_t.parseString(struct_var[0])[1]
        print(svar, type(svar))
        for k in svar:
            pass
        '''

    def __parseObjects(self, obj):
        # For every object fname
        # print(obj)
        fname_t = Word(printables) + Suppress(Literal('('))
        fname = fname_t.parseString(obj[0])[0]
        self.m_obj.append(Object(fname))
        
        sname_marker = LineStart() + Literal('struct')
        pname_marker = LineStart() + Literal('ptype')
        array_marker = LineStart() + Literal('array')
        markers = sname_marker ^ pname_marker ^ array_marker

        
        sname_section = Group(
                    Suppress(sname_marker) + SkipTo(markers | stringEnd).setResultsName('sname')
                    ).setResultsName('snames', listAllMatches=True)
        pname_section = Group(
                    Suppress(pname_marker + Word(printables)) + SkipTo(markers | stringEnd).setResultsName('pname')
                    ).setResultsName('pnames', listAllMatches=True)
        aname_section = Group(
                    Suppress(array_marker)+ Word(printables) + Suppress(Word(printables))+ SkipTo(markers | stringEnd).setResultsName('aname')
                    ).setResultsName('anames', listAllMatches=True)

        sections = sname_section ^ pname_section ^ aname_section
        text = StringStart() + SkipTo(sections | StringEnd())
        doc = Optional(text) + ZeroOrMore(sections)
        svar = doc.parseString(obj[0])
        print(f"object: {fname}()")
        if 'struct' in obj[0]:
            for s in svar.snames:
                self.__parseStruct(fname, s.asList()[0])
        if 'ptype' in obj[0]:
            for p in svar.pnames:
                self.__parseVariables(fname, p.asList()[0])
        if 'array' in obj[0]:
            i =0
            #print("####",svar.anames)
            st =[]
            for item in svar.anames:
                st.append(item[0]+item[1])
            #st = svar.anames[0].asList()[0]+svar.anames[0].asList()[1]
            for arr in st:
                self.__parseArray(fname, arr, '')

        pass

    def parse(self, fileName):
        # Parse objects on keyword #0
        object_marker = LineStart() + Literal('#0')
        object_section = Group(
                    Suppress(object_marker) + SkipTo(object_marker | stringEnd).setResultsName('object')
                    ).setResultsName('objects', listAllMatches=True)

        #text = StringStart() + SkipTo(StringEnd())
        doc = ZeroOrMore(object_section)
        result = doc.parseFile(fileName)
        #print(result.objects)
        for i in result.objects:
            #print(i)
            self.__parseObjects(i)
        #p = result.objects[0].asList()
        #print("### P:",p[0], type(p[0]))
        pass

    def getObjects(self):
        return self.m_obj

    def getCallFlow(self):
        pass
