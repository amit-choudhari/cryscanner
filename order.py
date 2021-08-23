from object import Object 
from greenery import *
from greenery.lego import parse
from transitions import Machine
from transitions import *
from transitions.extensions import GraphMachine

class StateMachine(object):
    def __init__(self, fsm, states, finals):
        self.fsm = fsm
        self.events = set()
        self.finals = finals
        self.machine = Machine(model=self, states=states, initial=states[0])
        self.gmachine = GraphMachine(model=self, states=states, initial=states[0])
        self.machine.auto_transitions = True
        self.machine.send_event = True
        for key in fsm.map:
            for key2 in fsm.map[key]:
                self.machine.add_transition(trigger=key2, source=str(key), dest=str(fsm.map[key][key2]))
                self.gmachine.add_transition(trigger=key2, source=str(key), dest=str(fsm.map[key][key2]))
                self.events.add(key2)
                print(f"trigger={key2}, source={key}, dest={fsm.map[key][key2]}");
                #print(f"trigger={type(key2)}, source={type(key)}, dest={type(fsm.map[key][key2])}");

        #print(self.machine.states)

    def __str__(self):
        lines = f'\n{self.machine.states}, {self.fsm.map}\n'
        return lines
        pass

    # TODO do we need it?
    def create(self, sm):
        pass

    def verify(self, call_flow):
        i = None
        event = Event(None, self.machine)
        #print("###",event)
        try:
            for i in call_flow:
                self.trigger(i)
                print(i,' ',self.state)
            if int(self.state) in self.finals:
                print("Accepting state:",self.state)
            else:
                print(f"failed Order- current state:{self.state} trigger:{i} final:{self.finals}")
         
            self.get_graph().draw('my_state_diagram.png', prog='dot')
            #print(FSM.get_model_state())
        except Exception as e:
            print(e)
            print(f"Failed Order- current state:{self.state} trigger:{i} final:{self.finals}")
            pass

class Order(object):
    def __init__(self, obj, regx_order):
        self.Objects = obj
        self.regx_order = regx_order 
        self.FSM = self.__convertToFSM()

    def __str__(self):
        lines = ''
        lines = (f'{self.regx_order}')
        lines = lines + self.FSM.__str__()
        return lines

    def __convertToFSM(self):
        fsm = parse(self.regx_order).to_fsm()
        states = [str(x) for x in list(fsm.states)]
        FSM = StateMachine(fsm, states, fsm.finals)
        print(states)

        return FSM


    def set(self, obj, regx_order):
        self.Objects = obj
        self.reqx_order = regx_order 
        self.FSM = None

    # TODO order verification with logs
    def verify(self, LObj):
        call_flow = []
        for o in LObj:
            for k in self.FSM.events:
                if self.Objects[k].getfname() == o.getfname():
                    call_flow.append(k)

        print(call_flow)
        self.FSM.verify(call_flow)
        pass

