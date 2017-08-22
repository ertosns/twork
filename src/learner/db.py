import sqlite3 as sql
import os

SQL_DATE_FORMAT = "%Y-%m-%d %H:%M:%S";

'''
return 3d-vector of activity in the form
life[act0[day0,day1,...,dayn],act1[day0,day1,...,dayn],...,actn[day0,day1,...,dayn]]
in which dayi[activity_name, average_value, frequency]
'''
#TODO add max,low values for values for full information
def get_3d_vectors(target):
    3d_vector = [[[]]]
    ctask=0
    T_NAME=1
    with sql.connect(os.environ['TWORK_PROF']+'/twork.db') as db:
        cur = db.cursor()
        cur.execute('select * from DAILYTERM')
        tasks = sorted(cur.fetchall(), key=lambda session:session[1])
        if (target):
            tasks = [row for row in tasks if row[1] in target]

        for i in range(len(tasks)):
            if tasks[i][T_NAME] != tasks[i+1][T_NAME] or i == len(tasks)-1:
                tasks+=1
                3d_vector.append([[]]) #new column
                continue
                
            cur.execute('select * from {task[i][1]} where ROWID > {tasks[i][2]} and ROWID < {tasks[i+1][2]}')
            sessions = cur.fetchall()
            val=0
            freq=0
            if sessions[0][1] > 0:
                for row in session:
                    val+=row[1]
                    freq+=1
            else:
                for i in len(session):
                    if i == len(session)-1:
                        continue
                    diff = datetime(session[i+1][0], SQL_DATE_FORMAT) - datetime(session[i][0], SQL_DATE_FORMAT)
                    val += diff.sec + diff.minute*60 + diff.hour*60*60 + diff.day*60*60*24 + diff.month*60*60*24*30 + diff.year*60*60*24*365
                    freq+=1
                        
            3d_vector[task].append([task[0][1], val, freq])
                            
    return   3d_vector
