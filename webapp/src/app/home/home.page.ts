import {Component, OnInit, ElementRef} from '@angular/core';
import {CrownChinPointPair, Canvas, PhotoStandard, TiledPhotoRequest} from '../model/datatypes';
import {BackEndService, ImageLoadResult} from '../services/backend.service';

@Component({
    selector: 'app-home',
    template: `
        <ion-header>
            <ion-toolbar>
                <ion-buttons slot="end">
                    <ion-menu-button></ion-menu-button>
                </ion-buttons>
                <ion-title>
                    Home
                </ion-title>
            </ion-toolbar>
        </ion-header>

        <ion-content>
            <div class="container-fluid">
                <ion-progress-bar color="success" [value]="appDataLoadingProgress"> </ion-progress-bar>
            </div>

            <ion-grid>
                <ion-row>
                    <ion-col
                        ><app-landmark-editor
                            style="margin: 0 auto;"
                            [inputPhoto]="imageLoadResult"
                            [crownChinPointPair]="crownChinPointPair"
                            (edited)="onLandmarksEdited($event)"
                            [photoDimensions]="photoStandard?.dimensions"
                        >
                        </app-landmark-editor
                    ></ion-col>
                    <ion-col>
                        <form>
                            <input
                                id="selectImage"
                                type="file"
                                name="uploads[]"
                                accept="image/*"
                                style="display: none;"
                                (change)="loadImage($event)"
                            />
                        </form>
                    </ion-col>
                </ion-row>
            </ion-grid>
            <div class="container-fluid">
                <div class="row">
                    <div class="col-sm-12 col-md-6">
                        <div class="container-fluid px-0">
                            <div class="row">
                                <div class="col"></div>
                            </div>
                            <div class="row">
                                <div class="col">
                                    <div class="text-center">
                                        <ion-button
                                            class="ion-padding"
                                            color="primary"
                                            (click)="el.nativeElement.querySelector('#selectImage').click()"
                                        >
                                            Choose photo
                                        </ion-button>
                                    </div>
                                </div>
                            </div>
                        </div>
                    </div>
                    <div class="col-md-6 col-lg-4 col-sm-12">
                        <app-photo-standard-selector
                            class="col-sm"
                            (photoStandardSelected)="onPhotoStandardSelected($event)"
                        >
                        </app-photo-standard-selector>
                        <app-print-definition-selector
                            class="col-sm"
                            (printDefinitionSelected)="onPrintDefinitionSelected($event)"
                        >
                        </app-print-definition-selector>
                    </div>
                </div>
                <div class="row mt-2">
                    <a
                        *ngIf="outImgSrc != '#'"
                        [href]="outImgSrc"
                        download="print.png"
                        class="text-center col-lg-8 col-sm-12"
                    >
                        <img [src]="outImgSrc" *ngIf="outImgSrc != '#'" class="fit" />
                    </a>
                </div>
            </div>
            <!--ion-card class="welcome-card">
        <img src="/assets/shapes.svg" alt="" />
        <ion-card-header>
          <ion-card-subtitle>Get Started</ion-card-subtitle>
          <ion-card-title>Welcome to Ionic</ion-card-title>
        </ion-card-header>
        <ion-card-content>
          <p>
            Now that your app has been created, you'll want to start building
            out features and components. Check out some of the resources below
            for next steps.
          </p>
        </ion-card-content>
      </ion-card>
      <ion-list lines="none">
        <ion-list-header>
          <ion-label>Resources</ion-label>
        </ion-list-header>
        <ion-item href="https://ionicframework.com/docs/">
          <ion-icon slot="start" color="medium" name="book"></ion-icon>
          <ion-label>Ionic Documentation</ion-label>
        </ion-item>
        <ion-item href="https://ionicframework.com/docs/building/scaffolding">
          <ion-icon slot="start" color="medium" name="build"></ion-icon>
          <ion-label>Scaffold Out Your App</ion-label>
        </ion-item>
        <ion-item href="https://ionicframework.com/docs/layout/structure">
          <ion-icon slot="start" color="medium" name="grid"></ion-icon>
          <ion-label>Change Your App Layout</ion-label>
        </ion-item>
        <ion-item href="https://ionicframework.com/docs/theming/basics">
          <ion-icon slot="start" color="medium" name="color-fill"></ion-icon>
          <ion-label>Theme Your App</ion-label>
        </ion-item>
      </ion-list-->
        </ion-content>
    `,
    styles: [
        `
            .welcome-card img {
                max-height: 35vh;
                overflow: hidden;
            }

            .fit {
                max-width: 99%;
                max-height: 99%;
            }

            .app-version {
                color: darkgray;
                font-family: monospace;
                font-size: 7pt;
            }
        `
    ]
})
export class HomePage implements OnInit {
    appReady = false;
    appDataLoadingProgress = 0.02;

    pendingFile: File;

    imageLoadResult: ImageLoadResult;
    outImgSrc = '#';

    // Model data
    crownChinPointPair: CrownChinPointPair;
    photoStandard: PhotoStandard;
    canvas: Canvas;

    constructor(public el: ElementRef, public beService: BackEndService) {
        beService.runtimeInitialized.subscribe((success: boolean) => {
            this.appReady = success;
            this.appDataLoadingProgress = 1.0;
            this.processInputImage();
        });

        beService.appLoadingProgressReported.subscribe((progressPct: number) => {
            this.appDataLoadingProgress = progressPct / 100.0;
        });
    }

    ngOnInit(): void {
        //  this.echoString = '' + this.beService._isMobilePlatform;
    }

    processInputImage() {
        if (!this.appReady || !this.pendingFile) {
            return; // Nothing to do yet
        }
        // Load the image file to detect landmarks
        this.beService.loadImageInMemory(this.pendingFile).then(result => {
            this.pendingFile = null;
            this.imageLoadResult = result;
            this.retrieveLandmarks();
        });
    }

    loadImage(event) {
        const fileList: FileList = event.target.files;
        if (fileList && fileList[0]) {
            this.pendingFile = fileList[0];
            this.crownChinPointPair = null;
            this.imageLoadResult = null;
            this.processInputImage();
        }
    }

    retrieveLandmarks() {
        console.log(this.imageLoadResult.imgKey);
        this.beService.retrieveLandmarks(this.imageLoadResult.imgKey).then(landmarks => {
            if (landmarks.errorMsg) {
                console.log(landmarks.errorMsg);
            } else {
                if (landmarks.crownPoint && landmarks.chinPoint) {
                    console.log('Landmarks calculated.');
                    this.crownChinPointPair = landmarks;
                    this.createPrint();
                }
            }
        });
    }

    onPhotoStandardSelected(photo: PhotoStandard) {
        this.photoStandard = photo;
        this.createPrint();
    }

    onPrintDefinitionSelected(canvas: Canvas) {
        this.canvas = canvas;
        this.createPrint();
    }

    onLandmarksEdited(crownChinPointPair: CrownChinPointPair) {
        this.crownChinPointPair = crownChinPointPair;
        this.createPrint();
    }

    createPrint() {
        if (!this.imageLoadResult || !this.canvas || !this.crownChinPointPair || !this.photoStandard) {
            return;
        }
        console.log('Creating print output');
        const req = new TiledPhotoRequest(
            this.imageLoadResult.imgKey,
            this.photoStandard.dimensions,
            this.canvas,
            this.crownChinPointPair
        );
        console.log(req);

        this.beService.getTiledPrint(req).then(outputDataUrl => {
            if (this.outImgSrc) {
                URL.revokeObjectURL(this.outImgSrc);
            }
            this.outImgSrc = outputDataUrl;
        });
    }
}
