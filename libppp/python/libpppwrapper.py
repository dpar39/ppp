import os
import sys
import json
from ctypes import *

if sys.platform == "linux" or sys.platform == "linux2":
    libfile = 'liblibppp.so'
elif sys.platform == "darwin":
    libfile = 'liblibppp.dylib'
elif sys.platform == "win32":
    libfile = 'libppp.dll'


def resolve_filepath(rel_path):
    """
    """
    dirname, _ = os.path.split(os.path.abspath(__file__))

    while True:
        test_path = os.path.join(dirname, rel_path)
        if os.path.isfile(test_path):
            return test_path
        parent_dir = os.path.dirname(dirname)
        if not parent_dir or parent_dir == dirname:
            return None
        dirname = parent_dir


libfilepath = resolve_filepath(libfile)
libppp = cdll.LoadLibrary(libfilepath)

libppp.configure.restype = bool
libppp.configure.argtypes = [c_char_p]

libppp.set_image.restype = bool
libppp.set_image.argtypes = [c_char_p, c_int, c_char_p]

libppp.detect_landmarks.restype = bool
libppp.detect_landmarks.argtypes = [c_char_p, c_char_p]

libppp.create_tiled_print.restype = int
libppp.create_tiled_print.argtypes = [c_char_p, c_char_p, c_char_p]


def configure(config_file):
    """
    """
    with open(config_file) as fp:
        cfg = fp.read()
    return libppp.configure(cfg)


def set_image(img_content):
    """
    """
    try:
        if os.path.isfile(img_content):
            with open(img_content, 'rb') as fp:
                img_content = fp.read()
    except TypeError:
        pass

    img_content_len = len(img_content)
    img_key = create_string_buffer(16)
    success = libppp.set_image(img_content, img_content_len, img_key)
    if success:
        return img_key.value
    return None


def detect_landmarks(img_key):
    """
    """
    assert img_key and isinstance(img_key, str), 'Invalid image key'

    landmarks = create_string_buffer(65535)
    success = libppp.detect_landmarks(img_key, landmarks)
    if success:
        return landmarks.value
    return None


def create_tiled_print(img_key, request):
    """
    """
    assert request, 'Request is empty'
    if not isinstance(request, str):
        request = json.dumps(request)

    png_content = create_string_buffer(8*1024*1024)  # A buffer of 8MB at least
    num_bytes = libppp.create_tiled_print(img_key, request, png_content)
    png_d = png_content.raw[0:num_bytes]
    return png_d


def main():
    # Let's check that it works
    lib_cfg = resolve_filepath('config.json')
    assert configure(lib_cfg)
    print("Libppp configured successfully")

    img_key = set_image(resolve_filepath(
        'research/sample_test_images/000.jpg'))
    assert img_key, 'Unable to set image'
    print("Set image with key=%s" % img_key)

    landmarks = detect_landmarks(img_key)

    assert landmarks, 'No landmarks were retrieved'

    lm = json.loads(landmarks)
    print("Detected landmarks as: %s" % json.dumps(lm))

    request = {
        'crownPoint': lm['crownPoint'],
        'chinPoint': lm['chinPoint'],
        'canvas': {
            'height': 4.0,
            'width': 6.0,
            'resolution': 300,
            'units': "inch"
        },
        'standard': {
            'pictureWidth': 2.0,
            'pictureHeight': 2.0,
            'faceHeight': 1.1875,
            'eyesHeight': 1.25,
            'units': "inch"
        }
    }

    png_content = create_tiled_print(img_key, request)
    with open('output.png', 'wb') as fp:
        fp.write(png_content)
    print("Created tiled print from request")


if __name__ == "__main__":
    main()
