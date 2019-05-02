export class Point {
    x: number;
    y: number;
    constructor(x: number, y: number) {
        this.x = x;
        this.y = y;
    }
}

export enum UnitType {
    mm = 'mm',
    cm = 'cm',
    inch = 'inch'
}

export class PhotoDimensions {
    pictureWidth: number;
    pictureHeight: number;
    units: string;
    faceHeight?: number;
    crownTop?: number;
    dpi: number;
}

export class PhotoStandard {
    id: string;
    text: string;
    country: string;
    docType: string;
    dimensions: PhotoDimensions;

    backgroundColor?: string;
    printable?: boolean;
    digital?: boolean;
    officialLinks: string[] = [];
    comments?: string;

    constructor(id: string, text: string) {
        this.id = id;
        this.text = text;
    }
}

export class Canvas {
    id: string;
    text: string;
    height: number;
    width: number;
    resolution: number;
    units: UnitType;

    constructor(id: string, text: string) {
        this.id = id;
        this.text = text;
    }
}

export class LandMarks {
    crownPoint: Point;
    chinPoint: Point;
    errorMsg: string;
}

export class CrownChinPointPair {
    crownPoint: Point;
    chinPoint: Point;
    constructor(crownPoint = null, chinPoint = null) {
        this.crownPoint = crownPoint;
        this.chinPoint = chinPoint;
    }
}

export class TiledPhotoRequest {
    imgKey: string;
    standard: PhotoDimensions;
    canvas: Canvas;
    crownPoint: Point;
    chinPoint: Point;

    constructor(imgKey: string, ps: PhotoDimensions, canvas: Canvas, ccPoints: CrownChinPointPair) {
        this.imgKey = imgKey;
        this.standard = ps;
        this.canvas = canvas;
        this.crownPoint = ccPoints.crownPoint;
        this.chinPoint = ccPoints.chinPoint;
    }
}
