import { Component, Input } from '@angular/core';

/**
 * Generated class for the InputPhotoComponent component.
 *
 * See https://angular.io/docs/ts/latest/api/core/index/ComponentMetadata-class.html
 * for more info on Angular Components.
 */
@Component({
  selector: 'input-photo',
  templateUrl: 'input-photo.html'
})
export class InputPhotoComponent {

  text: string;
  _photo: string = null;

  constructor() {
    console.log('Hello InputPhotoComponent Component');
    this.text = 'Input photo component';
  }

  @Input()
  set photo(value: string) {
    this._photo  = value;
  }

  get photo() : string
  {
    return this._photo;
  }
}
