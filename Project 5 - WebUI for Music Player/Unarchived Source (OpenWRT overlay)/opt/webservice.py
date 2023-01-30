from flask import Flask, flash, request, redirect, url_for, send_from_directory
from werkzeug.utils import secure_filename
from mpd import MPDClient
import os
import threading

mpdIP = "127.0.0.1" #"0.0.0.0" #localhost
mpdPort = 6600

hostIP= '0.0.0.0'
hostPort=6699

ALLOWED_EXTENSIONS = {'mp3','flac'}

app = Flask(__name__)

app.secret_key = 'lines-lab4'
musicLibraryDir = '/lol/var/lib/mpd/music/'

stopped = False
client = MPDClient()
client_lock = threading.Lock()
with client_lock:
   client.connect(mpdIP, mpdPort)
   client.update()

def reconnect():
    try:
        client.ping()
    except Exception:
        client.connect(mpdIP, mpdPort)
        client.update()

@app.route("/")
def index():
    reconnect()
    output = ""
    for song in client.playlistinfo():
        if(output != ""):
            output += "<br>"
        output += song["file"]
    return '''
        <form action="/upload" method="get">
            <input type="submit" value="Upload song" name="Submit"/>
        </form>
        <form action="/reload">
            <input type=submit value="|<" formaction="/previous">
            <input type=submit value="> ||" formaction="/playpause">
            <input type=submit value=">|" formaction="/next">
            <input type=submit value="Volume Down" formaction="/volumedown">
            <input type=submit value="Volume Up" formaction="/volumeup">
        </form>
        <form action="/download">
            <input type=text name=file>
            <input type=submit value=Download formaction="/download">
		    <input type=submit value="Move Up" formaction="/up">
		    <input type=submit value="Move Down" formaction="/down">
            <input type=submit value=Delete formaction="/delete">
        </form>
        ''' + "<h4>Current song:</h4>" + client.currentsong()["file"] + "<br><br>" + "<h4>Playlist:</h4>" + output

def allowed_file(filename):
    if filename.rsplit(".", 1)[1].lower() in ALLOWED_EXTENSIONS:
        return True
    else:
        return False

@app.route('/upload', methods=['GET', 'POST'])
def upload_file():
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
            absFilePath=os.path.join(musicLibraryDir, filename)
            file.save(absFilePath)
            reconnect()
            client.update()
            command="mpc add "
            command=command+filename
            reconnect()
            os.system(command)
            # client.add(filename)
            # #client.add(absFilePath)
            # #client.add(musicLibraryDir)
            # #return redirect(url_for("index"))
        else:
            return "Error uploading file"
    return '''
    <!doctype html>
    <title>Upload new File</title>
    <h1>Upload new File</h1>
    <form method=post enctype=multipart/form-data>
    <input type=file name=file>
    <input type=submit value=Upload>
    </form>
    '''

#------- Manage Playlist ------#
@app.route("/download")
def download():
    filename = request.args.get('file')
    if filename in os.listdir(musicLibraryDir):
        return send_from_directory(musicLibraryDir, filename, as_attachment=True)
    else:
        return redirect(url_for("index"))

@app.route("/up")
def up():
    filename = request.args.get('file')
    if filename in os.listdir(musicLibraryDir):
        reconnect()
        playlist = client.playlistinfo()
        pos = 0
        to = len(playlist) - 1
        for song in playlist:
            if song["file"] == filename :
                break
            pos += 1
        if pos > 0:
            to = pos - 1
        reconnect()
        client.move(pos, to)
    return redirect(url_for("index"))

@app.route("/down")
def down():
    filename = request.args.get('file')
    if filename in os.listdir(musicLibraryDir):
        reconnect()
        playlist = client.playlistinfo()
        pos = 0
        to = 0
        for song in playlist:
            if song["file"] == filename :
                break
            pos += 1
        if pos < len(playlist) - 1:
            to = pos + 1
        reconnect()
        client.move(pos, to)   
    return redirect(url_for("index"))

@app.route("/delete")
def delete():
    filename = request.args.get('file')
    if filename in os.listdir(musicLibraryDir):
        reconnect()
        pos = 0
        for song in client.playlistinfo():
            if song["file"] == filename :
                break
            pos += 1
        reconnect()
        client.delete(pos)
    return redirect(url_for("index"))


#------- Current Song Actions ------#
@app.route("/reload")
def reload():
    return redirect(url_for("index"))

@app.route("/previous")
def previous():
    reconnect()
    client.previous()
    return redirect(url_for("index"))

@app.route("/next")
def next():
    reconnect()
    client.next()
    return redirect(url_for("index"))

@app.route("/playpause")
def playpause():
    global stopped
    if stopped == False :
        stopped = True
        reconnect()
        client.pause()
    else :
        stopped = False
        reconnect()
        client.play()
    return redirect(url_for("index"))

@app.route("/volumedown")
def volumedown():
    reconnect()
    volume = (int)(client.status()['volume'])
    if volume >= 20 :
        reconnect()
        client.setvol(volume - 20)
    return redirect(url_for("index"))

@app.route("/volumeup")
def volumeup():
    reconnect()
    volume = (int)(client.status()['volume'])
    if volume <= 80 :
        reconnect()
        client.setvol(volume + 20)
    return redirect(url_for("index"))


if __name__ == '__main__':
    app.run(debug=True, port=hostPort, host=hostIP)