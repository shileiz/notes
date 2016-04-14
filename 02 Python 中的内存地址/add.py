def add_one(a):
    print("id(a)={id_a}".format(id_a=id(a)))
    a+=1
    print("id(a)={id_a}".format(id_a=id(a)))
    return a
n=1
print("id(n)={id_n}".format(id_n=id(n)))
add_one(n)
print("id(n)={id_n}".format(id_n=id(n)))
print n
