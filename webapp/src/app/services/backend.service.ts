import { Injectable, EventEmitter, SecurityContext } from '@angular/core';
import { DomSanitizer } from '@angular/platform-browser';
import { TiledPhotoRequest, CrownChinPointPair } from '../model/datatypes';

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

  private _cacheImageLoadResult: ImageLoadResult;
  private _cachePrintResult: any; // SafeResourceUrl
  private _cacheLandmarks: any; // Landmarks

  private _onImageSet: (result: ImageLoadResult) => void;
  private _onLandmarksDetected: (landmarks: object) => void;
  private _onCreateTiledPrint: (pngDataUrl: string) => void;

  constructor(private sanitizer: DomSanitizer) {
    // this.plt.ready().then((readySource) => {
    //     this._isMobilePlatform = this.plt.is('ios') || this.plt.is('android');
    // });

    this.worker = new Worker('assets/wasm-worker.js');
    this.worker.postMessage({ cmd: 'start' }); // Start the worker.

    this.worker.addEventListener(
      'message',
      (e: MessageEvent) => {
        switch (e.data.cmd) {
          case 'onRuntimeInitialized':
            this._runtimeInitialized = true;
            this.runtimeInitialized.emit(true);
            break;
          case 'onImageSet':
            const imageKey = e.data.imgKey;
            const exifInfo = e.data.EXIFInfo;
            const imageDataUrl = this.createPngDataUrl(e.data.pngUrl);
            this._cacheImageLoadResult = new ImageLoadResult(imageKey, imageDataUrl, exifInfo);
            this._onImageSet(this._cacheImageLoadResult);
            break;
          case 'onLandmarksDetected':
            this._cacheLandmarks = e.data.landmarks;
            this._onLandmarksDetected(this._cacheLandmarks);
            break;
          case 'onCreateTilePrint':
            this._cachePrintResult = this.createPngDataUrl(e.data.pngUrl);
            this._onCreateTiledPrint(this._cachePrintResult);
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
        this.worker.postMessage({ cmd: 'setImage', imageData: arrayBuffer });
      };
      readerRaw.readAsArrayBuffer(file);
    });
  }

  retrieveLandmarks(imgKey: string): Promise<any> {
    return new Promise((resolve, reject) => {
      this._onLandmarksDetected = resolve;
      this.worker.postMessage({ cmd: 'detectLandmarks', imgKey });
    });
  }

  getTiledPrint(req: TiledPhotoRequest): Promise<string> {
    return new Promise((resolve, reject) => {
      this._onCreateTiledPrint = resolve;
      this.worker.postMessage({ cmd: 'createTiledPrint', request: req });
    });
  }

  getCacheImageLoadResult(): ImageLoadResult {
    return this._cacheImageLoadResult;
  }
  getCachePrintResult(): any {
    return this._cachePrintResult ? this._cachePrintResult : '#';
  }
  getCacheLandmarks(): any {
    return this._cacheLandmarks;
  }
  getRuntimeInitialized(): boolean {
    return this._runtimeInitialized;
  }

  updateCrownChin(crownChinPointPair: CrownChinPointPair) {
    this._cacheLandmarks = crownChinPointPair;
  }
}
