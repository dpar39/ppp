import { Injectable, EventEmitter, SecurityContext } from '@angular/core';
import { DomSanitizer, SafeResourceUrl } from '@angular/platform-browser';
import { CrownChinPointPair } from '../model/datatypes';
import { StorageService } from './storage.service';
import { Point } from '../model/geometry';

export class ImageLoadResult {
  constructor(
    public imgKey: string,
    public imgDataUrl: SafeResourceUrl,
    public exifInfo: any,
    public imageData: ImageData
  ) {}
}

export interface AppLoadProgress {
  progress: number; // 0 - 1
  step: string; // current step just completed
}

const LANDMARKS_KEY = 'landmarks';
const INPUT_IMAGE_NAME = 'inputImageName';

@Injectable({ providedIn: 'root' })
export class BackEndService {
  // Fired when initialized
  runtimeInitialized: EventEmitter<boolean> = new EventEmitter();

  appLoadingProgressReported: EventEmitter<AppLoadProgress> = new EventEmitter();

  landmarksUpdated: EventEmitter<CrownChinPointPair> = new EventEmitter();

  imageLoaded: EventEmitter<ImageLoadResult> = new EventEmitter();

  private _runtimeInitialized = false;
  private _worker: Worker;
  private _isRestoringSession = false;

  private _cacheImageLoadResult: ImageLoadResult;
  private _cacheLandmarks: CrownChinPointPair; // Landmarks
  private _printOutputFile: File;

  constructor(private sanitizer: DomSanitizer, private storage: StorageService) {
    this._worker = new Worker('assets/wasm-worker.js');
    this._worker.postMessage({ cmd: 'start' }); // Start the worker.

    this._worker.addEventListener(
      'message',
      (e: MessageEvent) => {
        switch (e.data.cmd) {
          case 'onRuntimeInitialized':
            this._runtimeInitialized = true;
            this.loadPreviousSession().then(() => this.runtimeInitialized.emit(true));
            break;
          case 'onImageSet':
            const imageKey = e.data.imgKey;
            const exifInfo = e.data.EXIFInfo;
            const imgBuffer = e.data.imageBuffer as ArrayBuffer;
            const width = e.data.width;
            const height = e.data.height;
            const imageData = new ImageData(new Uint8ClampedArray(imgBuffer), width, height);
            this._cacheImageLoadResult = new ImageLoadResult(imageKey, null, exifInfo, imageData);
            this.imageLoaded.emit(this._cacheImageLoadResult);
            if (this._isRestoringSession) {
              this._cacheLandmarks = this.storage.getItem(LANDMARKS_KEY);
              this.landmarksUpdated.emit(this._cacheLandmarks);
              this._isRestoringSession = false;
            } else {
              this.retrieveLandmarks(imageKey);
            }
            break;
          case 'onLandmarksDetected':
            this._cacheLandmarks = e.data.landmarks;
            this.persistResults();
            this.landmarksUpdated.emit(this._cacheLandmarks);
            break;
          case 'onAppDataLoadingProgress':
            this.appLoadingProgressReported.emit(e.data as AppLoadProgress);
            break;
        }
      },
      false
    );
  }

  createPngDataUrl(pngBlobUrl: string): any {
    return this.sanitizer.bypassSecurityTrustResourceUrl(pngBlobUrl);
  }

  loadImageInMemory(inputSrc: File | Blob): void {
    this._cacheImageLoadResult = null;
    this._cacheLandmarks = null;
    const file = inputSrc as File;
    if (!this._isRestoringSession && file) {
      this.storage.setItem(INPUT_IMAGE_NAME, file.name);
      this.storage.saveFile(file.name, file); // save input file as loaded
    }
    this.imageLoaded.emit(null); // loading new image...
    this.blobToArrayBuffer(inputSrc).then((imageDataArrayBuf) => {
      this._worker.postMessage({ cmd: 'setImage', imageData: imageDataArrayBuf }, [imageDataArrayBuf]);
    });
  }

  retrieveLandmarks(imgKey: string): void {
    this._worker.postMessage({ cmd: 'detectLandmarks', imgKey });
  }

  getCacheImageLoadResult(): ImageLoadResult {
    return this._cacheImageLoadResult;
  }

  getCacheLandmarks(): any {
    return this._cacheLandmarks;
  }
  getRuntimeInitialized(): boolean {
    return this._runtimeInitialized;
  }

  updateCrownChin(crownPoint: Point, chinPoint: Point) {
    this._cacheLandmarks.crownPoint = crownPoint;
    this._cacheLandmarks.chinPoint = chinPoint;
    this.storage.setItem(LANDMARKS_KEY, this._cacheLandmarks);
    //this.landmarksUpdated.emit(this._cacheLandmarks);
  }

  persistResults() {
    this.storage.setItem(LANDMARKS_KEY, this._cacheLandmarks);
  }

  setPrintOutput(pr: File) {
    this._printOutputFile = pr;
  }

  getPrintOutput() {
    return this._printOutputFile;
  }

  async loadPreviousSession() {
    const lastImageName = this.storage.getItem(INPUT_IMAGE_NAME);
    if (!lastImageName) {
      return;
    }
    if (!this.storage.fileExists(lastImageName)) {
      return;
    }

    const blob = await this.storage.loadFile(lastImageName);
    this._isRestoringSession = true;
    this.loadImageInMemory(blob);

    // const imageLoadResults = this.storage.getItem('imageResult') as ImageLoadResult;
    // if (!imageLoadResults) {
    //   return;
    // }
    // imageLoadResults.imageData = null;
    // const imgKey = imageLoadResults.imgKey;
    // const landmarks = this.storage.getItem(LANDMARKS_KEY);

    // const blob = await this.storage.loadFile(imgKey);
    // if (blob) {
    //   const blobUrl = this.toSafeResourceUrl(blob);
    //   imageLoadResults.imgDataUrl = blobUrl;
    //   this._cacheImageLoadResult = imageLoadResults;
    //   this._cacheLandmarks = landmarks;

    //   this.imageLoaded.emit(imageLoadResults);
    //   this.landmarksUpdated.emit(this._cacheLandmarks);
    // }
  }

  private async blobToArrayBuffer(blob: Blob | File): Promise<ArrayBuffer> {
    return new Promise((accept, reject) => {
      const fileReader = new FileReader();
      fileReader.onload = function (event) {
        accept(event.target.result as ArrayBuffer);
      };
      fileReader.readAsArrayBuffer(blob);
    });
  }
}
