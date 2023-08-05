import os
import glob

import cv2
import mediapipe as mp

from pose_estimation import compute_pose

mp_drawing = mp.solutions.drawing_utils
mp_drawing_styles = mp.solutions.drawing_styles
mp_face_mesh = mp.solutions.face_mesh

# For static images:
IMAGE_FILES = [
    "/src/research/mugshot_frontal_original_all/001_frontal.jpg",
    "/src/research/mugshot_frontal_original_all/002_frontal.jpg",
    "/src/research/mugshot_frontal_original_all/003_frontal.jpg",
    "/src/research/mugshot_frontal_original_all/004_frontal.jpg",
]


def compute_landmarks(image):
    with mp_face_mesh.FaceMesh(
        static_image_mode=True, max_num_faces=1, refine_landmarks=True, min_detection_confidence=0.5
    ) as face_mesh:
        results = face_mesh.process(cv2.cvtColor(image, cv2.COLOR_BGR2RGB))
    return results.multi_face_landmarks


def draw_results(image, face_landmarks):
    annotated_image = image.copy()
    mp_drawing.draw_landmarks(
        image=annotated_image,
        landmark_list=face_landmarks,
        connections=mp_face_mesh.FACEMESH_TESSELATION,
        landmark_drawing_spec=None,
        connection_drawing_spec=mp_drawing_styles.get_default_face_mesh_tesselation_style(),
    )
    mp_drawing.draw_landmarks(
        image=annotated_image,
        landmark_list=face_landmarks,
        connections=mp_face_mesh.FACEMESH_CONTOURS,
        landmark_drawing_spec=None,
        connection_drawing_spec=mp_drawing_styles.get_default_face_mesh_contours_style(),
    )
    mp_drawing.draw_landmarks(
        image=annotated_image,
        landmark_list=face_landmarks,
        connections=mp_face_mesh.FACEMESH_IRISES,
        landmark_drawing_spec=None,
        connection_drawing_spec=mp_drawing_styles.get_default_face_mesh_iris_connections_style(),
    )
    return annotated_image


def main():

   # for image_file in glob.glob("/src/research/mugshot_frontal_original_all/*.jpg"):
    #for image_file in glob.glob("algorithms/tmp/*.jpg"):
    for image_file in glob.glob("/src/research/my_database/*.jpg"):

        image = cv2.imread(image_file)

        multi_face_landmarks = compute_landmarks(image)
        if not multi_face_landmarks:
            continue

        landmarks = list(multi_face_landmarks[0].landmark)
        exif = {"image_height": image.shape[0], "image_width": image.shape[1]}
        yaw, pitch, roll = compute_pose(landmarks, exif)

        annotated_image = draw_results(image, multi_face_landmarks[0])
        print(f"File {os.path.basename(image_file)}: Yaw = {yaw:+.1f}, Pitch = {pitch:+.1f}, Roll = {roll:+.1f} ")


if __name__ == "__main__":
    main()
