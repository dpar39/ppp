
from ctypes import *

libppp = cdll.LoadLibrary("liblibppp.dylib")

libppp.configure.restype = bool
libppp.configure.argtypes = [c_char_p]

libppp.set_image.restype = bool
libppp.set_image.argtypes = [c_char_p, c_int, c_char_p]

libppp.detect_landmarks.restype = bool
libppp.detect_landmarks.argtypes = [c_char_p, c_char_p]


with open('config.json') as fp:
    cfg = fp.read()

success = libppp.configure(cfg)

with open('../research/sample_test_images/000.jpg', 'rb') as fp:
    jpg_content = fp.read()

jpg_content_len = len(jpg_content)

img_key = create_string_buffer(20)
success = libppp.set_image(jpg_content, jpg_content_len, img_key)

img_key = img_key.value

landmarks = create_string_buffer(65535)
success = libppp.detect_landmarks(img_key, landmarks)
print (landmarks.value)