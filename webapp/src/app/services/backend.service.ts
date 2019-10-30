import {Injectable, EventEmitter, SecurityContext} from '@angular/core';
import {DomSanitizer} from '@angular/platform-browser';
import {TiledPhotoRequest} from '../model/datatypes';

export class ImageLoadResult {
    constructor(public imgKey: string, public imgDataUrl: string, public exifInfo: any) {}
}

@Injectable()
export class BackEndService {
    runtimeInitialized: EventEmitter<boolean> = new EventEmitter();
    appLoadingProgressReported: EventEmitter<number> = new EventEmitter();

    _isMobilePlatform = false;
    _module: any;
    _runtimeInitialized = false;
    worker: Worker;

    private _onImageSet: (result: ImageLoadResult) => void;
    private _onLandmarksDetected: (landmarks: object) => void;
    private _onCreateTiledPrint: (pngDataUrl: string) => void;

    constructor(private sanitizer: DomSanitizer) {
        // this.plt.ready().then((readySource) => {
        //     this._isMobilePlatform = this.plt.is('ios') || this.plt.is('android');
        // });

        this.worker = new Worker('assets/wasm-worker.js');
        this.worker.postMessage({cmd: 'start'}); // Start the worker.

        this.worker.addEventListener(
            'message',
            (e: MessageEvent) => {
                switch (e.data.cmd) {
                    case 'onRuntimeInitialized':
                        this._runtimeInitialized = true;
                        this.runtimeInitialized.emit(true);
                        console.log('All good and configured');
                        break;
                    case 'onImageSet':
                        console.log(`Image has been set: ${e.data.imgKey}`);
                        const imageKey = e.data.imgKey;
                        const exifInfo = e.data.EXIFInfo;
                        const imageDataUrl = this.createPngDataUrl(e.data.pngUrl);
                        this._onImageSet(new ImageLoadResult(imageKey, imageDataUrl, exifInfo));
                        break;
                    case 'onLandmarksDetected':
                        this._onLandmarksDetected(e.data.landmarks);
                        break;
                    case 'onCreateTilePrint':
                        const pngDataUrl = this.createPngDataUrl(e.data.pngUrl);
                        this._onCreateTiledPrint(pngDataUrl);
                        break;
                    case 'onAppDataLoadingProgress':
                        this.appLoadingProgressReported.emit(e.data.progressPct);
                        break;
                }
            },
            false
        );
    }

    createPngDataUrl(pngUrl): any {
        return this.sanitizer.bypassSecurityTrustResourceUrl(pngUrl);
    }

    loadImageInMemory(file: File): Promise<ImageLoadResult> {
        return new Promise((resolve, reject) => {
            this._onImageSet = resolve;
            const readerRaw = new FileReader();
            readerRaw.onloadend = () => {
                const arrayBuffer = readerRaw.result;
                this.worker.postMessage({cmd: 'setImage', imageData: arrayBuffer});
            };
            readerRaw.readAsArrayBuffer(file);
        });
    }

    retrieveLandmarks(imgKey: string): Promise<any> {
        return new Promise((resolve, reject) => {
            this._onLandmarksDetected = resolve;
            this.worker.postMessage({cmd: 'detectLandmarks', imgKey: imgKey});
        });
    }

    getTiledPrint(req: TiledPhotoRequest): Promise<string> {
        return new Promise((resolve, reject) => {
            this._onCreateTiledPrint = resolve;
            this.worker.postMessage({cmd: 'createTiledPrint', request: req});
        });
    }
}
