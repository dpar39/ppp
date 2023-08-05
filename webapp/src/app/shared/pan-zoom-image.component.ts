import { Component, ElementRef, HostListener, Input, ViewChild } from '@angular/core';
import interact from 'interactjs';

@Component({
  selector: 'app-pan-zoom-image',
  template: `
    <div #viewport class="viewport" (wheel)="onMouseWheel($event)">
      <img #image class="out-image" [src]="imageUrl" (load)="makeGesturable(true)" />
    </div>
  `,
  styles: [
    `
      .viewport {
        position: relative;
        width: 100%;
        height: 100%;
        touch-action: none;
        overflow: hidden;
      }
      .out-image {
        display: inline-block;
        position: absolute;
        height: auto;
        border: 1px solid gray;
      }
    `,
  ],
})
export class PanZoomImageComponent {
  xLeft = 0;
  yTop = 0;
  zoom = 1;
  ratio = 0;

  viewportWidth = 0;
  viewportHeight = 0;
  imageWidth = 0;
  imageHeight = 0;

  constructor() {}

  @Input('imageUrl')
  imageUrl: any;

  @ViewChild('image')
  image: ElementRef;

  @ViewChild('viewport')
  viewport: ElementRef;

  initialized = false;

  ngAfterViewInit() {
    setTimeout(() => this.makeGesturable(false), 50);
  }

  @HostListener('contextmenu', ['$event'])
  onRightClick(event: Event) {
    event.preventDefault();
  }

  @HostListener('window:resize', ['$event'])
  onResize(_event: Event) {
    this.makeGesturable(false);
  }

  ngOnDestroy() {
    let viewportElmt = this.viewport.nativeElement as HTMLDivElement;
    interact(viewportElmt).unset();
  }

  makeGesturable(loaded: boolean) {
    let zoomStart: number;

    if (loaded) {
      this.ngOnDestroy();
      this.initialized = false;
    }

    let imageElmt = this.image.nativeElement as HTMLImageElement;
    let viewportElmt = this.viewport.nativeElement as HTMLDivElement;

    this.imageWidth = imageElmt.naturalWidth;
    this.imageHeight = imageElmt.naturalHeight;
    this.viewportWidth = viewportElmt.clientWidth;
    this.viewportHeight = viewportElmt.clientHeight;

    if (
      this.initialized ||
      !(this.viewportWidth > 0) ||
      !(this.viewportHeight > 0) ||
      !(this.imageWidth > 0) ||
      !(this.imageHeight > 0)
    ) {
      return;
    }

    this.initialized = true;

    interact(viewportElmt)
      .draggable({
        inertia: true,
        onmove: (event) => {
          this.xLeft += event.dx;
          this.yTop += event.dy;
          this.renderImage();
        },
      })
      .on('doubletap', (e) => {
        this.zoomFit();
      })
      .gesturable({
        onstart: (e) => {
          zoomStart = this.zoom;
        },
        onmove: (e) => {
          const newZoom = zoomStart * e.scale;
          this.setZoom(newZoom, e.clientX, e.clientY);
        },
      });

    this.zoomFit();
  }

  setZoom(newZoom: number, cx: number, cy: number) {
    if (newZoom <= 0.5) {
      newZoom = 0.5;
    }
    if (newZoom > 32) {
      newZoom = 32;
    }
    const [x, y] = this.screenToPixel(cx, cy);
    const effectiveScale = this.ratio * (this.zoom - newZoom);
    this.yTop += y * effectiveScale;
    this.xLeft += x * effectiveScale;
    this.zoom = newZoom;
    this.renderImage();
  }

  computeMinScale(width: number, height: number) {
    const xRatio = this.viewportWidth / width;
    const yRatio = this.viewportHeight / height;
    return xRatio < yRatio ? xRatio : yRatio;
  }

  screenToPixel(x: number, y: number) {
    const xPrime = (x - this.xLeft) / this.ratio / this.zoom;
    const yPrime = (y - this.yTop) / this.ratio / this.zoom;
    return [xPrime, yPrime];
  }

  zoomFit() {
    this.zoom = 1;
    this.ratio = this.computeMinScale(this.imageWidth, this.imageHeight);
    this.xLeft = this.viewportWidth / 2 - (this.ratio * this.imageWidth) / 2;
    this.yTop = this.viewportHeight / 2 - (this.ratio * this.imageHeight) / 2;
    this.renderImage();
  }

  renderImage() {
    let img = this.image.nativeElement as HTMLImageElement;
    img.style.top = '' + this.yTop + 'px';
    img.style.left = '' + this.xLeft + 'px';
    const dx = Math.round(this.imageWidth * this.ratio * this.zoom);
    img.style['min-width'] = img.style.width = '' + dx + 'px';
  }

  onMouseWheel(e: WheelEvent) {
    this.makeGesturable(false);
    e.preventDefault();
    if (this.imageHeight < 100 || this.imageHeight < 100) {
      return;
    }
    let viewportElmt = this.viewport.nativeElement as HTMLDivElement;
    const clientRect = viewportElmt.getBoundingClientRect();
    const xClient = e.clientX - clientRect.left;
    const yClient = e.clientY - clientRect.top;
    const scale = e.deltaY < 0 ? 1.1 : 1 / 1.1;
    const newZoom = this.zoom * scale;
    this.setZoom(newZoom, xClient, yClient);
  }
}
