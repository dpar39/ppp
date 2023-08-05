from collections import namedtuple
from typing import List

from face_geometry import PCF, get_metric_landmarks, procrustes_landmark_basis


import numpy as np
import cv2


class ExifInfo:
    def __init__(self) -> None:
        self.focal_length_in_35mm = 0


Point3d = namedtuple("Point3d", ["x", "y", "z"])

points_idx = [33,263,61,291,199]
points_idx = points_idx + [key for (key,val) in procrustes_landmark_basis]
points_idx = list(set(points_idx))
points_idx.sort()


def compute_pose(landmarks: List[Point3d], exif_info):

    face_3d = []
    face_2d = []

    img_w = exif_info["image_width"]
    img_h = exif_info["image_height"]

    focal_length = img_w
    center = (img_w / 2, img_h / 2)
    camera_matrix = np.array(
        [[focal_length, 0, center[0]], [0, focal_length, center[1]], [0, 0, 1]],
        dtype="double",
    )
    distortion_matrix = np.zeros((4, 1), dtype=np.float64)

    pcf = PCF(
        near=1,
        far=10000,
        frame_height=img_h,
        frame_width=img_w,
        fy=camera_matrix[1, 1],
    )

    lmnp = np.array([(lm.x, lm.y, lm.z) for lm in landmarks[:468]])
    lmnp = lmnp.T

    metric_landmarks, pose_transform_mat = get_metric_landmarks(lmnp.copy(), pcf)

    image_points = lmnp[0:2, points_idx].T * np.array([img_w, img_h])[None, :]
    model_points = metric_landmarks[0:3, points_idx].T

    success, rotation_vector, translation_vector = cv2.solvePnP(
        model_points, image_points, camera_matrix, distortion_matrix, flags=cv2.SOLVEPNP_ITERATIVE
    )

    # see here:
    # https://github.com/google/mediapipe/issues/1379#issuecomment-752534379
    # pose_transform_mat[1:3, :] = -pose_transform_mat[1:3, :]
    rmat, _ = cv2.Rodrigues(pose_transform_mat[:3, :3])

    pitch, yaw, roll = map(lambda a: float(a) * 180/3.14159, rmat)
    return yaw, pitch, roll
