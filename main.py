from rule_scanner import parseRules
from log_scanner import parseLogs

# TODO Parse Logs

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
    opr.verify()
    print(opr)

# Extract Order and verify
print("\nVerifying Order...")
O = R.getOrder()
for order in O:
    order.verify()
    print(order)
