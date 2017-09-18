
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

export class CrownChinPointPair {
    crownPoint: Point;
    chinPoint: Point;
}

export interface INativeWrapper {

    // Sets an image in the C++ backend, returns the image key to be used in subsequent calls
    // Returns the image key that identifies the image in the backend
    setImage(dataUrl: string): Promise<string>

    detectLandMarks(imgKey: string): Promise<LandMarks>

}