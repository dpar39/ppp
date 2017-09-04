
export class Point {
    x: number;
    y: number;
    constructor(x: number, y: number)  {
        this.x = x;
        this.y = y;
    }
}

export enum UnitType {
    mm,
    cm,
    inch
}

export class PassportStandard {

    pictureWidth: number;
    pictureHeight: number;
    faceHeight: number;
    unit: UnitType
}

export class LandMarks {

    crownPoint: Point;
    chinPoint: Point;
    errorMsg: string;
}