"""
Convert landmarks
"""
import os
import glob

SCFACE_DB_DIR = r'G:\VSOnline\PassportPhotoApp\research\mugshot_frontal_original_all'

def main():
    """
    Converts landmarks to VIA CSV format
    """
    csv_lines = ['#filename,file_size,file_attributes,region_count,region_id,region_shape_attributes,region_attributes\n']
    ann_files = glob.glob(os.path.join(SCFACE_DB_DIR, '*.pos'))
    jpg_files = glob.glob(os.path.join(SCFACE_DB_DIR, '*.jpg'))

    for i in range(len(ann_files)):
        ann_file = ann_files[i]
        jpg_file = jpg_files[i]
        filesize = os.path.getsize(jpg_file)

        jpg_name = os.path.basename(jpg_file)
        with open(ann_file) as fid:
            land_marks = fid.readlines()

        mark_indices = [0, 16, 6, 9, 14, 15]
        num_marks = len(mark_indices)
        for j in range(num_marks):
            coords = land_marks[mark_indices[j]].split(' ')
            x = int(float(coords[0]) + 0.5)
            y = int(float(coords[1]) + 0.5)
            line = '%s,%d,"{}",%d,%d,"{""name"":""point"",""cx"":%d,""cy"":%d}","{}"\n' \
                    % (jpg_name, filesize, num_marks, j, x, y)
            csv_lines.append(line)

    ann_via_file = os.path.join(SCFACE_DB_DIR, 'via_region_data.csv')
    with open(ann_via_file, 'wb') as fid:
        fid.writelines(csv_lines)

if __name__ == '__main__':
    main()

