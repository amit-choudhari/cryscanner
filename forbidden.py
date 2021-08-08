from object import Object 

class Forbidden:
    def __init__(self, obj, forbidden):
        self.obj = obj
        self.forbidden = forbidden

    def __str__(self):
        lines = ''
        lines = (f'{self.forbidden}')
        return lines

    def verify(self, LObj):
        for k in self.forbidden:
            fname = self.obj[k].getfname()
            for o in LObj:
                if o.getfname() == fname:
                    print(f"Forbidden API {fname}() is used")
        pass
