import { Component, OnInit, ElementRef, ViewChild, HostListener } from '@angular/core';
import { Input, Output, EventEmitter } from '@angular/core';

import * as $ from 'jquery';
import * as interact from 'interactjs';
import { Point, CrownChinPointPair } from '../model/datatypes';

@Component({
  selector: 'app-landmark-editor',
  template: `
    <div id="container" class="bg-dark">
      <img id="photo" alt="" title="Input picture" [src]="inputPhoto" />
      <div class="landmark" id="crownMark"></div>
      <div class="landmark" id="chinMark"></div>
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

      #photo {
        position: absolute;
      }

      #container {
        max-height: 50vh;
        height: 300px;
        border: 1px solid #363434;
        border-radius: 5px;
        margin: 1em auto;
      }

      img {
        image-orientation: from-image;
      }
    `
  ]
})
export class LandmarkEditorComponent implements OnInit {
  private _imageWidth = 0;
  private _imageHeight = 0;
  private _viewPortWidth = 0;
  private _viewPortHeight = 0;

  private _xleft = 0; // Offset in screen pixels
  private _ytop = 0;
  private _zoom = 0;
  private _ratio = 0; // Ratio between image pixels and screen pixels

  private _imgElmt: any = null;
  private _containerElmt: any = null;
  private _crownMarkElmt: any = null;
  private _chinMarkElmt: any = null;

  chinPoint: Point;
  crownPoint: Point;

  private _inputPhoto = '#';
  @Input()
  set inputPhoto(value: string) {
    const newImg = new Image();
    newImg.onload = () => {
      this._imageWidth = newImg.width;
      this._imageHeight = newImg.height;
      this._inputPhoto = value;
      if (this._imageWidth > 100 && this._imageHeight > 100) {
        this.calculateViewPort();
        this.zoomFit();
        this.renderImage();
        this.renderLandMarks();
      }
    };
    newImg.src = value;
  }
  get inputPhoto(): string {
    return this._inputPhoto;
  }

  landMarksVisible = false;

  @Input()
  set crownChinPointPair(value: CrownChinPointPair) {
    if (value) {
      this.crownPoint = value.crownPoint;
      this.chinPoint = value.chinPoint;
    }
    this.renderLandMarks();
  }

  @Output()
  edited: EventEmitter<any> = new EventEmitter<any>();

  constructor(private el: ElementRef) {
    this.crownPoint = new Point(0, 0);
    this.chinPoint = new Point(0, 0);
  }

  ngOnInit() {
    this._imgElmt = this.el.nativeElement.querySelector('#photo');
    this._containerElmt = this.el.nativeElement.querySelector('#container');
    this._crownMarkElmt = this.el.nativeElement.querySelector('#crownMark');
    this._chinMarkElmt = this.el.nativeElement.querySelector('#chinMark');

    const that = this;
    interact('.landmark').draggable({
      // enable inertial throwing
      inertia: false,
      // keep the element within the area of it's parent
      restrict: {
        restriction: 'parent',
        endOnly: true,
        elementRect: { top: 0, left: 0, bottom: 1, right: 1 }
      },
      // call this function on every dragmove event
      onmove: function(event) {
        const target = event.target;
        // keep the dragged position in the x/y attributes
        const x = (parseFloat(target.getAttribute('x')) || 0) + event.dx;
        const y = (parseFloat(target.getAttribute('y')) || 0) + event.dy;
        // translate the element
        that.translateElement(target, new Point(x, y));
      },
      // call this function on every dragend event
      onend: function(event) {
        that.updateLandMarks();
      }
    });
  }

  zoomFit(): void {
    const xratio = this._viewPortWidth / this._imageWidth;
    const yratio = this._viewPortHeight / this._imageHeight;
    this._ratio = xratio < yratio ? xratio : yratio;
    this._xleft = this._viewPortWidth / 2 - (this._ratio * this._imageWidth) / 2;
    this._ytop = this._viewPortHeight / 2 - (this._ratio * this._imageHeight) / 2;
  }

  calculateViewPort(): void {
    if (!this._containerElmt) {
      return;
    }
    this._viewPortWidth = this._containerElmt.clientWidth;
    this._viewPortHeight = this._containerElmt.clientHeight;
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
    this.translateElement(this._imgElmt, new Point(this._xleft, this._ytop));
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
      $('.landmark').css('visibility', 'visible');
    } else {
      $('.landmark').css('visibility', 'hidden');
    }
  }

  pixelToScreen(elmt: any, pt: Point): Point {
    return new Point(
      this._xleft + pt.x * this._ratio - elmt.clientWidth / 2,
      this._ytop + pt.y * this._ratio - elmt.clientHeight / 2
    );
  }

  screenToPixel(elmt: any): Point {
    return new Point(
      (parseFloat(elmt.getAttribute('x')) + elmt.clientWidth / 2 - this._xleft) / this._ratio,
      (parseFloat(elmt.getAttribute('y')) + elmt.clientHeight / 2 - this._ytop) / this._ratio
    );
  }

  updateLandMarks() {
    this.crownPoint = this.screenToPixel(this._crownMarkElmt);
    this.chinPoint = this.screenToPixel(this._chinMarkElmt);
    this.edited.emit({
      crownPoint: this.crownPoint,
      chinPoint: this.chinPoint
    });
  }
}
