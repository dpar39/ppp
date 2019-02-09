import { Injectable, EventEmitter } from '@angular/core';
import { DomSanitizer, SafeResourceUrl } from '@angular/platform-browser';

import { Plugins } from '@capacitor/core';
import { Platform } from '@ionic/angular';
import { TiledPhotoRequest } from '../model/datatypes';


@Injectable()
export class BackEndService {

    runtimeInitialized: EventEmitter<boolean> = new EventEmitter();

    _isMobilePlatform = false;
    _module: any;
    _runtimeInitialized = false;
    worker: Worker;

    private _onImageSet: (imgId: string) => void;
    private _onLandmarksDetected: (landmarks: object) => void;
    private _onCreateTiledPrint: (pngDataUrl: SafeResourceUrl) => void;

    constructor(private sanitizer: DomSanitizer, private plt: Platform) {
        this.plt.ready().then((readySource) => {
            this._isMobilePlatform = this.plt.is('ios') || this.plt.is('android');
        });

        this.worker = new Worker('assets/wasm-worker.js');
        this.worker.postMessage({'cmd': 'start'}); // Start the worker.

        this.worker.addEventListener('message', (e: MessageEvent) => {
            switch (e.data.cmd) {
                case 'onRuntimeInitilized':
                    this._runtimeInitialized = true;
                    this.runtimeInitialized.emit(true);
                    console.log('All good and configured');
                    break;
                case 'onImageSet':
                    console.log(`Image has been set: ${e.data.imgKey}`);
                    this._onImageSet(e.data.imgKey);
                    break;
                case 'onLandmarksDetected':
                     this._onLandmarksDetected(e.data.landmarks);
                    break;
                case 'onCreateTilePrint':
                    const pngArraryBuffer = e.data.pngData;
                    const blob = new Blob( [ pngArraryBuffer ], { type: 'image/png' } );
                    const imageUrl = URL.createObjectURL( blob );
                    const pngDataUrl = this.sanitizer.bypassSecurityTrustResourceUrl(imageUrl);
                    this._onCreateTiledPrint(pngDataUrl);
                    break;
            }
        }, false);
    }

    uploadImageToServer(file: File): Promise<string> {
        return new Promise((resolve, reject) => {
            const reader = new FileReader();
            reader.onload = () => {
                const arrayBuffer = reader.result;
                this.worker.postMessage({'cmd': 'setImage', 'imageData': arrayBuffer});
                this._onImageSet = resolve;
            };
            reader.readAsArrayBuffer(file);
        });

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
        //                 // We have a sucessful response from the server
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
    }

    retrieveLandmarks(imgKey: string): Promise<any> {
        return new Promise((resolve, reject) => {
            this._onLandmarksDetected = resolve;
            this.worker.postMessage({'cmd': 'detectLandmarks', 'imgKey': imgKey});
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
            this.worker.postMessage({'cmd': 'createTiledPrint', 'request': req});
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
