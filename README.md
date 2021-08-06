# Cryscanner
A generic tool to identify misuse of cryptographic libraries.

## Table of contents
* [General info](#general-info)
* [Core components](#core-components)
* [Setup](#setup)
* [TODO](#todo)

## General info

## Core components
There are two major components of this tool.
1. debug\_lib.py (runtime tool for log collection)
2. main.py (offline tool for log processing)
a. log\_scanner.py: Parses the logs and stores as a structured data.
b. rule\_scanner.py: Parses the rules file to extracts the information on parameter constraints, order of function calls, forbidden function calls. Creates a state machine to verify the call flow. Constraints are verified on the bases of equality(EQ), randomness(RAND), prime(PRIME), replay(REPLAY), password(PASS).

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

['0', '1', '2', '3', '4', '5', '6']

Loading Objects...
a = SSL_CTX_new()
var:meth = None

b = OPENSSL_init_ssl()
var:opts = None
var:settings = None

c = SSL_CTX_get_cert_store()
var:ctx = None

d = SSL_new()
var:ctx = None

e = SSL_write()
var:s = None
var:buf = None
var:num = None


Verifying Constraints...

Verifying Order...
a   1
b   2
c   3
c   3
d   4
b   5
e   6
ab(c)+dbe

```

## Done
- [x] Implement rule\_parser and extract list of objects, contraints, order.
- [x] Build a state machine based on order.
- [x] Implement log\_parser and extract list of objects and parameters.
- [x] Verify Order rules
## TODO
- [-] Identify Final state in order verification
- [-] Implement Operations for constraint check (EQ, RAND, PRIME, REPLAY, PASS)
- [-] Add support of return value

**Free Software, Hell Yeah!**

