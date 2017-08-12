import { Component, ElementRef } from '@angular/core';

import { LandmarkEditorComponent } from './landmark-editor/landmark-editor.component'

@Component({
  selector: 'app-root',
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.css']
})
export class AppComponent {
  title = 'app';

  imageSrc: string;
  constructor(private el: ElementRef) {
  }

  loadImage(event) {
    let fileList: FileList = event.target.files;
    if (fileList && fileList[0]) {
      let file = fileList[0];
      // Upload the file to the server to detect landmarks
      this.uploadImageToServer(file);
      // Read the image and display it
      let reader = new FileReader();
      let that = this;
      reader.onload = function () {
        let imgdata = reader.result;
        that.imageSrc = imgdata;
      }
      reader.readAsDataURL(file);
    }
  }
  uploadImageToServer(file: File) {

  }
}
