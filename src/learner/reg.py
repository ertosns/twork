import tasks
from sklearn import linear_model

def rand_coef():
    vectors = tasks.getVects(None)
    reg = linear_model.LinearRegression()
    xytuple = zip([vector[1:] for vector in vectors], range(len(vector)-1))
    xy = [list(tuple) for tuple in xytuple]
    reg.fit(xy)
    relation = zip([vector[1] for vector in vectors], [x[1] for x in reg.coef_]).sort(lambda task:task[1])
    print relation

rand_coef()
