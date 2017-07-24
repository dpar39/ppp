
import{ INativeWrapper} from './interfaces'

var cpp: any;
class NativeWrapper implements INativeWrapper
{
    setImage(dataUrl: string): Promise<string> {
        throw new Error("Method not implemented.");
    }
    detectLandMarks(imgKey: string): Promise<any> {
        throw new Error("Method not implemented.");
    }

}