"""
Convert landmarks
"""
import os
import glob
import os


def main():
    """
    Converts landmarks to VIA CSV format
    """
    scface_db_dir = os.path.join(os.path.dirname(__file__), "../mugshot_frontal_original_all")
    csv_lines = [
        "#filename,file_size,file_attributes,region_count,region_id,region_shape_attributes,region_attributes\n"
    ]
    ann_files = glob.glob(os.path.join(scface_db_dir, "*.pos"))
    jpg_files = glob.glob(os.path.join(scface_db_dir, "*.jpg"))

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
            coords = land_marks[mark_indices[j]].split(" ")
            x = int(float(coords[0]) + 0.5)
            y = int(float(coords[1]) + 0.5)
            line = '%s,%d,"{}",%d,%d,"{""name"":""point"",""cx"":%d,""cy"":%d}","{}"\n' % (
                jpg_name,
                filesize,
                num_marks,
                j,
                x,
                y,
            )
            csv_lines.append(line)

    ann_via_file = os.path.join(scface_db_dir, "via_region_data.csv")
    with open(ann_via_file, "w") as fid:
        fid.writelines(csv_lines)


if __name__ == "__main__":
    main()
