from ctypes import *

fepick = CDLL('../fepick.dll')
fepick.fepick_case.restype = c_int
fepick.fepick_case.argtypes = (POINTER(c_int), POINTER(c_char_p), c_int)

def fepick_case(id_list, title_list):
    n = len(id_list)
    p = (c_int*n)()
    s = (c_char_p*n)()
    for i in range(n):
        p[i] = int(id_list[i])
        s[i] =  str.encode(title_list[i])
    rval = fepick.fepick_case(p, s, n)
    for i in range(n):
        id_list[i] = p[i]
    return rval

n = 10000
id_list = [-x for x in range(1,n+1)]
title_list = ["SUBCASE " + str(abs(i)) for i in id_list]

active_count = fepick_case(id_list, title_list)

print("ACTIVE COUNT: " + str(active_count))
print("ACTIVE CASES:")

for x in id_list:
    if x > 0:
        print(x)