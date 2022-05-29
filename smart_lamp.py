import sqlite3

smart_lamp_db = '/var/jail/home/sophiez/smart_lamp.db'

def request_handler(request):

    conn = sqlite3.connect(smart_lamp_db)
    c = conn.cursor()  # move cursor into database (allows us to execute commands)
    c.execute('''CREATE TABLE IF NOT EXISTS smart_lamp_table (red int, green int, blue int);''') # run a CREATE TABLE command
    things = c.execute('''SELECT * FROM smart_lamp_table''')
    
    for x in things:
        color = (x[0], x[1], x[2])

    return color
