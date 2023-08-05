import { Component, ElementRef, OnInit } from '@angular/core';
import { AppLoadProgress, BackEndService, ImageLoadResult } from '../services/backend.service';
import { AlertController } from '@ionic/angular';
import { Camera, CameraResultType } from '@capacitor/camera';
import { Router } from '@angular/router';
import { Subscription } from 'rxjs';

@Component({
  selector: 'app-start',
  template: `
    <ion-header>
      <ion-toolbar>
        <ion-buttons slot="end">
          <ion-menu-button></ion-menu-button>
        </ion-buttons>
        <ion-title id="title"> Photo ID Creator </ion-title>
      </ion-toolbar>
    </ion-header>

    <ion-content>
      <ion-grid class="c-flex-grid">
        <ion-row>
          <ion-col class="ion-text-center">
            <ion-label class="ion-text-center" color="medium">{{ appLoadProgress.step }}</ion-label>
          </ion-col>
        </ion-row>
        <ion-row>
          <ion-col>
            <ion-progress-bar
              class="progress-bar"
              color="success"
              [value]="appLoadProgress.progress"
              [buffer]="appLoadProgress.progress"
            >
            </ion-progress-bar>
          </ion-col>
        </ion-row>
        <ion-row><ion-col style="min-height: 200px"></ion-col> </ion-row>
        <ion-row class="c-ion-row" id="dropZone" appDragDrop (onFileDropped)="loadImage($event)">
          <ion-col>
            <ion-grid>
              <ion-row
                ><ion-col class="ion-text-center">
                  <ion-label>Let's start by taking a new photo or loading an existing one from the gallery</ion-label>
                </ion-col></ion-row
              >
              <ion-row>
                <ion-col>
                  <ion-button expand="block" color="primary" (click)="loadFromFile()">
                    <ion-icon name="folder" class="ion-padding-end"></ion-icon>
                    Gallery
                  </ion-button>
                </ion-col>
                <ion-col>
                  <ion-button expand="block" color="primary" (click)="takePicture()">
                    <ion-icon name="camera" class="ion-padding-end"></ion-icon>
                    <span>Camera</span>
                  </ion-button>
                </ion-col>
              </ion-row>
            </ion-grid>
          </ion-col>
          <form>
            <input id="selectImage" type="file" name="uploads[]" accept="image/*" hidden (change)="loadImage($event)" />
          </form>
        </ion-row>
        <ion-row *ngIf="appReady && imageLoaded">
          <ion-col></ion-col>
          <ion-col>
            <ion-button [style.display]="appReady ? 'visible' : 'none'" expand="block" (click)="goNext()">
              <ion-icon name="crop-outline" class="ion-padding-end" slot="start"></ion-icon>
              <span>Crop</span>
              <ion-icon name="chevron-forward" class="ion-padding-start" slot="end"></ion-icon>
            </ion-button>
          </ion-col>
        </ion-row>
      </ion-grid>
    </ion-content>
  `,
  styles: [
    `
      .c-ion-row {
        align-items: center;
        justify-content: center;
        height: 100%;
      }

      #dropZone {
        border-radius: 5px;
        border-color: rgba(255, 0, 0, 0);
      }
    `,
  ],
})
export class StartPage implements OnInit {
  appReady = false;
  imageLoaded = false;

  imageLoadedSub: Subscription;
  runtimeInitializedSub: Subscription;
  appLoadProgressSub: Subscription;

  appLoadProgress: AppLoadProgress = {
    progress: 0,
    step: 'Loading...',
  };
  private _pendingFile?: File;

  constructor(
    private el: ElementRef,
    private beService: BackEndService,
    private router: Router,
    private alertController: AlertController
  ) {}

  ngOnInit() {
    this.appReady = this.beService.getRuntimeInitialized();
    if (this.appReady) {
      this.appLoadProgress.progress = 1.0;
      this.appLoadProgress.step = 'Application Ready';
    }
    this.runtimeInitializedSub = this.beService.runtimeInitialized.subscribe((success: boolean) => {
      this.appReady = success;
      this.appLoadProgress.progress = 1.0;
      this.processInputImage();
    });

    this.appLoadProgressSub = this.beService.appLoadingProgressReported.subscribe((a: AppLoadProgress) => {
      this.appLoadProgress = a;
    });

    this.imageLoaded = this.beService.getCacheImageLoadResult() != null;
    this.imageLoadedSub = this.beService.imageLoaded.subscribe(
      (ilr: ImageLoadResult) => (this.imageLoaded = ilr && !!ilr.imgKey)
    );
  }

  ngOnDestroy() {
    this.runtimeInitializedSub.unsubscribe();
    this.imageLoadedSub.unsubscribe();
    this.appLoadProgressSub.unsubscribe();
  }

  processInputImage() {
    if (!this.appReady || !this._pendingFile) {
      return; // Nothing to do yet
    }

    this.beService.loadImageInMemory(this._pendingFile);
    this._pendingFile = null;
    this.router.navigate(['/crop']);
  }

  takePicture() {
    // Otherwise, make the call:
    Camera.getPhoto({
      quality: 100,
      allowEditing: true,
      resultType: CameraResultType.Uri,
    })
      .then((image) => {
        const xhr = new XMLHttpRequest();
        xhr.responseType = 'blob';
        xhr.onload = () => {
          const blob = xhr.response;
          blob.lastModifiedDate = new Date();
          blob.name = 'camera-picture';
          this._pendingFile = blob;
          this.processInputImage();
        };
        xhr.open('GET', image.webPath as string);
        xhr.send();
      })
      .catch((err) => {
        if (typeof err == 'string' && err.includes('cancelled')) {
          return;
        }
        this.alertController
          .create({
            header: 'No Camera Available',
            message: 'Make sure your webcam is properly connected.',
            buttons: ['OK'],
          })
          .then((alert) => alert.present());
        console.error(err);
      });
  }

  loadFromFile() {
    this.el.nativeElement.querySelector('#selectImage').click();
  }

  loadImage(event: Event) {
    const target = event.target as HTMLInputElement;
    const fileList = target.files;
    if (fileList && fileList[0]) {
      this._pendingFile = fileList[0];
      this.processInputImage();
    }
  }

  goNext() {
    this.router.navigate(['/crop']);
  }
}
