import { Point, middlePoint, pointAtDistance, RotatedRect } from './geometry';

export class PhotoDimensions {
  pictureWidth: number;
  pictureHeight: number;
  units: string;
  faceHeight?: number;
  crownTop?: number;
  bottomEyeLine?: number;
  dpi: number;
}

export function getCroppingCenter(p: PhotoDimensions, crownPoint: Point, chinPoint: Point): Point {
  if (!(p.crownTop > 0) && !(p.bottomEyeLine > 0)) {
    // Estimate the center of the picture to be the median point between the crown point and the chin point
    return middlePoint(crownPoint, chinPoint);
  }

  let crownTop = 0;
  if (p.crownTop > 0) {
    crownTop = p.crownTop;
  } else if (p.bottomEyeLine > 0) {
    const chinFrownCoefficient = 0.8945 / 1.7699;
    crownTop = p.pictureHeight - p.bottomEyeLine - p.faceHeight * (1 - chinFrownCoefficient);
  } else {
    throw new Error('Logic error');
  }

  const crownToCenter = p.pictureHeight / 2 - crownTop;
  const mmToPixRatio = crownPoint.sub(chinPoint).norm() / p.faceHeight;
  const crownToCenterPix = mmToPixRatio * crownToCenter;
  return pointAtDistance(crownPoint, chinPoint, crownToCenterPix);
}

export function getCroppingRectangle(p: PhotoDimensions, crownPoint: Point, chinPoint: Point): RotatedRect {
  const centerPic = getCroppingCenter(p, crownPoint, chinPoint);
  const faceHeightPix = crownPoint.distTo(chinPoint);
  const normal = crownPoint.angle(chinPoint);
  const scale = faceHeightPix / p.faceHeight;
  const cropHeightPix = p.pictureHeight * scale;
  const cropWidthPix = p.pictureWidth * scale;
  return new RotatedRect(centerPic, cropHeightPix, cropWidthPix, normal);
}
