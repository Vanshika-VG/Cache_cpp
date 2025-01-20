import numpy as np


def Choice():
    options = ['0', '1']
    s = ''.join(np.random.choice(options, (32)))
    ans = hex(int(s, 2))
    if len(ans) < 10:
        cnt = 10 - len(ans)
        ans = ans + cnt*'0'
    start = np.random.choice([0,1])
    
    if start == 0:
        mode = "R: "
    else:
        mode = "W: "
    return mode+"0x"+ans[2:]

list = np.array([])
for i in range(50):
    list = np.append(list, Choice())
print(len(list))
list = np.append(list, list)
print(len(list))
list = np.append(list, list)
print(len(list))
list = np.append(list, list)
np.random.shuffle(list)
print(len(list))
with open("Hex_Random_Numbers.txt", "w") as file:
    file.write('\n'.join(list))