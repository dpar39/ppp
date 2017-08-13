import { Component, OnInit, ElementRef, ViewChild } from '@angular/core';
import * as $ from 'jquery'

import { Input, Output } from '@angular/core'

import * as interact from 'interactjs';

import { Point } from '../model/datatypes'

@Component({
  selector: 'app-landmark-editor',
  templateUrl: './landmark-editor.component.html',
  styleUrls: ['./landmark-editor.component.css']
})
export class LandmarkEditorComponent implements OnInit {

  private _inputPhoto: string = "data:image/gif;base64,R0lGODlhAQABAAAAACwAAAAAAQABAAA=";

  public crownPoint: Point;
  public chinPoint: Point;

  private m_imageWidth: number;
  private m_imageHeight: number;
  private m_viewPortWidth: number;
  private m_viewPortHeight: number;

  private m_xleft: number; // Offset in screen pixels
  private m_ytop: number;
  private m_zoom: number;
  private m_ratio: number;   // Ratio between image pixels and screen pixels

  landMarksVisible: boolean = false;

  m_imgElmt: any;
  m_containerElmt: any;
  m_crownMarkElmt: any;
  m_chinMarkElmt: any;

  @Input()
  set inputPhoto(value: string) {

    var newImg = new Image();
    let that = this;
    newImg.onload = function () {
      that.m_imageWidth = newImg.width;
      that.m_imageHeight = newImg.height;
      that._inputPhoto = value;
      that.calculateViewPort();
      that.zoomFit();
      that.renderImage();
      that.setLandMarks();
    };
    newImg.src = value;
  }
  get inputPhoto(): string {
    return this._inputPhoto;
  }

  constructor(private el: ElementRef) {
  }

  ngOnInit() {
    this.m_imgElmt = this.el.nativeElement.querySelector('#photo');
    this.m_containerElmt = this.el.nativeElement.querySelector('#container');
    this.m_crownMarkElmt = this.el.nativeElement.querySelector('#crownMark');
    this.m_chinMarkElmt = this.el.nativeElement.querySelector('#chinMark');
  }
  zoomFit(): void {
    let xratio = this.m_viewPortWidth / this.m_imageWidth;
    let yratio = this.m_viewPortHeight / this.m_imageHeight;
    this.m_ratio = xratio < yratio ? xratio : yratio;
    this.m_xleft = this.m_viewPortWidth / 2 - this.m_ratio * this.m_imageWidth / 2;
    this.m_ytop = this.m_viewPortHeight / 2 - this.m_ratio * this.m_imageHeight / 2;
  };
  calculateViewPort(): void {
    this.m_viewPortWidth = this.m_containerElmt.clientWidth;
    this.m_viewPortHeight = this.m_containerElmt.clientHeight;
  };

  renderImage(): void {

    let xw = this.m_imageWidth * this.m_ratio;
    let yh = this.m_imageHeight * this.m_ratio;

    this.m_imgElmt.width = xw;
    this.m_imgElmt.height = yh;
    this.translateElement(this.m_imgElmt, new Point(this.m_xleft, this.m_ytop));
    console.log(this)
  }

  translateElement(elmt: any, pt: Point) {
    // Translate the element position
    elmt.transform =
      elmt.webkitTransform =
      'translate(' + pt.x + 'px, ' + pt.y + 'px)';
    // Store it in attached properties
    elmt.setAttribute('x', pt.x);
    elmt.setAttribute('y', pt.y);
  };

  setLandMarks(): void {
    // Testing data
    //m_crownPoint = crownPoint || { "x": 1.136017e+003, "y": 6.216124e+002 };
    //m_chinPoint = chinPoint || { "x": 1.136017e+003, "y": 1.701095e+003 };
    //this.m_crownPoint = crownPoint || this.m_crownPoint;
    //this.m_chinPoint = chinPoint || this.m_chinPoint;

    if (this.crownPoint && this.crownPoint.x && this.crownPoint.y
      && this.chinPoint && this.chinPoint.x && this.chinPoint.y) {
      let p1 = this.pixelToScreen(this.m_crownMarkElmt, this.crownPoint);
      let p2 = this.pixelToScreen(this.m_chinMarkElmt, this.chinPoint);
      this.translateElement(this.m_crownMarkElmt, p1);
      this.translateElement(this.m_chinMarkElmt, p2);
      $(".landmark").css('visibility', 'visible');
    } else {
      $(".landmark").css('visibility', 'hidden');
    }
  };

  pixelToScreen(elmt: any, pt: Point): Point {
    return new Point(
      this.m_xleft + pt.x * this.m_ratio - elmt.clientWidth / 2,
      this.m_ytop + pt.y * this.m_ratio - elmt.clientHeight / 2);
  };

  screenToPixel(elmt: any): Point {
    return new Point(
      (parseFloat(elmt.getAttribute('x')) + elmt.clientWidth / 2 - this.m_xleft) / this.m_ratio,
      (parseFloat(elmt.getAttribute('y')) + elmt.clientHeight / 2 - this.m_ytop) / this.m_ratio);
  }
}
