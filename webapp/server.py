import os
from flask import Flask, request, send_from_directory

def secure_filename(x):
    return x

app = Flask(__name__, static_url_path='', static_folder='dist')

@app.route('/')
def root():
    return app.send_static_file('index.html')


@app.route('/api/upload', methods=['POST'])
def upload_image():
    if 'uploads[]' in request.files:
        file1 = request.files['uploads[]']
        fname = secure_filename(file1.filename)
        file1.save(os.path.join('uploads', fname))


if __name__ == "__main__":
    app.run(debug=True)