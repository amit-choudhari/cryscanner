
# TODO add support param obj
class Param:
    def __init__(self, ptype, name, value):
        self.ptype = ptype
        self.name = name
        self.value = value

    def __str__(self):
        lines.append(f'name: [{self.name}],'
                'ptype: [{self.ptype}],'
                'value: [{self.value}]')

    def getPtype(self):
        return self.ptype
        pass

    def getname(self):
        return self.name
        pass

    def getValue(self):
        return self.value
        pass

    def setValue(self, value):
        self.value = value
        pass

class Object:
    def __init__(self, fname):
        self.fname = fname 
        self.vars = []

    def __str__(self):
        lines = ''
        lines = (f'{self.fname}()\n')
        for k, v in self.vars.items():
            lines +=  (f"var:{k} = {v}\n")
        return lines


    def getfname(self):
        return self.fname

    def getVarList(self):
        return self.vars

    def getVar(self, name):
        return self.vars[name]

    def addVarList(self, var_dict):
        self.vars = var_dict

    # TODO add support for return value
    def getRet(self):
        pass

    # TODO add support for return value
    def setRet(self):
        pass

