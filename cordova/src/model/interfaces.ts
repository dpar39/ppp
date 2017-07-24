

class Point {
    x: number;
    y: number;
}


 interface INativeWrapper {

    // Sets an image in the C++ backend, returns the image key to be used in subsequent calls
    // Returns the image key that identifies the image in the backend
    setImage(dataUrl: string): Promise<string>

    detectLandMarks(imgKey: string): Promise<ILandMarks>

}


interface ILandMarks {
    crownPoint: Point;
    chinPoint: Point;
}

export { INativeWrapper, ILandMarks}