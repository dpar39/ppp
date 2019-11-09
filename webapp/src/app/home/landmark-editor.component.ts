import { Component, OnInit, ElementRef, HostListener } from '@angular/core';
import { Input, Output, EventEmitter } from '@angular/core';

import interact from 'interactjs';
import { CrownChinPointPair } from '../model/datatypes';
import { middlePoint, Point, rotatedRectangle, pointsAtDistanceNorm } from '../model/geometry';
import { ImageLoadResult } from '../services/backend.service';
import { PhotoDimensions, getCroppingCenter } from '../model/photodimensions';

@Component({
  selector: 'app-landmark-editor',
  template: `
    <div id="viewport">
      <img
        id="inputPhoto"
        alt="Input Image"
        title="Input picture"
        [src]="getImageDataUrl()"
        (load)="imageLoaded()"
      />

      <svg class="box" [style.visibility]="landmarkVisibility" pointer-events="none">
        <defs>
          <mask id="mask" x="0" y="0" width="100%" height="100%">
            <rect x="0" y="0" width="100%" height="100%" fill="#ffffff" />
            <rect id="cropArea" x="0" y="0" width="200" height="200" fill="#000" />
          </mask>
        </defs>
        <rect x="0" y="0" width="1000" height="1000" fill-opacity="0.4" mask="url(#mask)" />
        <rect id="cropRect" x="0" y="0" width="200" height="200" fill="none" />
        <line id="middleLine" x1="0" y1="0" x2="200" y2="200" class="annotation" />
        <line id="crownLine" x1="0" y1="0" x2="200" y2="200" class="annotation" />
        <line id="chinLine" x1="0" y1="0" x2="200" y2="200" class="annotation" />
        <ellipse id="faceEllipse" cx="100" cy="50" rx="100" ry="50" fill="none" class="annotation" />
      </svg>

      <div class="landmark" id="crownMark" [style.visibility]="landmarkVisibility"></div>
      <div class="landmark" id="chinMark" [style.visibility]="landmarkVisibility"></div>
    </div>
  `,
  styles: [
    `
      .landmark {
        width: 1em;
        height: 1em;
        border: 1px solid green;
        border-radius: 50%;
        background-color: greenyellow;
        opacity: 0.6;
        float: left;
        position: absolute;
        visibility: hidden;
      }

      #inputPhoto {
        position: absolute;
        z-index: 0;
      }

      #viewport {
        position: relative;
        max-height: 80vh;
        min-height: 40vh;
        border: 1px solid #363434;
        border-radius: 5px;
        margin: 0 auto;
        background: #333;
      }

      @media (max-aspect-ratio: 5/4) {
        #viewport {
          height: 46vh;
        }
      }

      @media (min-aspect-ratio: 4/5) {
        #viewport {
          height: 40vh;
        }
      }

      .box {
        position: absolute;
        width: 100%;
        height: 100%;
        border-radius: 5px;
      }

      .annotation {
        stroke: orange;
        stroke-width: 2;
        stroke-dasharray: 5, 5;
        stroke-opacity: 0.7;
      }
    `
  ]
})
export class LandmarkEditorComponent implements OnInit {
  @Input()
  set inputPhoto(value: ImageLoadResult) {
    if (this._imgElmt && this._imgElmt.src) {
      URL.revokeObjectURL(this._imgElmt.src);
    }
    this._imageLoadResult = value;
    this._imageWidth = 1;
    this._imageHeight = 1;
    if (this._imgElmt) {
      this._imgElmt.style.visibility = 'hidden';
    }
  }
  get inputPhoto(): ImageLoadResult {
    return this._imageLoadResult;
  }

  @Input()
  set crownChinPointPair(value: CrownChinPointPair) {
    if (value) {
      this.crownPoint = value.crownPoint;
      this.chinPoint = value.chinPoint;
    }
    this.renderLandMarks();
  }

  @Input()
  set photoDimensions(value: PhotoDimensions) {
    this._photoDimensions = value;
    this.renderLandMarks();
  }

  constructor(private _el: ElementRef) {
    this.crownPoint = new Point(0, 0);
    this.chinPoint = new Point(0, 0);
  }
  private _imageWidth = 0;
  private _imageHeight = 0;
  private _viewPortWidth = 0;
  private _viewPortHeight = 0;

  private _xLeft = 0; // Offset in screen pixels
  private _yTop = 0;
  private _zoom = 1;
  private _ratio = 0; // Ratio between image pixels and screen pixels

  // Annotation elements
  private _imgElmt: any = null;
  private _viewPortElmt: any = null;
  private _crownMarkElmt: any = null;
  private _chinMarkElmt: any = null;
  private _middleLine: any = null;
  private _crownLine: any = null;
  private _chinLine: any = null;
  private _faceEllipse: any = null;
  private _cropArea: any = null;
  private _cropRect: any = null;

  chinPoint: Point;
  crownPoint: Point;

  private _photoDimensions: PhotoDimensions;

  landmarkVisibility = 'hidden';

  private _imageLoadResult: ImageLoadResult;

  @Output()
  edited: EventEmitter<any> = new EventEmitter<any>();

  imageLoaded() {
    this._imageWidth = this._imgElmt.naturalWidth;
    this._imageHeight = this._imgElmt.naturalHeight;
    if (this._imageWidth > 100 && this._imageHeight > 100) {
      this._imgElmt.style.visibility = 'visible';
      this.calculateViewPort();
      this.zoomFit();
      this.renderImage();
      this.renderLandMarks();
    }
  }

  ngOnInit() {
    const thisEl = this._el.nativeElement;
    this._imgElmt = thisEl.querySelector('#inputPhoto');
    this._viewPortElmt = thisEl.querySelector('#viewport');
    this._crownMarkElmt = thisEl.querySelector('#crownMark');
    this._chinMarkElmt = thisEl.querySelector('#chinMark');

    this._middleLine = thisEl.querySelector('#middleLine');
    this._crownLine = thisEl.querySelector('#crownLine');
    this._chinLine = thisEl.querySelector('#chinLine');

    this._faceEllipse = thisEl.querySelector('#faceEllipse');
    this._cropArea = thisEl.querySelector('#cropArea');
    this._cropRect = thisEl.querySelector('#cropRect');

    interact('.landmark').draggable({
      // enable inertial throwing
      inertia: false,
      // keep the element within the area of it's parent
      modifiers: [
        interact.modifiers.restrictRect({
          restriction: 'parent',
          endOnly: true,
          elementRect: { top: 0, left: 0, bottom: 1, right: 1 }
        })
      ],

      // call this function on every dragmove event
      onmove: event => {
        const target = event.target;
        // keep the dragged position in the x/y attributes
        const x = (parseFloat(target.getAttribute('x')) || 0) + event.dx;
        const y = (parseFloat(target.getAttribute('y')) || 0) + event.dy;
        // translate the element
        this.translateElement(target, new Point(x, y));
        this.renderAnnotations();
      },
      // call this function on every dragend event
      onend: event => {
        this.updateLandMarks();
      }
    });

    // var angleScale = {
    //   angle: 0,
    //   scale: 1
    // }
    // var resetTimeout;

    // function dragMoveListener (event) {
    //   var target = event.target
    //   // keep the dragged position in the data-x/data-y attributes
    //   var x = (parseFloat(target.getAttribute('data-x')) || 0) + event.dx
    //   var y = (parseFloat(target.getAttribute('data-y')) || 0) + event.dy

    //   // translate the element
    //   target.style.webkitTransform =
    //     target.style.transform =
    //       'translate(' + x + 'px, ' + y + 'px)'

    //   // update the posiion attributes
    //   target.setAttribute('data-x', x)
    //   target.setAttribute('data-y', y)
    // }

    // const scaleElement = this._imgElmt;
    // interact(this._imgElmt)
    //   .gesturable({
    //     onstart: function(event) {
    //       angleScale.angle -= event.angle;

    //       clearTimeout(resetTimeout);

    //       scaleElement.classList.remove('reset');
    //     },
    //     onmove: function(event) {
    //       // document.body.appendChild(new Text(event.scale))
    //       var currentAngle = event.angle + angleScale.angle;
    //       var currentScale = event.scale * angleScale.scale;

    //       scaleElement.style.webkitTransform = scaleElement.style.transform =
    //         'rotate(' + currentAngle + 'deg)' + 'scale(' + currentScale + ')';

    //       // uses the dragMoveListener from the draggable demo above
    //       dragMoveListener(event);
    //     },
    //     onend: function(event) {
    //       angleScale.angle = angleScale.angle + event.angle;
    //       angleScale.scale = angleScale.scale * event.scale;

    //       resetTimeout = setTimeout(reset, 1000);
    //       scaleElement.classList.add('reset');
    //     }
    //   })
    //   .draggable({ onmove: dragMoveListener });

    // function reset() {
    //   scaleElement.style.webkitTransform = scaleElement.style.transform = 'scale(1)';

    //   angleScale.angle = 0;
    //   angleScale.scale = 1;
    // }
  }

  getImageDataUrl() {
    if (!this._imageLoadResult || !this._imageLoadResult.imgDataUrl) {
      return 'data:image/gif;base64,R0lGODlhAQABAAAAACH5BAEKAAEALAAAAAABAAEAAAICTAEAOw==';
    }
    return this._imageLoadResult.imgDataUrl;
  }

  zoomFit(): void {
    const xRatio = this._viewPortWidth / this._imageWidth;
    const yRatio = this._viewPortHeight / this._imageHeight;
    this._ratio = xRatio < yRatio ? xRatio : yRatio;
    this._xLeft = this._viewPortWidth / 2 - (this._ratio * this._imageWidth) / 2;
    this._yTop = this._viewPortHeight / 2 - (this._ratio * this._imageHeight) / 2;
  }

  calculateViewPort(): void {
    if (!this._viewPortElmt) {
      return;
    }
    this._viewPortWidth = this._viewPortElmt.clientWidth;
    this._viewPortHeight = this._viewPortElmt.clientHeight;
  }

  @HostListener('window:resize', ['$event'])
  onResize(event) {
    this.calculateViewPort();
    this.zoomFit();
    this.renderImage();
    this.renderLandMarks();
  }

  renderImage(): void {
    if (this._imageWidth <= 0 || this._imageHeight <= 0) {
      return;
    }
    const xw = this._imageWidth * this._ratio;
    const yh = this._imageHeight * this._ratio;
    this._imgElmt.width = xw;
    this._imgElmt.height = yh;
    this.translateElement(this._imgElmt, new Point(this._xLeft, this._yTop));
  }

  translateElement(elmt: any, pt: Point) {
    // Translate the element position
    elmt.style.transform = elmt.style.webkitTransform = `translate(${pt.x}px, ${pt.y}px)`;
    // Store it in attached properties
    elmt.setAttribute('x', pt.x);
    elmt.setAttribute('y', pt.y);
  }

  setLandMarks(crownPoint: Point, chinPoint: Point): void {
    this.crownPoint = crownPoint;
    this.chinPoint = chinPoint;
    this.renderLandMarks();
  }

  renderLandMarks(): void {
    if (
      this.crownPoint &&
      this.crownPoint.x &&
      this.crownPoint.y &&
      this.chinPoint &&
      this.chinPoint.x &&
      this.chinPoint.y &&
      this._imageWidth > 100 &&
      this._imageHeight > 100
    ) {
      const p1 = this.pixelToScreen(this._crownMarkElmt, this.crownPoint);
      const p2 = this.pixelToScreen(this._chinMarkElmt, this.chinPoint);
      this.translateElement(this._crownMarkElmt, p1);
      this.translateElement(this._chinMarkElmt, p2);
      this.renderAnnotations();
      this.landmarkVisibility = 'visible';
    } else {
      this.landmarkVisibility = 'hidden';
    }
  }

  pixelToScreen(elmt: any, pt: Point): Point {
    return new Point(
      this._xLeft + pt.x * this._ratio - elmt.clientWidth / 2,
      this._yTop + pt.y * this._ratio - elmt.clientHeight / 2
    );
  }

  screenToPixel(pt: Point | any, round = false): Point {
    if (pt.x === undefined || pt.y == undefined) {
      pt = this.getMarkScreenCenter(pt);
    }
    const xPrime = (pt.x - this._xLeft) / this._ratio;
    const yPrime = (pt.y - this._yTop) / this._ratio;
    if (round) {
      return new Point(Math.round(xPrime), Math.round(yPrime));
    }
    return new Point(xPrime, yPrime);
  }

  getMarkScreenCenter(elmt: any) {
    const x = parseFloat(elmt.getAttribute('x')) + (elmt.clientWidth + 0.5) / 2.0;
    const y = parseFloat(elmt.getAttribute('y')) + (elmt.clientHeight + 0.5) / 2.0;
    return new Point(x, y);
  }

  private _setRotatedRect(svgElmt: any, center: Point, w: number, h: number, angle: number) {
    svgElmt.setAttribute('x', center.x - w / 2);
    svgElmt.setAttribute('y', center.y - h / 2);
    svgElmt.setAttribute('width', w);
    svgElmt.setAttribute('height', h);
    svgElmt.setAttribute('transform', `rotate(${angle}, ${center.x}, ${center.y})`);
  }

  private _renderSegment(svdElmt: any, p1: Point, p2: Point){
    svdElmt.setAttribute('x1', p1.x);
    svdElmt.setAttribute('y1', p1.y);
    svdElmt.setAttribute('x2', p2.x);
    svdElmt.setAttribute('y2', p2.y);
  }

  renderAnnotations() {
    const p1 = this.getMarkScreenCenter(this._crownMarkElmt);
    const p2 = this.getMarkScreenCenter(this._chinMarkElmt);

    // Render middle line
    this._renderSegment(this._middleLine, p1, p2);

    const faceHeight = p1.distTo(p2);
    const crownSegment = pointsAtDistanceNorm(p1, p2, faceHeight*0.5, p1);
    this._renderSegment(this._crownLine, crownSegment[0], crownSegment[1]);

    const chinSegment = pointsAtDistanceNorm(p1, p2, faceHeight*0.5, p2);
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
    if (!this._photoDimensions) {
      return;
    }

    const cropCenter = getCroppingCenter(this._photoDimensions, p1, p2);
    const scale = faceHeight / this._photoDimensions.faceHeight;
    const dx = this._photoDimensions.pictureHeight * scale;
    const dy = this._photoDimensions.pictureWidth * scale;

    this._setRotatedRect(this._cropArea, cropCenter, dx, dy, angleDeg);
    this._setRotatedRect(this._cropRect, cropCenter, dx, dy, angleDeg);
    const points = rotatedRectangle(cropCenter, dx, dy, angleRad);
    const invalidCrop = points.some(pt => {
      const ptPix = this.screenToPixel(pt);
      return ptPix.x < 0 || ptPix.x > this._imageWidth || ptPix.y < 0 || ptPix.y > this._imageHeight;
    });
    this._cropRect.setAttribute('stroke', invalidCrop ? 'red' : 'green');
  }

  updateLandMarks() {
    this.crownPoint = this.screenToPixel(this._crownMarkElmt, true);
    this.chinPoint = this.screenToPixel(this._chinMarkElmt, true);
    this.edited.emit(new CrownChinPointPair(this.crownPoint, this.chinPoint));
  }
}
