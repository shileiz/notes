def swap(a, b):
    t = a
    a = b
    b = t

def change_list(l):
    l[0] = "changed 1"
    l.append("append last")

if __name__ == "__main__":
##    i = 11
##    j = 22
##    swap(i, j)
##    print("i=%d,j=%d"%(i,j))
    l = [1,2,3,]
    change_list(l)
    print(l)
