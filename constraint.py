from object import Object 

class Constraint:
    def __init__(self, operation, obj, operand, eq, rhs):
        self.operation = operation
        self.Object = obj
        self.operand = operand
        self.eq = eq
        self.rhs = rhs

    def __str__(self):
        lines = ''
        lines = (f'{self.operation}({self.Object.getfname()}:{self.operand} {self.eq} {self.rhs})\n')
        return lines

    def set(self, operation, obj, operand, eq, rhs):
        self.operation = operation
        self.Object = obj
        self.operand = operand
        self.eq = eq
        self.rhs = rhs

    #TODO Add verification funtion as per Operation
    def verify(self):
        # implement EQ, RAND, PRIME, REPLAY, PASSWORD
        return 

