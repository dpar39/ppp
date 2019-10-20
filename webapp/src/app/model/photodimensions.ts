import {Point, toMillimeters, middlePoint, pointAtDistance, rotatedRectangle} from './geometry';

export class PhotoDimensions {
    pictureWidth: number;
    pictureHeight: number;
    units: string;
    faceHeight?: number;
    crownTop?: number;
    dpi: number;


}


export function getCroppingCenter(p: PhotoDimensions, crownPoint: Point, chinPoint: Point): Point {
    if (!(p.crownTop > 0)) {
        // Estimate the center of the picture to be the median point between the crown point and the chin point
        return middlePoint(crownPoint, chinPoint);
    }
    const crownToBottom = p.pictureHeight - p.crownTop;
    const crownToCenter = crownToBottom - p.pictureHeight / 2;
    const mmToPixRatio = crownPoint.sub(chinPoint).norm() / p.faceHeight;
    const crownToCenterPix = mmToPixRatio * crownToCenter;
    return pointAtDistance(crownPoint, chinPoint, crownToCenterPix);
}

export function getCroppingRectangle(p: PhotoDimensions, crownPoint: Point, chinPoint: Point): Point[] {
    const centerPic = getCroppingCenter(p, crownPoint, chinPoint);
    const faceHeightPix = crownPoint.distTo(chinPoint);
    const normal = crownPoint.angle(chinPoint);
    const scale = faceHeightPix / p.faceHeight;
    const cropHeightPix = p.pictureHeight * scale;
    const cropWidthPix = p.pictureWidth * scale;

    return rotatedRectangle(centerPic, cropWidthPix, cropHeightPix, normal);
}