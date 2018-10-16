import { TouchSequence } from "selenium-webdriver";

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

export class PassportStandard {

    constructor(
        public pictureWidth: number,
        public pictureHeight: number,
        public faceHeight: number,
        public units: UnitType,
        public name: string = 'Custom') {
    }

    sizestring() {
        const unitStr: string = this.units === UnitType.inch ? '"' : '' + this.units;
        return `${this.pictureWidth} x ${this.pictureHeight}${unitStr}`;
    }
}

export class Canvas {
    height: number;
    width: number;
    resolution: number;
    units: UnitType;
}

export class LandMarks {

    crownPoint: Point;
    chinPoint: Point;
    errorMsg: string;
}

export class CrownChinPointPair {
    crownPoint: Point;
    chinPoint: Point;
}

export class TiledPhotoRequest {
    imgKey: string;
    standard: PassportStandard;
    canvas: Canvas;
    crownPoint: Point;
    chinPoint: Point;

    constructor(imgKey: string, ps: PassportStandard, canvas: Canvas, ccPoints: CrownChinPointPair) {
        this.imgKey = imgKey;
        this.standard = ps;
        this.canvas = canvas;
        this.crownPoint = ccPoints.crownPoint;
        this.chinPoint = ccPoints.chinPoint;
    }
}