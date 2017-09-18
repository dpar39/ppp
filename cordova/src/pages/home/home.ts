
import { Component, AfterViewInit} from '@angular/core';
import { NavController } from 'ionic-angular';
import { Camera, CameraOptions } from '@ionic-native/camera'
import { Http } from '@angular/http';

import { InputPhotoComponent } from '../../components/input-photo/input-photo';
import { LandmarkEditorComponent } from '../../components/landmark-editor/landmark-editor';

import {CrownChinPointPair } from '../../model/interfaces'

declare var cpp: any;

@Component({
  selector: 'page-home',
  templateUrl: 'home.html',
  viewProviders: [InputPhotoComponent, LandmarkEditorComponent]
})
export class HomePage implements AfterViewInit {


  imageKey: string;
  imageSrc: string = "#";
  crownChinPointPair: CrownChinPointPair;

  private _camera: Camera;
  private _cppReady = false;

  constructor(public navCtrl: NavController, camera: Camera, private http: Http) {
    this._camera = camera;
  }

  ngAfterViewInit() {
    this.imageKey = "----";

  }

  initCppEngine() {
    if (cpp != null && !this._cppReady) {
      this.http.get('/assets/config.json').subscribe(data => {
        this.imageKey = cpp.NativeWrapper.configure(data.json(), (ret) => {
           this.imageKey = ret;
           this._cppReady = true;
        }, (err)=> {
           this.imageKey = err;
        });
      });
    }
  }

  takePicture()  {
    const options: CameraOptions = {
      quality: 99,
      destinationType: this._camera.DestinationType.DATA_URL,
      encodingType: this._camera.EncodingType.JPEG,
      mediaType: this._camera.MediaType.PICTURE
    };

    this._camera.getPicture(options).then((imageData) => {
      this.initCppEngine();
      let base64Image = 'data:image/png;base64,' + imageData;
      this.imageSrc = base64Image;
      this.imageKey = cpp.NativeWrapper.setImage(imageData, (imgKey) => {
        this.imageKey = imgKey;
      });
    }, (err) => {

    });
    this.imageKey = "Loading image..."
  }
}
