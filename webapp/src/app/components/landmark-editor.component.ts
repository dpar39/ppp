import {Component, OnInit, ElementRef, ViewChild, HostListener} from '@angular/core';
import {Input, Output, EventEmitter} from '@angular/core';

import interact from 'interactjs';
import {Point, CrownChinPointPair} from '../model/datatypes';
import {ImageLoadResult} from '../services/back-end.service';

@Component({
    selector: 'app-landmark-editor',
    template: `
        <div id="viewport">
            <img id="photo" alt="Input Image" title="Input picture" [src]="getImageDataUrl()" (load)="imageLoaded()" />
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

            #photo {
                position: absolute;
            }

            #viewport {
                max-height: 50vh;
                min-height: 40vh;
                border: 1px solid #363434;
                border-radius: 5px;
                margin: 5px auto;
                background: #333;
            }
        `
    ]
})
export class LandmarkEditorComponent implements OnInit {
    private _imageWidth = 0;
    private _imageHeight = 0;
    private _viewPortWidth = 0;
    private _viewPortHeight = 0;

    private _xLeft = 0; // Offset in screen pixels
    private _yTop = 0;
    private _zoom = 1;
    private _ratio = 0; // Ratio between image pixels and screen pixels

    private _imgElmt: any = null;
    private _containerElmt: any = null;
    private _crownMarkElmt: any = null;
    private _chinMarkElmt: any = null;

    chinPoint: Point;
    crownPoint: Point;

    landmarkVisibility = 'hidden';

    private _imageLoadResult: ImageLoadResult;

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

    @Input()
    set inputPhoto(value: ImageLoadResult) {
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
        this._containerElmt = this.el.nativeElement.querySelector('#viewport');
        this._crownMarkElmt = this.el.nativeElement.querySelector('#crownMark');
        this._chinMarkElmt = this.el.nativeElement.querySelector('#chinMark');

        const that = this;
        interact('.landmark').draggable({
            // enable inertial throwing
            inertia: false,
            // keep the element within the area of it's parent
            modifiers: [
                interact.modifiers.restrictRect({
                    restriction: 'parent',
                    endOnly: true,
                    elementRect: {top: 0, left: 0, bottom: 1, right: 1}
                })
            ],

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

    screenToPixel(elmt: any): Point {
        return new Point(
            Math.round((parseFloat(elmt.getAttribute('x')) + elmt.clientWidth / 2 - this._xLeft) / this._ratio),
            Math.round((parseFloat(elmt.getAttribute('y')) + elmt.clientHeight / 2 - this._yTop) / this._ratio)
        );
    }

    updateLandMarks() {
        this.crownPoint = this.screenToPixel(this._crownMarkElmt);
        this.chinPoint = this.screenToPixel(this._chinMarkElmt);
        this.edited.emit(new CrownChinPointPair(this.crownPoint, this.chinPoint));
    }
}
