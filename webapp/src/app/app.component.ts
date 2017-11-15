import { Component, ElementRef } from '@angular/core';

import { LandmarkEditorComponent } from './landmark-editor/landmark-editor.component'

import { HttpRequest, HttpEventType, HttpParams } from '@angular/common/http';
import { Http, Response, Headers, RequestOptions, URLSearchParams } from '@angular/http';

import { Point, LandMarks, CrownChinPointPair } from './model/datatypes';

import {BackEndService } from './services/back-end.service';

@Component({
  selector: 'app-root',
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.css'],
})
export class AppComponent {

  title = 'app';
  imageKey: string;
  imageSrc = '#';
  crownChinPointPair: CrownChinPointPair;

  constructor(private el: ElementRef, private http: Http, private beService: BackEndService) {
  }

  loadImage(event) {
    const fileList: FileList = event.target.files;
    if (fileList && fileList[0]) {
      const file = fileList[0];
      this.crownChinPointPair = null;
      // Upload the file to the server to detect landmarks
      this.beService.uploadImageToServer(file).then(imgKey => {
        this.imageKey = imgKey;
        this.retrieveLandmarks();
      });
      // Read the image and display it
      const reader = new FileReader();
      reader.onload = () => {
        const imgdata = reader.result;
        this.imageSrc = imgdata;
      };
      reader.readAsDataURL(file);
    }
  }

  retrieveLandmarks() {
    const params: URLSearchParams = new URLSearchParams();
    params.set('imgKey', this.imageKey);

    this.http.get('/api/landmarks', { search: params }).subscribe(data => {
      const landmarks: LandMarks = data.json();
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
