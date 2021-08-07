from object import Object 
import sympy

class Constraint:
    def __init__(self, operation, obj, operand, eq, rhs):
        self.operation = operation
        self.Object = obj
        self.operand = operand
        self.eq = eq
        self.rhs = rhs
        self.options = {'EQ' : self.__equate,
           'RAND' : self.__checkRand,
           'PRIME' : self.__checkPrime,
           'REPLAY' : self.__checkReplay,
           'PASSWORD' : self.__checkPassword,
        }

    def __str__(self):
        lines = ''
        lines = (f'{self.operation}({self.Object.getfname()}:{self.operand} {self.eq} {self.rhs})')
        return lines

    def __checkPrime(self, LObj):
        print("Checking Prime")
        for k in LObj:
            if k.getfname() == self.Object.getfname():
                res = sympy.isprime(int(k.getVar(self.operand)))
                print(f'{self.__str__()} {int(k.getVar(self.operand))}:{res}')

    def __checkRand(self, LObj):
        seen = set()
        res = True
        print("Checking Rand")
        for k in LObj:
            if k.getfname() == self.Object.getfname():
                if int(k.getVar(self.operand),0) not in seen:
                    #values.append(int(k.getVar(self.operand)))
                    seen.add(int(k.getVar(self.operand),0))
                    print(f'Pass {int(k.getVar(self.operand),0)}')
                    res = True
                else:
                    res = False
                    print(f'Failed {int(k.getVar(self.operand),0)}')
                    break
        print(f'{self.__str__()} :{res}')

    def __checkReplay(self):
        print("Checking Replay")

    def __checkPassword(self):
        print("Checking Password")

    def __equate(self):
        print("Checking equation")


    def set(self, operation, obj, operand, eq, rhs):
        self.operation = operation
        self.Object = obj
        self.operand = operand
        self.eq = eq
        self.rhs = rhs

    #TODO Add verification funtion as per Operation
    def verify(self, LObj):
        # implement EQ, RAND, PRIME, REPLAY, PASSWORD
        self.options[self.operation](LObj)
