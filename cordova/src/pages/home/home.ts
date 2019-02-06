
import { Component, AfterViewInit } from '@angular/core';
import { NavController } from 'ionic-angular';
import { Camera, CameraOptions } from '@ionic-native/camera'
import { Http } from '@angular/http';
import { ResponseContentType } from '@angular/http';

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
    this.initCppEngine();
  }

  initCppEngine() : void {
    if (!this._cppReady) {
      this.http.get('assets/config.json').subscribe(r => {
        let objcfg = r.json();
        this.http.get('assets/sp_model.dat', {
          responseType: ResponseContentType.Blob
        }).subscribe(rr => {
          let reader = new FileReader();
          reader.readAsDataURL(rr.blob());
          reader.onloadend = () => {
            let content64 = reader.result as string;
            content64 = content64.substring(content64.indexOf(',') + 1);
            objcfg.shapePredictor.data = content64;
            let config = JSON.stringify(objcfg);
            objcfg = null; // memory cleanup
            cpp.NativeWrapper.configure(config, (ret) => {
              config = null; // memory cleanup
              this._cppReady = true;
              this.imageKey = 'Configured!';
            });
          };
        });
      });
    }
  }

  loadPicture(): void {
    this.takePicture(false);
  }

  takePicture(useCamera: boolean = true) {
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
      cpp.NativeWrapper.setImage(imageData, (imgKey) => {
        this.imageKey = imgKey;
        this.detectLandmarks();
      });
    }, (err) => {
      console.log(err);
    });
    this.imageKey = "Loading image..."
  }

  detectLandmarks() {
    cpp.NativeWrapper.detectLandMarks(this.imageKey, (lmstr) => {
       let landmarks : CrownChinPointPair = JSON.parse(lmstr);
       this.crownChinPointPair = landmarks;
      // this.imageKey = lmstr;;
    });
  }
}
