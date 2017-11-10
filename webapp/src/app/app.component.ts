import { Component, ElementRef } from '@angular/core';

import { LandmarkEditorComponent } from './landmark-editor/landmark-editor.component'

import { HttpRequest, HttpEventType, HttpParams } from '@angular/common/http';
import { Http, Response, Headers, RequestOptions, URLSearchParams } from '@angular/http';

import { Point, LandMarks, CrownChinPointPair } from './model/datatypes'

@Component({
  selector: 'app-root',
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.css'],
})
export class AppComponent {

  title = 'app';
  imageKey: string;
  imageSrc: string = "#";
  crownChinPointPair: CrownChinPointPair;

  constructor(private el: ElementRef, private http: Http) {
  }

  loadImage(event) {
    let fileList: FileList = event.target.files;
    if (fileList && fileList[0]) {
      let file = fileList[0];

      this.crownChinPointPair = null;

      // Upload the file to the server to detect landmarks
      this.uploadImageToServer(file);
      // Read the image and display it
      let reader = new FileReader();
      reader.onload = () => {
        let imgdata = reader.result;
        this.imageSrc = imgdata;
      }
      reader.readAsDataURL(file);
    }
  }

  uploadImageToServer(file: File): Promise<any> {
    return new Promise((resolve, reject) => {
      let formData = new FormData();
      formData.append('uploads[]', file, file.name);

      let that = this;
      let xhr: XMLHttpRequest = new XMLHttpRequest();

      xhr.onreadystatechange = () => {
        if (xhr.readyState == 4 && xhr.status == 200) {
          // We have a sucessful response from the server
          console.log("Image successfully uploaded to the server");

          var response = JSON.parse(xhr.responseText);

          if (response.imgKey) {
            that.imageKey = response.imgKey;
            that.retrieveLandmarks();
          }
        }
      };

      xhr.upload.onprogress = (evt) => {
        if (evt.lengthComputable) {
          // calculate the percentage of upload completed
          let percentComplete = evt.loaded / evt.total;
          percentComplete = percentComplete * 100;

          // update the Bootstrap progress bar with the new percentage
          // $('.progress-bar').text(percentComplete + '%');
          // $('.progress-bar').width(percentComplete + '%');

          // // once the upload reaches 100%, set the progress bar text to done
          // if (percentComplete === 100) {
          //     $('.progress-bar').html('Done');
          // }
        }
      };

      xhr.open('POST', '/api/upload', true);
      xhr.send(formData);
    });
  }

  retrieveLandmarks() {
    let params: URLSearchParams = new URLSearchParams();
    params.set('imgKey', this.imageKey);

    this.http.get('/api/landmarks', { search: params }).subscribe(data => {
      let landmarks : LandMarks = data.json();
      if (landmarks.errorMsg) {
        console.log(landmarks.errorMsg);
      } else {

        if (landmarks.crownPoint && landmarks.chinPoint) {
          console.log('Landmarks calculated.');
          this.crownChinPointPair = landmarks;
        }
      }
    }, err => {
      console.log(err);
    });
  }

  onLandmarksEdited(data) {

  }
}
