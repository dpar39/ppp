import {Injectable, EventEmitter} from '@angular/core';
import {DomSanitizer, SafeResourceUrl} from '@angular/platform-browser';
import {TiledPhotoRequest} from '../model/datatypes';

export class ImageLoadResult {
    constructor(public imgKey: string, public imgRotation: string, public imgDataUrl: string) {}
}

@Injectable()
export class BackEndService {
    runtimeInitialized: EventEmitter<boolean> = new EventEmitter();
    appLoadingProgressReported: EventEmitter<number> = new EventEmitter();

    _isMobilePlatform = false;
    _module: any;
    _runtimeInitialized = false;
    worker: Worker;

    private _imageRotation = '';
    private _imageDataUrl = '';
    private _imageKey = '';

    private _onImageSet: (result: ImageLoadResult) => void;

    private _onLandmarksDetected: (landmarks: object) => void;
    private _onCreateTiledPrint: (pngDataUrl: SafeResourceUrl) => void;

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
                        this._imageKey = e.data.imgKey;
                        this._onLoadImageEnded();
                        break;
                    case 'onLandmarksDetected':
                        this._onLandmarksDetected(e.data.landmarks);
                        break;
                    case 'onCreateTilePrint':
                        const pngArrayBuffer = e.data.pngData;
                        const blob = new Blob([pngArrayBuffer], {type: 'image/png'});
                        const imageUrl = URL.createObjectURL(blob);
                        const pngDataUrl = this.sanitizer.bypassSecurityTrustResourceUrl(imageUrl);
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

    loadImageInMemory(file: File): Promise<ImageLoadResult> {
        this._imageDataUrl = null;
        this._imageKey = null;
        this._imageRotation = null;
        return new Promise((resolve, reject) => {
            this._imageDataUrl = null;
            this._onImageSet = resolve;

            const readerRaw = new FileReader();
            readerRaw.onloadend = () => {
                const arrayBuffer = readerRaw.result;
                this.worker.postMessage({cmd: 'setImage', imageData: arrayBuffer});
                this._imageRotation = this.computeOrientation(arrayBuffer as ArrayBuffer);
            };

            const readerDataUrl = new FileReader();
            readerDataUrl.onloadend = () => {
                this._imageDataUrl = readerDataUrl.result as string;
                this._onLoadImageEnded();
            };
            readerRaw.readAsArrayBuffer(file);
            readerDataUrl.readAsDataURL(file);
        });
    }

    computeOrientation(arrayBuffer: ArrayBuffer): string {
        const rotation = {
            1: 'rotate(0deg)',
            3: 'rotate(180deg)',
            6: 'rotate(90deg)',
            8: 'rotate(270deg)'
        };

        const scanner = new DataView(arrayBuffer);
        let idx = 0;
        let value = 1; // Non-rotated is the default
        if (arrayBuffer.byteLength < 2 || scanner.getUint16(idx) !== 0xffd8) {
            return rotation[value];
        }

        idx += 2;
        let maxBytes = scanner.byteLength;
        while (idx < maxBytes - 2) {
            const uint16 = scanner.getUint16(idx);
            idx += 2;
            switch (uint16) {
                case 0xffe1: // Start of EXIF
                    const exifLength = scanner.getUint16(idx);
                    maxBytes = exifLength - idx;
                    idx += 2;
                    break;
                case 0x0112: // Orientation tag
                    // Read the value, its 6 bytes further out
                    // See page 102 at the following URL
                    // http://www.kodak.com/global/plugins/acrobat/en/service/digCam/exifStandard2.pdf
                    value = scanner.getUint16(idx + 6, false);
                    maxBytes = 0; // Stop scanning
                    break;
            }
        }
        return rotation[value];
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

    private _onLoadImageEnded() {
        if (this._imageKey && this._imageDataUrl && this._imageRotation) {
            this._onImageSet(new ImageLoadResult(this._imageKey, this._imageRotation, this._imageDataUrl));
        }
    }

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

    getTiledPrint(req: TiledPhotoRequest): Promise<SafeResourceUrl> {
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
