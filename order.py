from object import Object 
from greenery import *
from greenery.lego import parse
from transitions import Machine
from transitions import *
from transitions.extensions import GraphMachine

class StateMachine(object):
    def __init__(self, fsm, states):
        self.fsm = fsm
        self.machine = Machine(model=self, states=states, initial=states[0], ignore_invalid_triggers=True)
        self.gmachine = GraphMachine(model=self, states=states, initial=states[0], ignore_invalid_triggers=True)
        self.machine.auto_transitions = True
        self.machine.send_event = True
        for key in fsm.map:
            for key2 in fsm.map[key]:
                self.machine.add_transition(trigger=key2, source=str(key), dest=str(fsm.map[key][key2]))
                self.gmachine.add_transition(trigger=key2, source=str(key), dest=str(fsm.map[key][key2]))
                print(f"trigger={key2}, source={key}, dest={fsm.map[key][key2]}");
                #print(f"trigger={type(key2)}, source={type(key)}, dest={type(fsm.map[key][key2])}");

        #print(self.machine.states)
        #print(self.machine.events)

    def __str__(self):
        lines = f'\n{self.machine.states}, {self.fsm.map}\n'
        return lines
        pass

    # TODO do we need it?
    def create(self, sm):
        pass

    def verify(self):
        event = Event(None, self.machine)
        print("###",event)
        print(self.state)
        self.trigger('a')
        print(self.state)
        self.trigger('b')
        print(self.state)
        self.trigger('c')
        print(self.state)
        self.trigger('d')
        print(self.state)
        self.trigger('a')
        print(self.state)

        self.get_graph().draw('my_state_diagram.png', prog='dot')
        #print(FSM.get_model_state())
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
        #SM = StateMachine()
        FSM = StateMachine(fsm, states)
        print(states)

        return FSM


    def set(self, obj, regx_order):
        self.Objects = obj
        self.reqx_order = regx_order 
        self.FSM = None

    # TODO order verification with logs
    def verify(self):
        self.FSM.verify()
        pass

