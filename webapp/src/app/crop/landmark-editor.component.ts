import { Component, OnInit, ElementRef, HostListener, SecurityContext, OnDestroy } from '@angular/core';
import { Platform } from '@ionic/angular';

import interact from 'interactjs';
import { CrownChinPointPair } from '../model/datatypes';
import { middlePoint, Point, pointsAtDistanceNorm, RotatedRect } from '../model/geometry';
import { ImageLoadResult, BackEndService } from '../services/backend.service';
import { getCroppingCenter, getCroppingRectangle } from '../model/photodimensions';
import { DomSanitizer } from '@angular/platform-browser';
import { PhotoStandardService } from '../services/photo-standard.service';
import { Subscription } from 'rxjs';

const DEVELOP = false;

@Component({
  selector: 'app-landmark-editor',
  template: `
    <canvas class="box-canvas" id="canvasInputPhoto"></canvas>
    <div id="viewport" (wheel)="onMouseWheel($event)" (mousemove)="onMouseMove($event)">
      <ion-spinner *ngIf="landmarkVisibility == 'hidden'" name="lines" color="medium"></ion-spinner>
      <svg class="box" [style.visibility]="landmarkVisibility">
        <style>
          .landmark {
            stroke-width: 2px;
            stroke-linecap: round;
            fill-opacity: 0.5;
            cursor: default;
          }
          #crownMark {
            fill: blueviolet;
            stroke: white;
          }
          #chinMark {
            fill: indianred;
            stroke: white;
          }

          .landmark:hover {
            cursor: grab;
            stroke-width: 3px;
            stroke: orange;
            fill-opacity: 0.7;
          }

          .annotation {
            stroke: lightgray;
            stroke-width: 1.5;
            stroke-dasharray: 5, 5;
            stroke-opacity: 0.7;
          }

          .dimension-text {
            font-family: sans-serif;
            font-size: 1.2em;
            fill: lightgray;
          }
          .dimension-line {
            stroke-width: 1;
            stroke: lightgray;
          }
          #cropRect {
            stroke-width: 2px;
            filter: url(#shadow);
          }
          .text {
            fill: white;
            text-shadow: 4px 4px 8px black;
          }
          #crownText {
            alignment-baseline: text-top;
          }
          #chinText {
            dominant-baseline: hanging;
            alignment-baseline: bottom;
          }
        </style>
        <!--image id="inputPhoto" x="0" y="0" width="300px" height="300px" [attr.xlink:href]="getImageSrcUrl()" /-->
        <defs>
          <filter id="glow" x="-30%" y="-30%" width="160%" height="160%">
            <feGaussianBlur stdDeviation="10 10" result="glow" />
            <feMerge>
              <feMergeNode in="glow" />
              <feMergeNode in="glow" />
              <feMergeNode in="glow" />
            </feMerge>
          </filter>
          <filter id="shadow">
            <feDropShadow dx="0" dy="0.2" stdDeviation="0.2" flood-color="white" />
          </filter>
          <mask id="mask" x="0" y="0" width="100%" height="100%">
            <rect id="imageArea" x="0" y="0" width="100vw" height="100vh" fill="#ffffff" />
            <rect id="cropArea" x="0" y="0" width="200" height="200" fill="#000" />
          </mask>
        </defs>
        <rect x="0" y="0" width="100vw" height="100vh" fill-opacity="0.4" mask="url(#mask)" />
        <rect id="cropRect" x="0" y="0" width="200" height="200" fill="none" />

        <!--  Photo dimensions annotations -->
        <line id="heightLine" x1="0" y1="0" x2="0" y2="0" class="dimension-line" />
        <text id="heightText" x="0" y="0" class="dimension-text" text-anchor="middle"></text>
        <line id="widthLine" x1="0" y1="0" x2="0" y2="0" class="dimension-line" />
        <text id="widthText" x="0" y="0" class="dimension-text" text-anchor="middle"></text>

        <line id="middleLine" x1="0" y1="0" x2="200" y2="200" class="annotation" />
        <line id="crownLine" x1="0" y1="0" x2="200" y2="200" class="annotation" />
        <line id="chinLine" x1="0" y1="0" x2="200" y2="200" class="annotation" />

        <ellipse id="faceEllipse" cx="100" cy="50" rx="100" ry="50" fill="none" class="annotation" />

        <circle class="landmark" id="crownMark" />
        <circle class="landmark" id="chinMark" />

        <text class="text" id="crownText">Crown</text>
        <text class="text" id="chinText">Chin</text>
      </svg>
    </div>
  `,
  styles: [
    `
      #viewport {
        position: relative;
        height: 100%;
        border: 1px solid #363434;
        border-radius: 5px;
        margin: 0 auto;
        background: transparent;
        touch-action: none;
        user-select: none;
        cursor: default;
        overflow: hidden;
      }

      .box {
        position: absolute;
        background: transparent;
        width: 100vw;
        height: 100vh;
        border-radius: 5px;
      }

      .box-canvas {
        position: absolute;
        width: 100%;
        height: 100%;
        background: #303030;
      }
      ion-spinner {
        position: absolute;
        left: 50%;
        top: 50%;
        transform: translate(-50%) scale(5);
      }
    `,
  ],
})
export class LandmarkEditorComponent implements OnInit, OnDestroy {
  private _imageLoadedSubscription: Subscription;
  private _landmarksUpdatedSubscription: Subscription;
  private _psSubscription: Subscription;

  private _canvasElmt: HTMLCanvasElement = null;
  private _inputImage: HTMLImageElement = null;
  private _inputBitmap: ImageBitmap = null;

  private _imageWidth = 0;
  private _imageHeight = 0;
  private _viewPortWidth = 0;
  private _viewPortHeight = 0;

  private _xLeft = 0; // Offset in screen pixels
  private _yTop = 0;
  private _zoom = 1;
  private _ratio = 0; // Ratio between image pixels and screen pixels

  // Annotation elements
  private _viewPortElmt: any = null;
  private _crownMarkElmt: any = null;
  private _chinMarkElmt: any = null;
  private _crownChinMarkSize = 24;
  private _middleLine: any = null;
  private _crownLine: any = null;
  private _chinLine: any = null;
  private _faceEllipse: any = null;
  private _cropArea: any = null;
  private _imageArea: any = null;
  private _cropRect: any = null;
  private _crownText: any = null;
  private _chinText: any = null;

  private _heightText: any;
  private _heightLine: any;
  private _widthText: any;
  private _widthLine: any;

  chinPoint: Point;
  crownPoint: Point;

  landmarkVisibility = 'hidden';

  private _imageLoadResult: ImageLoadResult;

  constructor(
    private _el: ElementRef,
    private _platform: Platform,
    private _sanitizer: DomSanitizer,
    private _psService: PhotoStandardService,
    private _beService: BackEndService
  ) {
    this.setLandmarks(null);
    if (DEVELOP) {
      this._imageLoadResult = new ImageLoadResult('', 'assets/icons/icon-512x512.png', null, null);
      this.crownPoint = new Point(128, 15);
      this.chinPoint = new Point(256, 220);
    }
  }

  onInputPhotoReady() {
    if (this._isViewportReady()) {
      this._zoomFit();
      this._zoomToCrop(0.2);
      this._renderImage();
      this._renderLandMarks();
    }
    this._psService.createCroppedImage(this._getImage(), this.crownPoint, this.chinPoint);
  }

  setInputPhoto(value: ImageLoadResult) {
    this.landmarkVisibility = 'hidden';
    this._inputBitmap = null;
    this._inputImage = null;
    if (value || !DEVELOP) {
      this._imageLoadResult = value;
    }
    if (!value) {
      this.setLandmarks(null);
    }
    if (!this._imageLoadResult) {
      return;
    }

    const imgData = this._imageLoadResult.imageData;
    if (imgData) {
      createImageBitmap(imgData).then((bm) => {
        this._inputBitmap = bm;
        this.onInputPhotoReady();
      });
    } else if (this._imageLoadResult.imgDataUrl) {
      this._inputImage = new Image();
      this._inputImage.onload = (e) => {
        this.onInputPhotoReady();
      };
      this._inputImage.src = this._getImageSrcUrl();
    }
  }

  setLandmarks(lm: CrownChinPointPair) {
    if (lm) {
      this.crownPoint = new Point(lm.crownPoint.x, lm.crownPoint.y);
      this.chinPoint = new Point(lm.chinPoint.x, lm.chinPoint.y);
      this._zoomToCrop(0.2);
      this._renderImage();
      this._renderLandMarks(false);
      this._psService.createCroppedImage(this._getImage(), this.crownPoint, this.chinPoint);
    } else {
      this.crownPoint = this.chinPoint = new Point(0, 0);
      this.landmarkVisibility = 'hidden';
    }
  }

  subscribe() {
    this._psSubscription = this._psService.photoStandardSelected.subscribe(() => this._updateLandMarks());
    this._landmarksUpdatedSubscription = this._beService.landmarksUpdated.subscribe((lm: CrownChinPointPair) =>
      this.setLandmarks(lm)
    );
    this._imageLoadedSubscription = this._beService.imageLoaded.subscribe((imageLoadResult: ImageLoadResult) =>
      this.setInputPhoto(imageLoadResult)
    );
  }

  unsubscribe() {
    this._imageLoadedSubscription?.unsubscribe();
    this._landmarksUpdatedSubscription?.unsubscribe();
    this._psSubscription?.unsubscribe();
  }

  ngOnDestroy() {
    this.unsubscribe();
    interact('.landmark').unset();
    interact('#viewport').unset();
  }

  _isViewportReady() {
    const bm = this._inputBitmap;
    if (bm && bm.width && bm.height) {
      this._imageWidth = bm.width;
      this._imageHeight = bm.height;
    } else if (this._inputImage != null) {
      this._imageWidth = this._inputImage.naturalWidth;
      this._imageHeight = this._inputImage.naturalHeight;
    }
    this._viewPortWidth = this._viewPortElmt?.clientWidth || 0;
    this._viewPortHeight = this._viewPortElmt?.clientHeight || 0;
    return this._imageWidth > 1 && this._imageHeight > 1 && this._viewPortWidth > 0 && this._viewPortHeight > 0;
  }

  ngOnInit() {
    const thisEl = this._el.nativeElement;
    this._canvasElmt = thisEl.querySelector('#canvasInputPhoto');

    this._viewPortElmt = thisEl.querySelector('#viewport');
    this._crownMarkElmt = thisEl.querySelector('#crownMark');
    this._chinMarkElmt = thisEl.querySelector('#chinMark');

    if (this._platform.is('mobile')) {
      this._crownChinMarkSize = 32;
    }
    this._crownMarkElmt.setAttribute('r', this._crownChinMarkSize / 2);
    this._chinMarkElmt.setAttribute('r', this._crownChinMarkSize / 2);

    this._middleLine = thisEl.querySelector('#middleLine');
    this._crownLine = thisEl.querySelector('#crownLine');
    this._chinLine = thisEl.querySelector('#chinLine');

    this._faceEllipse = thisEl.querySelector('#faceEllipse');
    this._cropArea = thisEl.querySelector('#cropArea');
    this._imageArea = thisEl.querySelector('#imageArea');

    this._cropRect = thisEl.querySelector('#cropRect');

    this._heightLine = thisEl.querySelector('#heightLine');
    this._heightText = thisEl.querySelector('#heightText');
    this._widthLine = thisEl.querySelector('#widthLine');
    this._widthText = thisEl.querySelector('#widthText');

    this._crownText = thisEl.querySelector('#crownText');
    this._chinText = thisEl.querySelector('#chinText');

    interact('.landmark').draggable({
      // enable inertial throwing
      inertia: false,
      // keep the element within the area of it's parent
      modifiers: [
        interact.modifiers.restrictRect({
          restriction: 'parent',
          endOnly: true,
          elementRect: { top: 0, left: 0, bottom: 1, right: 1 },
        }),
      ],

      // call this function on every dragmove event
      onmove: (event) => {
        const target = event.target;
        // keep the dragged position in the x/y attributes
        const x = (parseFloat(target.getAttribute('cx')) || 0) + event.dx;
        const y = (parseFloat(target.getAttribute('cy')) || 0) + event.dy;
        // translate the element
        this._positionLandmark(target, new Point(x, y));
        this._renderAnnotations();
      },
      // call this function on every dragend event
      onend: (event) => {
        this.crownPoint = this._screenToPixel(this._crownMarkElmt, true);
        this.chinPoint = this._screenToPixel(this._chinMarkElmt, true);
        this._updateLandMarks(true);
      },
    });

    let zoomStart: number;
    interact('#viewport')
      .draggable({
        inertia: false,
        onmove: (event) => {
          this._xLeft += event.dx;
          this._yTop += event.dy;
          this._renderImage();
          this._renderLandMarks();
        },
      })
      .on('doubletap', (e) => {
        this._zoomFit();
        this._renderImage();
        this._renderLandMarks();
      })
      .gesturable({
        onstart: (e) => {
          zoomStart = this._zoom;
        },
        onmove: (e) => {
          const newZoom = zoomStart * e.scale;
          this.setZoom(newZoom, e.clientX, e.clientY);
        },
      });

    this.setInputPhoto(this._beService.getCacheImageLoadResult());
    this.setLandmarks(this._beService.getCacheLandmarks());
    this.subscribe();
  }

  onMouseMove(e) {
    if (!DEVELOP) {
      return;
    }
    const clientRect = this._viewPortElmt.getBoundingClientRect();
    const x = e.clientX - clientRect.left;
    const y = e.clientY - clientRect.top;
    document.getElementById('title').innerHTML = `<span>${x}, ${y}</span>`;
  }

  onMouseWheel(e) {
    e.preventDefault();
    if (this._imageHeight < 100 || this._imageHeight < 100) {
      return;
    }
    const clientRect = this._viewPortElmt.getBoundingClientRect();
    const xClient = e.clientX - clientRect.left;
    const yClient = e.clientY - clientRect.top;
    const scale = e.deltaY < 0 ? 1.1 : 1 / 1.1;
    const newZoom = this._zoom * scale;
    this.setZoom(newZoom, xClient, yClient);
  }

  setZoom(newZoom: number, cx: number, cy: number) {
    if (newZoom <= 1) {
      newZoom = 1;
    }
    if (newZoom > 32) {
      newZoom = 32;
    }

    const imgPos = this._screenToPixel(new Point(cx, cy));
    const effectiveScale = this._ratio * (this._zoom - newZoom);

    this._yTop += imgPos.y * effectiveScale;
    this._xLeft += imgPos.x * effectiveScale;
    this._zoom = newZoom;

    this._renderImage();
    this._renderLandMarks();
  }

  private _getImageSrcUrl(): any {
    if (!this._imageLoadResult || !this._imageLoadResult.imgDataUrl) {
      return 'data:image/gif;base64,R0lGODlhAQABAAAAACH5BAEKAAEALAAAAAABAAEAAAICTAEAOw==';
    }

    return this._sanitizer.sanitize(SecurityContext.URL, this._imageLoadResult.imgDataUrl);
  }

  _zoomFit(): void {
    if (!this._isViewportReady()) {
      return;
    }

    this._zoom = 1;
    this._ratio = this._computeMinScale(this._imageWidth, this._imageHeight);
    this._xLeft = this._viewPortWidth / 2 - (this._ratio * this._imageWidth) / 2;
    this._yTop = this._viewPortHeight / 2 - (this._ratio * this._imageHeight) / 2;
  }

  private _zoomToCrop(padRatio: number) {
    const photoDimensions = this._psService.getSelectedStandard().dimensions;
    if (!photoDimensions || this.chinPoint.equals(this.crownPoint)) {
      return;
    }
    const rotRect = getCroppingRectangle(photoDimensions, this.crownPoint, this.chinPoint);
    const bbRect = rotRect.boundingBox();
    const vwPix = bbRect.width * (1 + padRatio);
    const vhPix = bbRect.height * (1 + padRatio);

    const newScale = this._computeMinScale(vwPix, vhPix);
    const newZoom = newScale / this._ratio;

    this._xLeft = this._viewPortWidth / 2 - bbRect.cx * newScale;
    this._yTop = this._viewPortHeight / 2 - bbRect.cy * newScale;
    this._zoom = newZoom;
  }

  private _computeMinScale(width, height) {
    const xRatio = this._viewPortWidth / width;
    const yRatio = this._viewPortHeight / height;
    return xRatio < yRatio ? xRatio : yRatio;
  }

  @HostListener('window:resize', ['$event'])
  onResize(event) {
    if (!this._isViewportReady()) {
      return;
    }
    this._zoomFit();
    this._renderImage();
    this._renderLandMarks();
  }

  @HostListener('contextmenu', ['$event'])
  onRightClick(event) {
    event.preventDefault();
  }

  private _positionLandmark(elmt: any, pt: Point) {
    elmt.setAttribute('cx', pt.x.toString());
    elmt.setAttribute('cy', pt.y.toString());
  }

  private _renderLandMarks(lmMoved = false): void {
    if (
      this.crownPoint &&
      this.crownPoint.x &&
      this.crownPoint.y &&
      this.chinPoint &&
      this.chinPoint.x &&
      this.chinPoint.y &&
      this._imageWidth > 10 &&
      this._imageHeight > 10 &&
      this._ratio > 0
    ) {
      if (!lmMoved) {
        const p1 = this._pixelToScreen(this.crownPoint);
        const p2 = this._pixelToScreen(this.chinPoint);
        this._positionLandmark(this._crownMarkElmt, p1);
        this._positionLandmark(this._chinMarkElmt, p2);
      }
      this._renderAnnotations();
      this.landmarkVisibility = 'visible';
    } else {
      this.landmarkVisibility = 'hidden';
    }
  }

  private _pixelToScreen(pt: Point): Point {
    return new Point(this._xLeft + pt.x * this._ratio * this._zoom, this._yTop + pt.y * this._ratio * this._zoom);
  }

  private _screenToPixel(pt: Point | any, round = false): Point {
    if (pt.x === undefined || pt.y === undefined) {
      pt = this._getMarkScreenCenter(pt);
    }
    const xPrime = (pt.x - this._xLeft) / this._ratio / this._zoom;
    const yPrime = (pt.y - this._yTop) / this._ratio / this._zoom;
    if (round) {
      return new Point(Math.round(xPrime), Math.round(yPrime));
    }
    return new Point(xPrime, yPrime);
  }

  private _getMarkScreenCenter(elmt: any) {
    const x = parseFloat(elmt.getAttribute('cx'));
    const y = parseFloat(elmt.getAttribute('cy'));
    return new Point(x, y);
  }

  private _setRotatedRect(svgElmt: any, center: Point, w: number, h: number, angle: number) {
    svgElmt.setAttribute('x', center.x - w / 2);
    svgElmt.setAttribute('y', center.y - h / 2);
    svgElmt.setAttribute('width', w);
    svgElmt.setAttribute('height', h);
    svgElmt.setAttribute('transform', `rotate(${angle}, ${center.x}, ${center.y})`);
  }

  private _renderSegment(svdElmt: any, p1: Point, p2: Point) {
    svdElmt.setAttribute('x1', p1.x);
    svdElmt.setAttribute('y1', p1.y);
    svdElmt.setAttribute('x2', p2.x);
    svdElmt.setAttribute('y2', p2.y);
  }

  private _renderDimensionText(svgTextElmt: any, svgLineElmt: any, p1: Point, p2: Point, text: string) {
    const p1s = pointsAtDistanceNorm(p1, p2, 10, p1);
    const p2s = pointsAtDistanceNorm(p1, p2, 10, p2);

    p1 = p1s[0];
    p2 = p2s[0];

    const pc = middlePoint(p1, p2);

    svgTextElmt.textContent = text;
    this._renderSegment(svgLineElmt, p1, p2);

    const angleRad = p2.angle(p1);
    let angleDeg = (angleRad * 180) / Math.PI;
    let alignmentBaseline = 'hanging';
    if (angleDeg > 90) {
      angleDeg -= 180;
      alignmentBaseline = 'hanging';
    } else if (angleDeg < -90) {
      angleDeg += 180;
      alignmentBaseline = 'hanging';
    }

    svgTextElmt.setAttribute('x', pc.x);
    svgTextElmt.setAttribute('y', pc.y);
    svgTextElmt.setAttribute('alignment-baseline', alignmentBaseline);
    svgTextElmt.setAttribute('transform', `rotate(${angleDeg}, ${pc.x}, ${pc.y})`);
  }

  private _renderImage() {
    if (!this._isViewportReady()) return;
    const xw = this._imageWidth * this._ratio * this._zoom;
    const yh = this._imageHeight * this._ratio * this._zoom;

    const ctx = this._canvasElmt.getContext('2d');
    this._canvasElmt.width = this._viewPortWidth;
    this._canvasElmt.height = this._viewPortHeight;
    ctx.clearRect(0, 0, this._canvasElmt.width, this._canvasElmt.height);
    ctx.drawImage(this._getImage(), this._xLeft, this._yTop, xw, yh);
  }

  private _renderAnnotations() {
    const p1 = this._getMarkScreenCenter(this._crownMarkElmt);
    const p2 = this._getMarkScreenCenter(this._chinMarkElmt);

    // Render middle line
    this._renderSegment(this._middleLine, p1, p2);

    const faceHeight = p1.distTo(p2);
    const crownSegment = pointsAtDistanceNorm(p1, p2, faceHeight * 0.4, p1);
    this._renderSegment(this._crownLine, crownSegment[0], crownSegment[1]);

    const chinSegment = pointsAtDistanceNorm(p1, p2, faceHeight * 0.4, p2);
    this._renderSegment(this._chinLine, chinSegment[0], chinSegment[1]);

    // Render face ellipse
    const ra = faceHeight / 2;
    const rb = 0.68 * ra;
    const pc = middlePoint(p1, p2);
    const angleRad = p2.angle(p1);
    const angleDeg = (angleRad * 180) / Math.PI;
    this._faceEllipse.setAttribute('rx', ra);
    this._faceEllipse.setAttribute('ry', rb);
    this._faceEllipse.setAttribute('cx', pc.x);
    this._faceEllipse.setAttribute('cy', pc.y);
    this._faceEllipse.setAttribute('transform', `rotate(${angleDeg}, ${pc.x}, ${pc.y})`);

    // Render photo cropping rectangle
    const photoDimensions = this._psService.getSelectedStandard().dimensions;
    if (!photoDimensions) {
      return;
    }

    // Render crown and chin text
    this._crownText.setAttribute('x', p1.x);
    this._crownText.setAttribute('y', p1.y);
    this._crownText.setAttribute('transform', `rotate(${angleDeg - 90}, ${p1.x}, ${p1.y}) translate(20,-5)`);

    this._chinText.setAttribute('x', p2.x);
    this._chinText.setAttribute('y', p2.y);
    this._chinText.setAttribute('transform', `rotate(${angleDeg - 90}, ${p2.x}, ${p2.y}) translate(20, 5)`);

    const cropCenter = getCroppingCenter(photoDimensions, p1, p2);
    const scale = faceHeight / photoDimensions.faceHeight;
    const dx = photoDimensions.pictureHeight * scale;
    const dy = photoDimensions.pictureWidth * scale;

    const svgElmt = this._imageArea;
    svgElmt.setAttribute('x', this._xLeft);
    svgElmt.setAttribute('y', this._yTop);
    svgElmt.setAttribute('width', this._imageWidth * this._ratio * this._zoom);
    svgElmt.setAttribute('height', this._imageHeight * this._ratio * this._zoom);

    this._setRotatedRect(this._cropArea, cropCenter, dx, dy, angleDeg);
    this._setRotatedRect(this._cropRect, cropCenter, dx, dy, angleDeg);

    const rotatedRect = new RotatedRect(cropCenter, dx, dy, angleRad);
    const points = rotatedRect.corners();
    const invalidCrop = points.some((pt) => {
      const ptPix = this._screenToPixel(pt);
      return ptPix.x < 0 || ptPix.x > this._imageWidth || ptPix.y < 0 || ptPix.y > this._imageHeight;
    });
    this._cropRect.setAttribute('stroke', invalidCrop ? 'red' : 'lightgray');

    // this._renderDimensionText(this._heightText, this._heightLine, points[2], points[1], '2in');
    // this._renderDimensionText(this._widthText, this._widthLine, points[3], points[2], '2in');
  }

  private _updateLandMarks(lmMoved = false) {
    this._renderLandMarks(lmMoved);
    this._beService.updateCrownChin(this.crownPoint, this.chinPoint);
    this._psService.createCroppedImage(this._getImage(), this.crownPoint, this.chinPoint);
  }

  private _getImage() {
    return this._inputBitmap || this._inputImage;
  }
}
