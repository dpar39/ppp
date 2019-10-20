export class Point {
    x: number;
    y: number;
    constructor(x: number, y: number) {
        this.x = x;
        this.y = y;
    }

    equals(pt: Point): boolean {
        return pt.x === this.x && pt.y === this.y;
    }
    norm(): number {
        return Math.sqrt(this.x * this.x + this.y * this.y);
    }

    add(pt: Point): Point {
        return new Point(this.x + pt.x, this.y + pt.y);
    }

    sub(pt: Point): Point {
        return new Point(this.x - pt.x, this.y - pt.y);
    }

    mult(scalar: number) {
        return new Point(this.x * scalar, this.y * scalar);
    }

    div(scalar: number) {
        return new Point(this.x / scalar, this.y / scalar);
    }

    distTo(pt: Point): number {
        const dx = this.x - pt.x;
        const dy = this.y - pt.y;
        return Math.sqrt(dx * dx + dy * dy);
    }

    angle(pt: Point): number {
        const dx = this.x - pt.x;
        const dy = this.y - pt.y;
        return Math.atan2(dy, dx);
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
