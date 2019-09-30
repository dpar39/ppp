import {Injectable, EventEmitter, SecurityContext} from '@angular/core';
import {DomSanitizer, SafeResourceUrl} from '@angular/platform-browser';
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
                        const imageDataUrl = this.createPngDataUrl(e.data.pngData);
                        this._onImageSet(new ImageLoadResult(imageKey, imageDataUrl, exifInfo));
                        break;
                    case 'onLandmarksDetected':
                        this._onLandmarksDetected(e.data.landmarks);
                        break;
                    case 'onCreateTilePrint':
                        const pngDataUrl = this.createPngDataUrl(e.data.pngData);
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

    createPngDataUrl(pngArrayBuffer): any {
        const blob = new Blob([pngArrayBuffer], {type: 'image/png'});
        const imageUrl = URL.createObjectURL(blob);
        const pngDataUrl = this.sanitizer.bypassSecurityTrustResourceUrl(imageUrl);
        return pngDataUrl; //this.sanitizer.sanitize(SecurityContext., pngDataUrl);
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

    // return new Promise((resolve, reject) => {
    //     if (true || this._isMobilePlatform) {
    //         const { PppPlugin } = Plugins;
    //         const reader = new FileReader();
    //         reader.readAsDataURL(file);
    //         reader.onloadend = () => {
    //             const content64 = reader.result as string;
    //             PppPlugin.setImage({ imgData: content64 }).then((response) => {
    //                 resolve(response.imgKey);
    //             });
    //         };
    //     }
    //     if (!this._isMobilePlatform) {
    //         const formData = new FormData();
    //         formData.append('uploads[]', file, file.name);
    //         const xhr: XMLHttpRequest = new XMLHttpRequest();
    //         xhr.onreadystatechange = () => {
    //             if (xhr.readyState === 4 && xhr.status === 200) {
    //                 // We have a successful response from the server
    //                 console.log('Image successfully uploaded to the server');
    //                 const response = JSON.parse(xhr.responseText);
    //                 const imgKey: string = response.imgKey;
    //                 if (imgKey) {
    //                     resolve(imgKey);
    //                 }
    //                 reject(imgKey);
    //             }
    //         };
    //         xhr.open('POST', '/api/upload', true);
    //         xhr.send(formData);
    //     }
    // });

    retrieveLandmarks(imgKey: string): Promise<any> {
        return new Promise((resolve, reject) => {
            this._onLandmarksDetected = resolve;
            this.worker.postMessage({cmd: 'detectLandmarks', imgKey: imgKey});
        });

        // return new Promise((resolve, reject) => {
        //     if (this._isMobilePlatform) {
        //         const { PppPlugin } = Plugins;

        //         PppPlugin.detectLandmarks({ imgKey: imgKey }).then((result) => {
        //             const landmarks = JSON.parse(result.landmarks);
        //             resolve(landmarks);
        //         });
        //     }

        //     if (!this._isMobilePlatform) {
        //         const xhr: XMLHttpRequest = new XMLHttpRequest();
        //         const url = '/api/landmarks/' + imgKey;
        //         xhr.open('GET', url);
        //         xhr.send();
        //         xhr.onreadystatechange = (e) => {
        //             if (xhr.readyState === 4 && xhr.status === 200) {
        //                 try {
        //                     const landmarks = JSON.parse(xhr.responseText);
        //                     resolve(landmarks);
        //                 } catch (e) {
        //                     console.log(e);
        //                     reject();
        //                 }
        //             }
        //         };
        //     }
        // });
    }

    getTiledPrint(req: TiledPhotoRequest): Promise<string> {
        return new Promise((resolve, reject) => {
            this._onCreateTiledPrint = resolve;
            this.worker.postMessage({cmd: 'createTiledPrint', request: req});
        });

        // return new Promise((resolve, reject) => {
        //     const xhr = new XMLHttpRequest();
        //     const url = '/api/photoprint';
        //     xhr.open('POST', url, true);
        //     xhr.setRequestHeader('Content-Type', 'application/json');
        //     xhr.responseType = 'blob';
        //     xhr.onreadystatechange = () => {
        //         if (xhr.readyState === 4 && xhr.status === 200) {
        //             const blob = xhr.response;
        //             const binaryData = [];
        //             binaryData.push(blob);
        //             const b = window.URL.createObjectURL(
        //                 new Blob(binaryData, { type: 'image/png' }));

        //             const outImgSrc = this.sanitizer.bypassSecurityTrustResourceUrl(b);
        //             resolve(outImgSrc);
        //         }
        //     };
        //     const data = JSON.stringify(req);
        //     xhr.send(data);
        // });
    }
}
