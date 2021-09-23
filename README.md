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
	a: EVP\_CIPHER\_CTX\_new()
	b: EVP\_CIPHER\_CTX\_free(c)
  	c: EVP\_EncryptInit\_ex()
  	d: EVP\_EncryptUpdate()
  	e: EVP\_EncryptFinal\_ex()
	i: EVP\_BytesToKey()
	h: aes\_ecb\_cipher()

ORDER
	(ab)*
	(cd+ed*)*

CONSTRAINTS
	x1: REPLAY(i:salt)
	x2: REPLAY(c:iv)
	x3: REPLAY(c:key)
	x5: RAND(i:salt)
	x6: RAND(c:key)

FORBIDDEN
	h
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
RULES file is " tests/aes_encryption/rules
LOGS file is " tests/aes_encryption/log.txt

=>PARSING LOGS...
object: EVP_CIPHER_CTX_new()
object: EVP_BytesToKey()
object: EVP_EncryptInit_ex()
object: aes_ecb_cipher()
...
object: EVP_CIPHER_CTX_free()
object: EVP_CIPHER_CTX_new()

=>PARSING RULES...
trigger=a, source=0, dest=1
trigger=b, source=1, dest=0
['0', '1']
trigger=c, source=0, dest=1
trigger=d, source=1, dest=2
trigger=d, source=2, dest=2
trigger=e, source=2, dest=3
trigger=c, source=3, dest=1
trigger=d, source=3, dest=3
['0', '1', '2', '3']
['h']

==>LOADING OBJECTS...
a
b
c
d
e
i
h

=>VERIFYING CONSTRAINTS...
Checking Replay
================================
Failed 115791883479003231763365946972015206849224643720322487168446144838650348756653
Failed 115791883479003231763365946972015206849224643720322486765513914036612612808365
Failed 115791883479003231763365946972015206849224643720322486765569254268833741463213
Failed 115791883479003231763365946972015206849224643720322486765753721709570836979373
Failed 115791883479003231763365946972015206849224643720322486765882848918086803840685

REPLAY(EVP\_BytesToKey:salt  ) :False
================================

Checking Replay
================================
Failed 69917253583556191692478147606208430964501388521261329726057961871917600211776
Failed 69917253583556191692478147606208430964501388521261329726057961871917600211776
Failed 69917253583556191692478147606208430964501388521261329726057961871917600211776

REPLAY(EVP\_EncryptInit\_ex:iv  ) :False
================================

Checking Replay
================================
Failed 77688215761253394586589689082649976992149542878860393383156848977076460267380
Failed 77688215761253394586589689082649976992149542878860393383156848977076460267380
Failed 77688215761253394586589689082649976992149542878860393383156848977076460267380
Failed 77688215761253394586589689082649976992149542878860393383156848977076460267380

REPLAY(EVP\_EncryptInit\_ex:key  ) :False
================================

Checking Randomness
================================
EVP\_BytesToKey salt
0xFFFFE23000000005555552800000000000000000000055550000BEEF0000DEAD
EVP\_BytesToKey salt
0xFFFFE23000000005555552800000000000000000000055550000BEEF0000DEAD
EVP\_BytesToKey salt
0xFFFFE23000000005555552800000000000000000000000020000BEEF0000DEAD
EVP\_BytesToKey salt
0xFFFFE23000000005555552800000000000000000000000020000BEEF0000DEAD
EVP\_BytesToKey salt
0xFFFFE23000000005555552800000000000000000000000050000BEEF0000DEAD
EVP\_BytesToKey salt
0xFFFFE23000000005555552800000000000000000000000050000BEEF0000DEAD
EVP\_BytesToKey salt
0xFFFFE230000000055555528000000000000000000000000F0000BEEF0000DEAD
EVP\_BytesToKey salt
0xFFFFE230000000055555528000000000000000000000000F0000BEEF0000DEAD
EVP\_BytesToKey salt
0xFFFFE23000000005555552800000000000000000000000160000BEEF0000DEAD
EVP\_BytesToKey salt
0xFFFFE23000000005555552800000000000000000000000160000BEEF0000DEAD
115791883479003231763365946972015206849224643720322487168446144838650348756653
115791883479003231763365946972015206849224643720322487168446144838650348756653
115791883479003231763365946972015206849224643720322486765513914036612612808365
115791883479003231763365946972015206849224643720322486765513914036612612808365
115791883479003231763365946972015206849224643720322486765569254268833741463213
115791883479003231763365946972015206849224643720322486765569254268833741463213
115791883479003231763365946972015206849224643720322486765753721709570836979373
115791883479003231763365946972015206849224643720322486765753721709570836979373
115791883479003231763365946972015206849224643720322486765882848918086803840685
115791883479003231763365946972015206849224643720322486765882848918086803840685
Eligible test from NIST-SP800-22r1a:
-monobit
-frequency\_within\_block
-longest\_run\_ones\_in\_a\_block
-dft
-non\_overlapping\_template\_matching
-serial
-approximate\_entropy
-cumulative sums
-random\_excursion
-random\_excursion\_variant
Test results:
- FAILED - score: 0.0 - Monobit - elapsed time: 0 ms
- FAILED - score: 0.0 - Frequency Within Block - elapsed time: 41 ms
- FAILED - score: 0.0 - Longest Run Ones In A Block - elapsed time: 0 ms
- FAILED - score: 0.002 - Discrete Fourier Transform - elapsed time: 13 ms
- FAILED - score: 0.0 - Non Overlapping Template Matching - elapsed time: 129 ms
- FAILED - score: 0.0 - Serial - elapsed time: 1253 ms
- FAILED - score: 0.0 - Approximate Entropy - elapsed time: 662 ms
- FAILED - score: 0.0 - Cumulative Sums - elapsed time: 166 ms
- FAILED - score: 0.337 - Random Excursion - elapsed time: 21 ms
- FAILED - score: 0.075 - Random Excursion Variant - elapsed time: 3 ms
Randomness test Failed!
================================

Checking Randomness
================================
EVP\_EncryptInit\_ex key
0xABC1F8CD602F8E314EC01E18919327CA2C37629FF77CD502FF6A3EF4442D2774
EVP\_EncryptInit\_ex key
0xABC1F8CD602F8E314EC01E18919327CA2C37629FF77CD502FF6A3EF4442D2774
EVP\_EncryptInit\_ex key
0xABC1F8CD602F8E314EC01E18919327CA2C37629FF77CD502FF6A3EF4442D2774
EVP\_EncryptInit\_ex key
0xABC1F8CD602F8E314EC01E18919327CA2C37629FF77CD502FF6A3EF4442D2774
EVP\_EncryptInit\_ex key
0xABC1F8CD602F8E314EC01E18919327CA2C37629FF77CD502FF6A3EF4442D2774
77688215761253394586589689082649976992149542878860393383156848977076460267380
77688215761253394586589689082649976992149542878860393383156848977076460267380
77688215761253394586589689082649976992149542878860393383156848977076460267380
77688215761253394586589689082649976992149542878860393383156848977076460267380
77688215761253394586589689082649976992149542878860393383156848977076460267380
Eligible test from NIST-SP800-22r1a:
-monobit
-frequency\_within\_block
-runs
-longest\_run\_ones\_in\_a\_block
-dft
-non\_overlapping\_template\_matching
-serial
-approximate\_entropy
-cumulative sums
-random\_excursion
-random\_excursion\_variant
Test results:
- PASSED - score: 0.576 - Monobit - elapsed time: 1 ms
- PASSED - score: 0.462 - Frequency Within Block - elapsed time: 7 ms
- FAILED - score: 0.005 - Runs - elapsed time: 0 ms
- PASSED - score: 0.06 - Longest Run Ones In A Block - elapsed time: 1 ms
- FAILED - score: 0.0 - Discrete Fourier Transform - elapsed time: 1 ms
- PASSED - score: 0.997 - Non Overlapping Template Matching - elapsed time: 66 ms
- PASSED - score: 0.112 - Serial - elapsed time: 620 ms
- PASSED - score: 0.024 - Approximate Entropy - elapsed time: 332 ms
- PASSED - score: 0.776 - Cumulative Sums - elapsed time: 91 ms
- FAILED - score: 0.112 - Random Excursion - elapsed time: 227 ms
- PASSED - score: 0.481 - Random Excursion Variant - elapsed time: 3 ms
Randomness test Passed!
================================


=>VERIFYING ORDER...
i/p state
['a', 'b', 'a', 'b', 'a', 'b', 'a', 'b', 'a', 'b', 'a', 'b', 'a', 'b', 'a', 'b', 'a', 'b', 'a', 'b', 'a']
a   1
b   0
a   1
b   0
a   1
b   0
a   1
b   0
a   1
b   0
a   1
b   0
a   1
b   0
a   1
b   0
a   1
b   0
a   1
b   0
a   1
failed Order- current state:1 trigger:a final:{0}
['c', 'd', 'e', 'd', 'c', 'd', 'e', 'd', 'c', 'd', 'e', 'd', 'c', 'd', 'e', 'd', 'c', 'd', 'e', 'd']
c   1
d   2
e   3
d   3
c   1
d   2
e   3
d   3
c   1
d   2
e   3
d   3
c   1
d   2
e   3
d   3
c   1
d   2
e   3
d   3
Accepting state: 3

=>VERIFYING FORBIDDEN API...
['h']
Forbidden API aes\_ecb\_cipher() is used

```
![State Machine](https://github.com/amitsirius/cryscanner/blob/main/my\_state\_diagram.png?raw=true)

## Done
- [x] Implement rule\_parser and extract list of objects, contraints, order.
- [x] Build a state machine based on order.
- [x] Implement log\_parser and extract list of objects and parameters.
- [x] Verify Order rules
- [x] Implement Operations for constraint check (EQ, RAND, PRIME, REPLAY)
- [x] Implement Forbidden API suppot in rules
- [x] Identify Final state in order verification
- [x] Add support for EQV for checking parameter corelations
- [x] Add randomness support using NIST tests
## TODO
- [-] Add support of return value

**Free Software, Hell Yeah!**

