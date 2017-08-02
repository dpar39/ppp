import { Component } from '@angular/core';
import { NavController } from 'ionic-angular';
import { AfterViewInit} from '@angular/core';
import { InputPhotoComponent } from '../../components/input-photo/input-photo';
import { Camera, CameraOptions } from '@ionic-native/camera'

declare var cpp: any;

@Component({
  selector: 'page-home',
  templateUrl: 'home.html',
  viewProviders: [InputPhotoComponent]
})
export class HomePage implements AfterViewInit {
  
  constructor(public navCtrl: NavController, camera: Camera) {
    this._camera = camera;
  }
  
  private _camera: Camera;
  private srcImg: string;
  cppProp: string = ".....";

  

  ngAfterViewInit() {
    this.cppProp = "xxxxxxx";
    try{
      if (cpp) {
        cpp.NativeWrapper.detectLandMarks("imgKey", ret => {
          this.cppProp = ret;
        });
      }
    }
    catch (Exception) {
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

      let base64Image = 'data:image/png;base64,' + imageData;
      this.srcImg = base64Image;
    }, (err) => {

    });
    this.cppProp = "Aha!!!!"
  }
}
