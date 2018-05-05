from matplotlib import pyplot
import numpy

def get_valeus(filename):
    X = []
    Ys = []
    with open(filename, 'rt') as file:
        for line in file:
            n, *rest = map(int, line.split())
            X.append(n)
            Ys.append(rest)
    return X, list(zip(*Ys))

# make line more smooth
def get_avg(values, step):
    return [numpy.mean(values[start:start+step]) for start in range(0, len(values), step)]


operations = ['push_back', 'remove', 'insert', 'get', 'set']

Xtree, Ytree = get_valeus('cmake-build-debug/results')
Xvec, Yvec = get_valeus('cmake-build-debug/results-vec')

for i in range(4):
    pyplot.subplot(220 + i  + 1)
    pyplot.plot(get_avg(Xtree, 10000), get_avg(Ytree[i], 10000), label='Tree ' + operations[i], color='g')
    pyplot.plot(get_avg(Xvec, 10000), get_avg(Yvec[i], 10000), label='vector ' + operations[i], color='r')
    pyplot.xlabel("Number of elements")
    pyplot.ylabel("time, nanoseconds")
    pyplot.legend()

# pyplot.subplot(221)
# pyplot.plot(get_avg(Xtree, 10000), get_avg(Ytree[2], 10000), label='Tree ' + operations[2], color='g')
# pyplot.plot(get_avg(Xvec, 10000), get_avg(Yvec[2], 10000), label='vector ' + operations[2], color='r')
#
# pyplot.subplot(222)
# pyplot.plot(get_avg(Xtree, 10000), get_avg(Ytree[1], 10000), label='Tree', color='g')
# pyplot.plot(get_avg(Xvec, 10000), get_avg(Yvec[1], 10000), label='vector', color='r')




pyplot.show()
