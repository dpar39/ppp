import {Injectable} from '@angular/core';
import {Http, ResponseContentType} from '@angular/http';
import {DomSanitizer, SafeResourceUrl} from '@angular/platform-browser';
import {Plugins} from '@capacitor/core';

import {TiledPhotoRequest} from '../model/datatypes';

@Injectable()
export class BackEndService {
  _isConfigured = false;

  constructor(private sanitizer: DomSanitizer, private http: Http) {

  }

  configure(): void {
    const {PppPlugin} = Plugins;
    this.http.get('assets/config.json').subscribe(r => {
      let objcfg = r.json();
      this.http
          .get('assets/sp_model.dat', {responseType: ResponseContentType.Blob})
          .subscribe(rr => {
            let reader = new FileReader();
            reader.readAsDataURL(rr.blob());
            reader.onloadend = () => {
              let content64 = reader.result as string;
              content64 = content64.substring(content64.indexOf(',') + 1);
              objcfg.shapePredictor.data = content64;

              PppPlugin.configure(objcfg).then(ret => {
                objcfg = null;  // memory cleanup
                this._isConfigured = true;
              });
            };
          });
    });
  }

  uploadImageToServer(file: File): Promise<string> {

    if (this._isConfigured) {
        const {PppPlugin} = Plugins;

        file
        let reader = new FileReader();
            reader.readAsDataURL(file);
            reader.onloadend = () => {
                let content64 = reader.result as string;
                return PppPlugin.setImage({ imageData: content64});
            }
    }

    return new Promise((resolve, reject) => {
      const formData = new FormData();
      formData.append('uploads[]', file, file.name);

      const xhr: XMLHttpRequest = new XMLHttpRequest();
      xhr.onreadystatechange = () => {
        if (xhr.readyState === 4 && xhr.status === 200) {
          // We have a sucessful response from the server
          console.log('Image successfully uploaded to the server');
          const response = JSON.parse(xhr.responseText);
          const imgKey: string = response.imgKey;
          if (imgKey) {
            resolve(imgKey);
          }
          reject(imgKey);
        }
      };
      xhr.open('POST', '/api/upload', true);
      xhr.send(formData);
    });
  }

  retrieveLandmarks(imageKey: string): Promise<any> {
    return new Promise((resolve, reject) => {
      const xhr: XMLHttpRequest = new XMLHttpRequest();
      const url = '/api/landmarks/' + imageKey;
      xhr.open('GET', url);
      xhr.send();
      xhr.onreadystatechange = (e) => {
        if (xhr.readyState === 4 && xhr.status === 200) {
          try {
            const landmarks = JSON.parse(xhr.responseText);
            resolve(landmarks);
          } catch (e) {
            console.log(e);
            reject();
          }
        }
      };
    });
  }

  getTiledPrint(req: TiledPhotoRequest): Promise<SafeResourceUrl> {
    return new Promise((resolve, reject) => {
      const xhr = new XMLHttpRequest();
      const url = '/api/photoprint';
      xhr.open('POST', url, true);
      xhr.setRequestHeader('Content-Type', 'application/json');
      xhr.responseType = 'blob';
      xhr.onreadystatechange = () => {
        if (xhr.readyState === 4 && xhr.status === 200) {
          const blob = xhr.response;
          const binaryData = [];
          binaryData.push(blob);
          const b = window.URL.createObjectURL(
              new Blob(binaryData, {type: 'image/png'}));

          const outImgSrc = this.sanitizer.bypassSecurityTrustResourceUrl(b);
          resolve(outImgSrc);
        }
      };
      const data = JSON.stringify(req);
      xhr.send(data);
    });
  }
}
