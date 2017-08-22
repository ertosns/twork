import db
from sklearn import linear_model

'''
return matrix life[activity[regression]]
in which regression=[activity_name, x1_coef_values, x1_coef_frequency]
'''

def tota_reg():
    reg = linear_model.LinearRegression()
    vectors = db.get_3d_vectors(None)
    val_coef = []
    freq_coef = []
    col = 0
    for activity in vectors:
        x = range[len(activity)]
        
        val = [day[1] for day in activity]
        val_coef.append([vectors[0]+'_value_x1_coef', reg.fit(x, val)[1]])
        
        freq = [day[2] for day in activity]
        task_ceof.append([vectors[0]+'_frequency_x1_coef', reg.fit(x, freq)[1]])
        
        col+=1
    return [val_coef, freq_coef]

def display_tota_relation():
    reg = tota_reg()
    merged_reg = sorted(reg[0]+reg[1], lambda act:act[1])
    for act in merged_reg:
        #format
        print act[0]+'     '+repr(act[1])

pass implement higher regression modules.
pass implement activity, trackers.
