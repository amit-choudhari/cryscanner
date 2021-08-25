import gdb
import logging
import gdb.printing
import time
import os

apis = ['EVP_CIPHER_CTX_new', 'EVP_CIPHER_CTX_cleanup', 'EVP_BytesToKey', 'EVP_EncryptInit_ex', 'EVP_DecryptInit_ex', 'EVP_EncryptUpdate','EVP_EncryptFinal_ex','EVP_DecryptUpdate','EVP_DecryptFinal_ex','EVP_CIPHER_CTX_free']

class SimpleCommand(gdb.Command):
        fo=open("log.txt","w+")
        fi=open("scratch.txt", "w+")
        def __init__(self):
            super(SimpleCommand, self).__init__("simple_command", gdb.COMMAND_USER)
	
        def print_args(self):
            print('=> START arguments')
            gdb.execute('info args')
            print('=> END arguments')

        def print_stacktrace(self):
            print('=> START stack frame')
            cur_frame = gdb.selected_frame()
            while cur_frame is not None:
                print(cur_frame.name())
                # get the next frame
                cur_frame = cur_frame.older()
            print('=> END stack frame')

        def parse_bt(self):
           line = self.fi.readline()
           gdb.execute("set logging off")
           gdb.execute("set logging file log.txt")
           gdb.execute("set logging on")
           print (line)
           gdb.execute("set logging off")
           gdb.execute("set logging file scratch.txt")
           gdb.execute("set logging on")

        def parse_args(self):
            line = self.fi.readline()
            try:
                while(line != ''):
                    if '=> INFO ARGS-END' in line:
                        break
                    arg = line.split(' =')[0]
                    s = "ptype "+arg
                    gdb.execute("set logging off")
                    ft = open("tmp_scratch.txt", "w+")
                    gdb.execute("set logging file tmp_scratch.txt")
                    gdb.execute("set logging on")
                    if 'salt' not in arg:
                        gdb.execute(s)
                    gdb.execute("set logging off")
                    gdb.execute("set logging file log.txt")
             
                    gdb.execute("set logging on")
                    tmp_line = ft.readline()
                    if 'struct' in tmp_line:
                        while(tmp_line != ''):
                            check_line = tmp_line
                            tmp_line = ft.readline()
                        print ('struct '+arg)
                        if '} *' in check_line:
                            s1 = 'print/x *' +arg
                        else:
                            s1 = 'print/x ' +arg
                        try:
                            gdb.execute(s1)
                        except:
                            pass
             
                    else:
                        if 'key' in arg:
                            print("array ",arg)
                            s1 = 'print/x *' +arg+'@32'
                            gdb.execute(s1)
                        elif 'salt' in arg:
                            print("array ",arg)
                            s1 = 'print/x *' +arg+'@8'
                            gdb.execute(s1)
                        else:
                            s1 = 'print/x '+arg
                            print (s+' '+line)
                    line = self.fi.readline()
            except:
                pass

            gdb.execute("set logging off")
            gdb.execute("set logging file scratch.txt")
            gdb.execute("set logging on")

        def invoke(self, arg, from_tty):
            # when we call simple_command from gdb, this method
            # is invoked
            gdb.execute("set logging file scratch.txt")
            gdb.execute("set print pretty on")
            gdb.execute("set print repeats 0")
            print("Hello from simple_command")
            gdb.execute('start')

            # Add breakpoints
            for api in apis:
                bp = gdb.Breakpoint(api)
            print('')
            print('=> BREAKPOINT END')

            logging.basicConfig(filename="scratch.txt", level=logging.INFO)

            while True:
                gdb.execute("set logging on")
                # TODO fix finish command
                #gdb.execute("finish")
                gdb.execute("continue")

                line = self.fi.readline()
                inferiors = gdb.inferiors()
                test = 0
                for inf in gdb.inferiors():
                    print('INF PROC'+str(inf.pid))
                    if inf.pid:
                        print('Continue')
                    else:
                        print('EXIT!!')
                        test = 1
                if test == 1:
                    break
                # TODO: Note the api called and i/p params
                #   TODO: Fetch different types of parameters
                #       like structs, pointer to structs.
                # TODO: Find a way to get output arguments.
                # TODO: Push to log file
                gdb.execute('next')
                print('=> BREAKPOINT HIT!!')
                print('=> Backtrace')
                gdb.execute('backtrace')
                print('=> INFO ARGS-START')
                gdb.execute('info args')
                print('=> INFO ARGS-END')
                gdb.execute('set logging off')
                gdb.execute('set logging on')

                print('=> BEFORE PARSE')
                gdb.execute("set logging off")
                gdb.execute("set logging on")

                # consume all commands
                # line = self.fi.readline()
                while(line != ''):
                    if 'INFO ARGS-START' in line:
                        self.parse_args()
                    if 'Backtrace' in line:
                        self.parse_bt()
                    line = self.fi.readline()
            print('=> AFTER CONSUMING COMMANDS')
            gdb.execute("set logging off")

SimpleCommand()
