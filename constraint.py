from object import Object 
from nistrng import *
import numpy
import sympy

class Constraint:
    def __init__(self, operation, obj, operand, eq, rhs):
        self.operation = operation
        self.Object = obj
        self.operand = operand
        self.eq = eq
        self.rhs = rhs
        self.res = True
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

    def __checkReplay(self, LObj):
        print("Checking Replay")
        seen = set()
        res = True
        for k in LObj:
            if k.getfname() == self.Object.getfname():
                if int(k.getVar(self.operand),0) not in seen:
                    #values.append(int(k.getVar(self.operand)))
                    seen.add(int(k.getVar(self.operand),0))
                    print(f'Pass {int(k.getVar(self.operand),0)}')
                else:
                    res = False
                    print(f'Failed {int(k.getVar(self.operand),0)}')
        print(f'{self.__str__()} :{res}')

    def __randtestNIST(self, rand_list):
        binary_sequence = []
        # unpack to single binary array
        for x in rand_list:
            print(x)
            binary_sequence.extend([int(d) for d in str(bin(x))[2:]])
        binary_sequence = numpy.array(binary_sequence)
        print("Random sequence encoded in 8-bit signed format:")
        print(binary_sequence)
        # Check the eligibility of the test and generate an eligible battery from the default NIST-sp800-22r1a battery
        eligible_battery: dict = check_eligibility_all_battery(binary_sequence, SP800_22R1A_BATTERY)
        # Print the eligible tests
        print("Eligible test from NIST-SP800-22r1a:")
        for name in eligible_battery.keys():
            print("-" + name)
        # Test the sequence on the eligible tests
        results = run_all_battery(binary_sequence, eligible_battery, False)
        # Print results one by one
        print("Test results:")
        for result, elapsed_time in results:
            if result.passed:
                print("- PASSED - score: " + str(numpy.round(result.score, 3)) + " - " + result.name + " - elapsed time: " + str(elapsed_time) + " ms")
            else:
                print("- FAILED - score: " + str(numpy.round(result.score, 3)) + " - " + result.name + " - elapsed time: " + str(elapsed_time) + " ms")


    # Test randomness using NIST SP800-22
    def __checkRand(self, LObj):
        rand_list = []
        print("Checking Rand")
        i = 0
        for k in LObj:
            if k.getfname() == self.Object.getfname():
                #values.append(int(k.getVar(self.operand)))
                print(k.getfname(), self.operand)
                try:
                    print(k.getVar(self.operand))
                    rand_list.append(int(k.getVar(self.operand),0))
                except:
                    print("Failed to get value")
        self.__randtestNIST(rand_list)

    def __checkPassword(self):
        # TODO Implement something like 
        # run hashcat with rockerlist
        print("Checking Password")

    def __equate(self, LObj):
        # TODO support < <= > >= == operations
        print("Checking equation")
        res = True

        if self.rhs[0].isnumeric():
            rhs = [int(i,0) for i in self.rhs]
        else:
            rhs = self.rhs

        for k in LObj:
            if k.getfname() == self.Object.getfname():
                lhs = int(k.getVar(self.operand), 0)
            else:
                continue

            if self.eq == '<':
                if lhs >= rhs:
                    self.res = False
                else:
                    #print(f'Pass {lhs} < {self.rhs[0]}')
                    pass
            elif self.eq == '<=':
                if lhs > rhs:
                    self.res = False
                else:
                    #print(f'Pass {lhs} <= {self.rhs[0]}')
                    pass
            elif self.eq == '>':
                if lhs <= rhs:
                    self.res = False
                else:
                    #print(f'Pass {lhs} > {self.rhs[0]}')
                    pass
            elif self.eq == '>=':
                if lhs < rhs:
                    self.res = False
                else:
                    #print(f'Pass {lhs} >= {self.rhs[0]}')
                    pass
            elif self.eq == '==':
                if lhs not in rhs:
                    print(rhs)
                    self.res = False
                else:
                    #print(f'Pass {lhs} == {self.rhs[0]}')
                    pass
            else:
                print('Unsupported operation ',self.eq)
                self.res = False
                break
        print(f'{self.__str__()} :{self.res}')



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
