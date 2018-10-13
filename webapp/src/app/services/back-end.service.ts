import { Injectable } from '@angular/core';
import { DomSanitizer } from '@angular/platform-browser';

import { TiledPhotoRequest } from '../model/datatypes';

@Injectable() export class BackEndService {

    constructor(private sanitizer: DomSanitizer) {
    }

    uploadImageToServer(file: File): Promise<string> {
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

    getTiledPrint(req: TiledPhotoRequest): Promise<any> {
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
                    const b = window.URL.createObjectURL(new Blob(binaryData, { type: 'image/png' }));

                    const outImgSrc = this.sanitizer.bypassSecurityTrustResourceUrl(b);
                    resolve(outImgSrc);
                }
            };
            const data = JSON.stringify(req);
            xhr.send(data);
        });
    }
}
