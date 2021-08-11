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
	a: SSL_CTX_new(meth)
	b: OPENSSL_init_ssl(opts, settings)
	c: SSL_CTX_get_cert_store(ctx)
	d: SSL_new(ctx)
	e: SSL_write(s, buf, num)

ORDER
	ab(c)+dbe

CONSTRAINTS
	PRIME(a:meth.flags)
	RAND(c:ctx.sha1)
	EQ(c:ctx.md5 == 1)
	EQ(c:ctx.md5 == 0)
	EQ(e:s.ext.psk_kex_mode >= 30)

FORBIDDEN
	c,d
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
=>PARSING LOGS...
object: SSL_CTX_new()
meth
object: OPENSSL_init_ssl()
settings
object: SSL_CTX_get_cert_store()
ctx
stats
ex_data
ext
srp_ctx
dane
ssl_mac_pkey_id
ssl_cipher_methods
ssl_digest_methods
ssl_mac_secret_size
object: SSL_CTX_get_cert_store()
ctx
stats
ex_data
ext
srp_ctx
dane
ssl_mac_pkey_id
ssl_cipher_methods
ssl_digest_methods
ssl_mac_secret_size
object: SSL_new()
ctx
stats
ex_data
ext
srp_ctx
dane
ssl_mac_pkey_id
ssl_cipher_methods
ssl_digest_methods
ssl_mac_secret_size
object: OPENSSL_init_ssl()
object: SSL_write()
s
statem
s3
tmp
valid_flags
dane
ex_data
ext
ocsp
srp_ctx

=>PARSING RULES...
trigger=a, source=0, dest=1
trigger=b, source=1, dest=2
trigger=c, source=2, dest=3
trigger=c, source=3, dest=3
trigger=d, source=3, dest=4
trigger=b, source=4, dest=5
['0', '1', '2', '3', '4', '5']
trigger=a, source=0, dest=1
trigger=b, source=1, dest=0
['0', '1']
['c', 'd']

==>LOADING OBJECTS...
a
b
c
d
e

=>VERIFYING CONSTRAINTS...
Checking Prime
PRIME(SSL_CTX_new:meth.flags  ) 2067337:True
PRIME(SSL_CTX_new:meth.flags  )
Checking Rand
Checking Replay
Pass 140737353809328
Pass 129
RAND(SSL_CTX_get_cert_store:ctx.sha1  ) :True
RAND(SSL_CTX_get_cert_store:ctx.sha1  )
Checking equation
EQ(SSL_CTX_get_cert_store:ctx.md5 == ['1']) :False
EQ(SSL_CTX_get_cert_store:ctx.md5 == ['1'])
Checking equation
EQ(SSL_CTX_get_cert_store:ctx.md5 == ['0']) :True
EQ(SSL_CTX_get_cert_store:ctx.md5 == ['0'])
Checking equation
EQ(SSL_write:s.ext.psk_kex_mode >= ['30']) :True
EQ(SSL_write:s.ext.psk_kex_mode >= ['30'])

=>VERIFYING ORDER...
i/p state
['a', 'b', 'c', 'c', 'd', 'b']
a   1
b   2
c   3
c   3
d   4
b   5
Accepting state: 5
['a', 'b', 'b']
a   1
b   0
Failed Order- current state:0 trigger:b final:{0}

=>VERIFYING FORBIDDEN API...
['c', 'd']
Forbidden API SSL_CTX_get_cert_store() is used
Forbidden API SSL_CTX_get_cert_store() is used
Forbidden API SSL_new() is used

```
![State Machine](https://github.com/amitsirius/cryscanner/blob/main/my_state_diagram.png?raw=true)

## Done
- [x] Implement rule\_parser and extract list of objects, contraints, order.
- [x] Build a state machine based on order.
- [x] Implement log\_parser and extract list of objects and parameters.
- [x] Verify Order rules
- [x] Implement Operations for constraint check (EQ, RAND, PRIME, REPLAY)
- [x] Implement Forbidden API suppot in rules
- [x] Identify Final state in order verification
## TODO
- [-] Add support of return value
- [-] Implement better version of RAND and PASS

**Free Software, Hell Yeah!**

