
import { Component, AfterViewInit} from '@angular/core';
import { NavController } from 'ionic-angular';
import { InputPhotoComponent } from '../../components/input-photo/input-photo';
import { Camera, CameraOptions } from '@ionic-native/camera'
import { Http } from '@angular/http';

declare var cpp: any;

@Component({
  selector: 'page-home',
  templateUrl: 'home.html',
  viewProviders: [InputPhotoComponent]
})
export class HomePage implements AfterViewInit {

  constructor(public navCtrl: NavController, camera: Camera, private http: Http) {
    this._camera = camera;
  }

  private _camera: Camera;
  private srcImg: string;
  public cppProp: string = ".....";

  private _cppReady = false;

  ngAfterViewInit() {
    this.cppProp = "----";

  }

  initCppEngine() {
    if (cpp != null && !this._cppReady) {
      this.http.get('/assets/config.json').subscribe(data => {
        this.cppProp = cpp.NativeWrapper.configure(data.json(), (ret) => {
           this.cppProp = ret;
           this._cppReady = true;
        }, (err)=> {
           this.cppProp = err;
        });
      });
    }
  }


  takePicture()  {

    const options: CameraOptions = {
      quality: 100,
      destinationType: this._camera.DestinationType.DATA_URL,
      encodingType: this._camera.EncodingType.PNG,
      mediaType: this._camera.MediaType.PICTURE
    };

    this._camera.getPicture(options).then((imageData) => {
      this.initCppEngine();
      let base64Image = 'data:image/png;base64,' + imageData;
      this.srcImg = base64Image;
      this.cppProp = cpp.NativeWrapper.setImage(imageData, (imgKey) => {
        this.cppProp = imgKey;
      });
    }, (err) => {

    });
    this.cppProp = "Aha!!!!"
  }
}
