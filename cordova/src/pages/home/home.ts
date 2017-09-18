
import { Component, AfterViewInit} from '@angular/core';
import { NavController } from 'ionic-angular';
import { Camera, CameraOptions } from '@ionic-native/camera'
import { Http } from '@angular/http';

import { LandmarkEditorComponent } from '../../components/landmark-editor/landmark-editor';

import { CrownChinPointPair } from '../../model/interfaces'

declare var cpp: any;

@Component({
  selector: 'page-home',
  templateUrl: 'home.html',
  viewProviders: [LandmarkEditorComponent]
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
    this.imageKey = "000000";
    this.initCppEngine();
  }

  initCppEngine() {
    if (cpp != null && !this._cppReady) {
      this.http.get('/assets/config.json').subscribe(data => {
        let config = JSON.stringify(data.json());
        //this.imageKey = config;
        this.imageKey = cpp.NativeWrapper.configure(config, (ret) => {
           this.imageKey = ret;
           this._cppReady = true;
        }, (err)=> {
           this.imageKey = err;
        });
      });
    }
  }

  loadPicture() : void {
    this.takePicture(false);
  }

  takePicture(useCamera : boolean = true)  {
    const options: CameraOptions = {
      quality: 100,
      destinationType: this._camera.DestinationType.DATA_URL,
      encodingType: this._camera.EncodingType.JPEG,
      mediaType: this._camera.MediaType.PICTURE,
      sourceType: useCamera ? this._camera.PictureSourceType.CAMERA : this._camera.PictureSourceType.SAVEDPHOTOALBUM
    };

    this._camera.getPicture(options).then((imageData) => {
      this.initCppEngine();
      let base64Image = 'data:image/jpg;base64,' + imageData;
      this.imageSrc = base64Image;
      this.imageKey = cpp.NativeWrapper.setImage(imageData, (imgKey) => {
        this.imageKey = imgKey;
        this.detectLandmarks();
      });
    }, (err) => {
      console.log(err);
    });
    this.imageKey = "Loading image..."
  }

  setImageInCpp()  {

  }

  detectLandmarks() {
    cpp.NativeWrapper.detectLandMarks(this.imageKey, (data) => {
      this.imageKey = data;
      // let landmarks : LandMarks = data.json();
      // this.crownChinPointPair = landmarks;
    });
  }
}
