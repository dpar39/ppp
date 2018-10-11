
import os
import json
from flask import Flask, abort, request, redirect, url_for, send_from_directory, flash, jsonify, make_response
from werkzeug.utils import secure_filename

import libpppwrapper as ppp

app = Flask(__name__, static_url_path='', static_folder='dist')
ALLOWED_IMG_FORMATS = set(['jpg', 'jpeg',  'png', 'webp', 'ppm'])

assert ppp.configure('config.json')

@app.route('/')
def root():
    return app.send_static_file('index.html')


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
            content = file.read()
            file_path = os.path.join('uploads', secure_filename(filename))
            with open(file_path, 'wb') as fp:
                fp.write(content)
            img_key = ppp.set_image(content)
            return jsonify({'imgKey': img_key})

@app.route('/api/landmarks')
def get_landmarks():
    img_key = request.args.get('imgKey')
    if img_key:
        lm = ppp.detect_landmarks(str(img_key))
        return lm

def parsePoint(pt):
    p = {
        'x': int(pt['x']),
        'y': int(pt['y']),
    }
    return p

def validateUnits(units):
    if units not in ["mm", "cm", "inch"]:
        raise Exception( "Unrecognized units: " + units)
    return units

def parseCanvas(canvas):
    obj = {
        'height': float(canvas['height']),
        'width': float(canvas['width']),
        'resolution': int(canvas['resolution']),
        'units': validateUnits(canvas['units'])
    }
    return obj

def parsePassportStandard(ps) :
    obj = {
        'pictureHeight': float(ps.pictureHeight),
        'pictureWidth': float(ps.pictureWidth),
        'faceHeight': float(ps.faceHeight),
        'units': validateUnits(ps.units)
    }
    return obj

@app.route('/api/photoprint',  methods=['POST'])
def get_photo_print():

    print_def = request.get_json()

    # TODO: validate request

    if 'imgKey' in print_def:
        img_key = print_def['imgKey']
        png_content = ppp.create_tiled_print(img_key, print_def)
        response = make_response(png_content)
        response.headers.set('Content-Type', 'image/png')
        return response
    return abort(400)

if __name__ == "__main__":
    app.run(debug=True)
