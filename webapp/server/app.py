import os
import sys
import json
import logging
import requests
from flask import Flask, jsonify, request
from werkzeug.utils import secure_filename

this_dir = os.path.dirname(os.path.realpath(__file__))
zip_to_latlon_file = os.path.join(this_dir, "zip-to-latlon.json")
with open(zip_to_latlon_file) as fp:
    zip_to_latlon = json.load(fp)

app = Flask(__name__, static_url_path="", static_folder="www")
ALLOWED_IMG_FORMATS = set(["jpg", "jpeg", "png", "webp", "ppm"])


@app.route("/api/ping", methods=["GET"])
def ping():
    return jsonify({"ping": "success"})

@app.route("/api/location", methods=["GET"])
def location():
    ip = request.headers.get("x-forwarded-for")
    if not ip:
        ip = request.remote_addr
    if ip == "127.0.0.1":
        ip = "136.56.142.237"
    url = f"https://api.ipgeolocationapi.com/geolocate/{ip}"
    app.logger.info("Requesting " + url)
    try:
        res = requests.get(url)
        res.raise_for_status()
        return jsonify(res.json())
    except requests.exceptions.HTTPError as err:
        app.logger.error("Error: " + err)
        return jsonify({"error": 1})


@app.route("/")
@app.route("/start")
@app.route("/settings")
@app.route("/about")
@app.route("/standards")
@app.route("/prints")
@app.route("/walgreens")
def catch_all(methods=["GET"]):
    return app.send_static_file("index.html")


if __name__ == "__main__":
    port = int(os.environ.get("PORT", 5000))
    app.run(host="0.0.0.0", port=port, debug=True)

if "DYNO" in os.environ:
    app.logger.addHandler(logging.StreamHandler(sys.stdout))
    app.logger.setLevel(logging.INFO)
