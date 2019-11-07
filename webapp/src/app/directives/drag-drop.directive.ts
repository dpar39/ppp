import { Directive, Output, Input, EventEmitter, HostBinding, HostListener } from '@angular/core';

@Directive({
  selector: '[appDragDrop]'
})
export class DragDropDirective {
  @Output() onFileDropped = new EventEmitter<any>();

  //@HostBinding('style.background-color') private background = '#f5fcff'
  //@HostBinding('style.opacity') private opacity = '1'
  // @HostBinding('style.cursor') private cursor = 'pointer';
  @HostBinding('style.border-color') private borderColor = '';
  @HostBinding('style.border-width') private borderWidth = '3px';
  @HostBinding('style.border-style') private borderStyle = 'dashed';

  private _borderNormal: string;
  private _borderDropping: string;

  constructor() {
    const docElmt = getComputedStyle(document.documentElement);
    this._borderNormal = docElmt.getPropertyValue('--ion-background-color');
    this._borderDropping = docElmt.getPropertyValue('--ion-color-primary');
    this.borderColor = this._borderNormal;
  }

  //Dragover listener
  @HostListener('dragover', ['$event']) onDragOver(evt) {
    if (!evt.dataTransfer || !evt.dataTransfer.items || evt.dataTransfer.items[0].kind !== 'file') {
      return;
    }
    evt.preventDefault();
    evt.stopPropagation();
    this.borderColor = this._borderDropping;
    // this.cursor = 'grab';
  }

  //Dragleave listener
  @HostListener('dragleave', ['$event']) public onDragLeave(evt) {
    evt.preventDefault();
    evt.stopPropagation();
    this.borderColor = this._borderNormal;
    // this.cursor = 'pointer';
  }

  //Drop listener
  @HostListener('drop', ['$event']) public ondrop(evt) {
    evt.preventDefault();
    evt.stopPropagation();
    this.borderColor = this._borderNormal;
    // this.cursor = 'pointer';
    let files = evt.dataTransfer.files;
    if (files.length > 0) {
      this.onFileDropped.emit(files);
    }
  }
}
