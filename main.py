from rule_scanner import parseRules
from log_scanner import parseLogs

# Parse Logs
L = parseLogs()
print("\nParsing Logs...")
L.parse('log.txt')
LObj = L.getObjects()
for k in LObj:
    print(k)

# Parse Rule file/s
R = parseRules()
R.parse("rules")
Obj = R.getObjects()
print("\nLoading Objects...")
for k, v in Obj.items():
    print(f"{k} = {v}")

# Extract Constraints and verify
C = R.getConstraints()
print("\nVerifying Constraints...")
for opr in C:
    opr.verify(LObj)
    print(opr)

# Extract Order and verify
print("\nVerifying Order...")
O = R.getOrder()
for order in O:
    print(order)
    order.verify(LObj)
