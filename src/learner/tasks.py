import sqlite3 as sql
import os

SQL_DATE_FORMAT = "%Y-%m-%d %H:%M:%S";

def getVects(target):
    vectors = []
    taskvec = []
    crows=0
    val=0
    db = sql.connect(os.environ['TWORK_PROF']+'/twork.db')
    cur = db.cursor()
    cur.execute('select * from DAILYTERM')
    tasks = sorted(cur.fetchall(), key=lambda session:session[1])
    if (target):
        tasks = [row for row in tasks if row[1] in target]

    for i in range(len(tasks)):
        if (i == len(tasks)-1):
            continue
        
        if (vectors[len(vectors)-1][0] != tasks[i][1] and tasks[i][1] == tasks[i+1][1]) if len(vect) > 0 else (tasks[i][1] == tasks[i+1][1]):
            taskvec[0] = task[0][1]
            cur.execute('select * from {task[i][1]} where ROWID > {tasks[i][2]} and ROWID < {tasks[i+1][2]}')
            sessions = cur.fetchall()
            if sessions[0][1] > 0:
                val=0
                crows=0
                for row in session:
                     val+=row[1]
                taskvec.append(val)
            else:
                for i in len(session):
                    if i < len(session)-1:
                        diff = datetime(session[i+1][0], SQL_DATE_FORMAT) - datetime(session[i][0], SQL_DATE_FORMAT)
                        taskvec.append(diff.sec + diff.minute*60 + diff.hour*60*60 +
                        diff.day*60*60*24 + diff.month*60*60*24*30 + diff.year*60*60*24*365)
                        vectors.append(taskvec)
    db.commit()
    db.close()
    return vect    
