from rule_scanner import parseRules
from log_scanner import parseLogs

RULES = "rules"
LOGS = "log.txt"
# Parse Logs
L = parseLogs()
print("\n=>PARSING LOGS...")
L.parse(LOGS)
LObj = L.getObjects()
#for k in LObj:
#    print(k)

# Parse Rule file/s
print("\n=>PARSING RULES...")
R = parseRules()
R.parse(RULES)
Obj = R.getObjects()
print("\n==>LOADING OBJECTS...")
for k, v in Obj.items():
    print(f"{k}")

# Extract Constraints and verify
C = R.getConstraints()
print("\n=>VERIFYING CONSTRAINTS...")
for opr in C:
    opr.verify(LObj)
    print(opr)

# Extract Order and verify
print("\n=>VERIFYING ORDER...")
O = R.getOrder()
print("i/p state")
for order in O:
    #print(order)
    order.verify(LObj)

# Extract Forbidden and verify
print("\n=>VERIFYING FORBIDDEN API...")
F = R.getForbidden()
print(F)
F.verify(LObj)
