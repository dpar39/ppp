import { Injectable } from '@angular/core';
import { Http } from '@angular/http'

@Injectable()
export class BackEndService {

  constructor(private http: Http) {
  }

  uploadImageToServer(file: File): Promise<string> {
    
    return new Promise((resolve, reject) => {
      const formData = new FormData();
      
      formData.append(file.name, file);

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
}
