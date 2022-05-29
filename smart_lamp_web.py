import sqlite3

smart_lamp_db = '/var/jail/home/sophiez/smart_lamp.db'

def request_handler(request):

    if request['method'] == 'GET':
        return f"""
        
        

        <h1>Lamp Control</h1>
        <form action="/sandbox/sc/sophiez/smart_lamp_web.py" method="post">
        <label for="red">Red (between 0 and 255):</label>
        <input type="range" id="red" name="red" min="0" max="255">
        <br>
        <label for="green">Green (between 0 and 255):</label>
        <input type="range" id="green" name="green" min="0" max="255">
        <br>
        <label for="blue">Blue (between 0 and 255):</label>
        <input type="range" id="blue" name="blue" min="0" max="255">
        <br>
        <input type="submit">
        </form>

        
        """

    else:
        conn = sqlite3.connect(smart_lamp_db)
        c = conn.cursor()  # move cursor into database (allows us to execute commands)
        c.execute('''CREATE TABLE IF NOT EXISTS smart_lamp_table (red int, green int, blue int);''') # run a CREATE TABLE command
        c.execute('''INSERT into smart_lamp_table VALUES (?,?,?);''', (request['form']['red'], request['form']['green'], request['form']['blue']))
        conn.commit() # commit commands
        conn.close() # close connection to database

        return (request['form']['red'], request['form']['green'], request['form']['blue'])
