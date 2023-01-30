from flask import Flask, flash, request, redirect, url_for, session, escape, send_file
from werkzeug.utils import secure_filename
import os

ALLOWED_EXTENSIONS = {'zip','txt', 'pdf', 'png', 'jpg', 'jpeg', 'gif','gz'}

app = Flask(__name__)

userList=dict()#instead of creating a DB, for our task this is enough. Also, persistence is not needed so it's fine to not do anything else.
userList["admin"]="admin"

app.secret_key = 'secretkey'

@app.route("/")
@app.route("/home")
def home(message=""):
    if 'username' not in session:
        return redirect(url_for('login'))
    entries = os.listdir('./files')
    output=""
    for entry in entries:
        if(output!=""):
            output+="<br>"
        #output+= entry
        output+='''<form action="/download">'''+entry+'''
        <input type="hidden" name="file" + value="''' +entry+'''">
        <input type=submit value=DL>
        </form>'''
    return "Logged in!<br>" + '''
        <form action="/logout" method="get">
            <input type="submit" value="Logout" name="Submit" id="goto_logout" />
        </form>
        <form action="/upload" method="get">
            <input type="submit" value="Upload file" name="Submit" id="upload" />
        </form>
        <form action="/download">
        <input type=text name=file>
        <input type=submit value=Download>
        </form>
        ''' + "Contents:<br><br>" + output


@app.route("/download")
def download():
    filename = request.args.get('file')
    if filename in os.listdir('./files'):
        return send_file(os.path.join('./files/', filename), as_attachment=True)
    else:
        return "No such file on server"

def allowed_file(filename):
    return '.' in filename and \
           filename.rsplit('.', 1)[1].lower() in ALLOWED_EXTENSIONS

@app.route('/upload', methods=['GET', 'POST'])
def upload_file():
    if 'username' in session:
        if request.method == 'POST':
            if 'file' not in request.files:
                flash('No file part')
                return redirect(request.url)
            file = request.files['file']
            if file.filename == '':
                flash('No selected file')
                return redirect(request.url)
            if file and allowed_file(file.filename):
                filename = secure_filename(file.filename)
                file.save(os.path.join('./files/', filename))
                return "Upload successful" + '''
                <form action="/" method="get">
                    <input type="submit" value="Go to files" name="Submit" id="goto_files" />
                </form>
                <form action="/upload" method="get">
                    <input type="submit" value="Upload another file" name="Submit" id="goto_upload" />
                </form>
                <form action="/logout" method="get">
                    <input type="submit" value="Logout" name="Submit" id="goto_logout" />
                </form>
                '''
        return '''
        <!doctype html>
        <title>Upload your files</title>
        <h1>Upload your files</h1>
        <form method=post enctype=multipart/form-data>
        <input type=file name=file>
        <input type=submit value=Upload>
        </form>
        <br>
        <form action="/" method="get">
            <input type="submit" value="Go to files" name="Submit" id="goto_files" />
        </form>
        <br>
        <form action="/logout" method="get">
            <input type="submit" value="Logout" name="Submit" id="goto_logout" />
        </form>
        '''
    return 'You must log in first' + '''
    <form action="/login" method="get">
        <input type="submit" value="Login" name="Submit" id="goto_login" />
    </form>
    '''
   
@app.route('/login', methods=['GET', 'POST'])
def login():
    if 'username' in session:
        return redirect(url_for('home'))
    if request.method == 'POST':
        username = request.form['username']
        password = request.form['password']
        if username in userList:
            if userList[username] == password :
                session['username'] = request.form['username']
                return redirect(url_for('login'))
            else:
                return 'Invalid username or password' + ''' 
                <form action="/login" method="get">
                    <input type="submit" value="Try again" name="Submit" id="goto_login" />
                </form>
                '''
        else:
            return 'Invalid username or password' + ''' 
                <form action="/login" method="get">
                    <input type="submit" value="Try again" name="Submit" id="goto_login" />
                </form>
                '''
    return '''
        <form action="" method="post">
            <p><input type=text name=username>
            <p><input type=password name=password>
            <p><input type=submit value=Login>
        </form>
        '''

@app.route('/logout')
def logout():
    session.pop('username', None)
    return redirect(url_for('login'))

if __name__ == '__main__':
   app.run(debug=True, port=8989, host='0.0.0.0')