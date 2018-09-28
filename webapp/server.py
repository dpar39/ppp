
import os
from flask import Flask, abort, request, redirect, url_for, send_from_directory, flash, jsonify
from werkzeug.utils import secure_filename

import libpppwrapper as ppp

app = Flask(__name__, static_url_path='', static_folder='dist')


@app.route('/')
def root():
    return app.send_static_file('index.html')


ALLOWED_IMG_FORMATS = set(['jpg', 'jpeg',  'png', 'webp', 'ppm'])


def allowed_file(filename):
    return '.' in filename and \
           filename.rsplit('.', 1)[1].lower() in ALLOWED_IMG_FORMATS


@app.route('/api/upload', methods=['POST'])
def upload_image():
    if request.method == 'POST':
        if 'uploads[]' not in request.files:
            abort(400)
        file = request.files['uploads[]']
        if file.filename == '':
            abort(400)
        if file and allowed_file(file.filename):
            filename = secure_filename(file.filename)
            if not os.path.isdir('uploads'):
                os.makedirs('uploads')
            file.save(os.path.join('uploads', filename))

            img_key =  ppp.set_image(file.read())
            return jsonify( {'imgKey': img_key})



if __name__ == "__main__":
    app.run(debug=True)
