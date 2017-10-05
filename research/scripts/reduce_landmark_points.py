"""
Modifies the bug_300W_large_face_landmark_dataset
"""
import os
import xml.etree.ElementTree as ET

TRAINING_DATA_DIR = r'E:\Data\ibug_300W_large_face_landmark_dataset'

REM_POINTS = [1, 2, 3, 4, 5, 6, 7,  11, 12, 13, 14, 15, 16, 17,    18, 19, 20, 21, 22,   23, 24, 25, 26, 27,    28, 29, 30, 31,   32, 33, 34, 35, 36,  61, 62, 63, 64, 65, 66, 67, 68]

POINTS_TO_REMOVE = set([x-1 for x in REM_POINTS])

# <images>
#   <image file='helen/trainset/146827737_1.jpg'>
#     <box top='224' left='70' width='295' height='295'>
#       <part name='00' x='145' y='355'/>
#       <part name='01' x='144' y='382'/>
#       <part name='02' x='145' y='407'/>


def remove_landmarks(orig_train_data_xml, mod_train_data_xml):
    tree = ET.parse(orig_train_data_xml)
    root = tree.getroot()
    all_boxes = root.findall('./images/image/box')
    for box in all_boxes:
        remove_parts = []
        for part in box.iter('part'):
            pt_number = int(part.get('name'))
            if pt_number in POINTS_TO_REMOVE:
                remove_parts.append(part)
        for r in remove_parts:
            box.remove(r)
    tree.write(mod_train_data_xml)

def main():
    """
    Main
    """
    with open(os.path.join(TRAINING_DATA_DIR, 'removed_landmarks.txt'), 'w') as fp:
        s = " ".join(map(str, REM_POINTS))
        fp.write(s)
    print 'Processing test data ...'
    remove_landmarks(os.path.join(TRAINING_DATA_DIR, 'labels_ibug_300W_test.orig.xml'), \
        os.path.join(TRAINING_DATA_DIR, 'labels_ibug_300W_test.xml'))
    print 'Processing training data ...'
    remove_landmarks(os.path.join(TRAINING_DATA_DIR, 'labels_ibug_300W_train.orig.xml'), \
        os.path.join(TRAINING_DATA_DIR, 'labels_ibug_300W_train.xml'))
    print 'Operation completed!'

if __name__ == "__main__":
    main()