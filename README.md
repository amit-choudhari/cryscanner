# Cryscanner
A generic tool to identify miuse of cryptographic libraries.

## Table of contents
* [General info](#general-info)
* [Core components](#core-components)
* [Setup](#setup)
* [TODO](#todo)

## General info

## Core components
There are two major components of this tool.
1. debug_lib.py (runtime tool for log collection)
2. main.py (offline tool for log processing)
a. log_scanner.py: Parses the logs and stores as a structured data.
b. rule_scanner.py: Parses the rules file to extracts the information on parameter constraints, order of function calls, forbidden function calls. Creates a state machine to verify the call flow. Constraints are verified on the bases of equality(EQ), randomness(RAND), prime(PRIME), replay(REPLAY), password(PASS).

#### Other helper files:
1. object.py: Basic block representing each function call. Contains function name, map of all parameter name and value. The naming conventions is as per the depth of variable```{struct_name(optional)}.{struct_name(optional)}.{variable_name}```. For example: given a function ```init(struct SSL_ctx *ctx)```. where ```struct SSL_ctx``` contains variable ```version```. version is referred as ```ctx.version``` in rules and also stored with same name in object.
2. constraint.py: Implements different constraint checks like EQ, RAND, PRIME, REPLAY, PASS.
3. order.py: Implements the state machine for expected call flow and verifies it with the runtime call flow.

#### Rule File:
Below is a sample rule file.
```
OBJECTS
	a: getInstance(digestAlgorithm, primes);
	b: getInstance(digestAlgorithm, iv);
	c: PBEParameterSpec(salt, primes);

ORDER
	a(b|c)+d

CONSTRAINTS
	EQ(a:primes >= 20000)
	EQ(c:primes >= 10000)
	EQ(a:digestAlgorithm == SHA || RSA)
	RAND(b:iv)
	PRIME(a:primes)
	REPLAY(b:iv)
```

## Setup
Run the debug_lib.py tool with GDB
```
$ gdb --args openssl genrsa
...
Reading symbols from openssl...
(gdb) source debug_lib.py 
(gdb) simple_command
```
This command will capture the runtime logs in log.txt
Run main.py
```
$ python main.py 

Parsing Logs...
SSL_CTX_new()
var:meth.version = -135102464
var:meth.flags = 32767
var:meth.mask = 140737354133008
var:meth.ssl_new = 0x7ffff7fafd30
...

OPENSSL_init_ssl()
var:settings.filename = 0x6574617473
var:settings.appname = 0x525f505645000000
var:settings.flags = 7089068670126935617
var:Noneopts = 140733193388033

trigger=a, source=0, dest=1
trigger=b, source=1, dest=2
trigger=c, source=1, dest=2
trigger=b, source=2, dest=2
trigger=c, source=2, dest=2
trigger=d, source=2, dest=3
['0', '1', '2', '3']

Loading Objects...
a = getInstance()
var:digestAlgorithm = None
var:primes = None

b = getInstance()
var:digestAlgorithm = None
var:iv = None

c = PBEParameterSpec()
var:salt = None
var:primes = None


Verifying Constraints...
EQ(getInstance:primes >= ['20000'])

EQ(PBEParameterSpec:primes >= ['10000'])

EQ(getInstance:digestAlgorithm == ['SHA', 'RSA'])

RAND(getInstance:iv  )

PRIME(getInstance:primes  )

REPLAY(getInstance:iv  )


Verifying Order...
### <Event('None')@140718686517136>
0
1
2
2
3
3
a(b|c)+d
OrderedDict([('0', <State('0')@140718686622528>), ('1', <State('1')@140718686624640>), ('2', <State('2')@140718686975312>), ('3', <State('3')@140718686976464>)]), {0: {'a': 1}, 1: {'b': 2, 'c': 2}, 2: {'b': 2, 'c': 2, 'd': 3}, 3: {}}

```

## Done
- [x] Implement rule_parser and extract list of objects, contraints, order.
- [x] Build a state machine based on order.
- [x] Implement log_parser and extract list of objects and parameters.
## TODO
- [-] Verify Order rules
- [-] Implement Operations for constraint check (EQ, RAND, PRIME, REPLAY, PASS)
- [-] Add support of return value

**Free Software, Hell Yeah!**

