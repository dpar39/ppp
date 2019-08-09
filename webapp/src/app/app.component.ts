import {Component, ElementRef, OnInit} from '@angular/core';

import {Canvas, CrownChinPointPair, PhotoStandard, TiledPhotoRequest, UnitType} from './model/datatypes';
import {BackEndService, ImageLoadResult} from './services/back-end.service';

@Component({
    selector: 'app-root',
    template: `
        <div class="container-fluid">
            <div class="row">
                <h2 class="col text-center">A photo ID creation tool</h2>
            </div>
            <div class="progress" style="height: 3px;">
                <div
                    class="progress-bar bg-success"
                    role="progressbar"
                    [style.width]="appDataLoadingProgress"
                    aria-valuemin="0"
                    aria-valuemax="100"
                ></div>
            </div>
        </div>

        <div class="container-fluid">
            <div class="row">
                <div class="col-sm-12 col-md-6">
                    <div class="container-fluid">
                        <div class="row">
                            <div class="col">
                                <app-landmark-editor
                                    style="margin: 0 auto;"
                                    [inputPhoto]="imageLoadResult"
                                    [crownChinPointPair]="crownChinPointPair"
                                    (edited)="onLandmarksEdited($event)"
                                >
                                </app-landmark-editor>

                            </div>
                        </div>
                        <div class="row">
                            <div class="col">
                                <div class="text-center">
                                    <button
                                        [disabled]="!appReady"
                                        type="button"
                                        class="btn btn-primary"
                                        style="margin-right: 1em"
                                        (click)="el.nativeElement.querySelector('#selectImage').click()"
                                    >
                                        Choose photo
                                    </button>
                                    <button
                                        type="button"
                                        [disabled]="!photoUploaded"
                                        class="btn btn-primary btn-primary-spacing"
                                        (click)="createPrint()"
                                    >
                                        Create Print
                                    </button>
                                    <form>
                                        <input
                                            id="selectImage"
                                            type="file"
                                            name="uploads[]"
                                            accept="image/*"
                                            style="visibility: hidden;"
                                            (change)="loadImage($event)"
                                        />
                                    </form>
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
                    (printDefinitionSelected)="onPrintDefinitionSelected($event)">
                    </app-print-definition-selector>
                </div>
            </div>
            <div class="row">
                <a *ngIf="outImgSrc != '#'" [href]="outImgSrc" download="print.png" class="col-lg-8 col-sm-12">
                    <img [src]="outImgSrc" *ngIf="outImgSrc != '#'" class="fit" />
                </a>
            </div>
        </div>
    `,
    styles: [
        `
            .fit {
                max-width: 99%;
                max-height: 99%;
            }
        `
    ]
})
export class AppComponent implements OnInit {
    echoString = 'Welcome to this app';

    appReady = false;
    appDataLoadingProgress = '0%';
    photoUploaded = false;

    imageLoadResult: ImageLoadResult;
    outImgSrc: any = '#';

    // Model data
    crownChinPointPair: CrownChinPointPair;
    photoStandard: PhotoStandard;
    canvas: Canvas;

    constructor(public el: ElementRef, private beService: BackEndService) {
        beService.runtimeInitialized.subscribe((success: boolean) => {
            this.appReady = success;
            this.appDataLoadingProgress = '100%';
        });

        beService.appLoadingProgressReported.subscribe((progressPct: number) => {
            this.appDataLoadingProgress = '' + progressPct + '%';
        });
    }

    ngOnInit(): void {
        //  this.echoString = '' + this.beService._isMobilePlatform;
    }

    loadImage(event) {
        const fileList: FileList = event.target.files;
        if (fileList && fileList[0]) {
            const file = fileList[0];
            this.crownChinPointPair = null;
            this.imageLoadResult = null;
            // Upload the file to the server to detect landmarks
            this.beService.loadImageInMemory(file).then(result => {
                this.imageLoadResult = result;
                this.retrieveLandmarks();
            });

        }
    }

    retrieveLandmarks() {
        this.beService.retrieveLandmarks(this.imageLoadResult.imgKey).then(landmarks => {
            if (landmarks.errorMsg) {
                console.log(landmarks.errorMsg);
            } else {
                if (landmarks.crownPoint && landmarks.chinPoint) {
                    console.log('Landmarks calculated.');
                    this.crownChinPointPair = landmarks;
                }
            }
        });
    }

    onPhotoStandardSelected(photo: PhotoStandard) {
        this.photoStandard = photo;
    }

    onPrintDefinitionSelected(canvas: Canvas) {
        this.canvas = canvas;
    }


    onLandmarksEdited(crownChinPointPair: CrownChinPointPair) {
        this.crownChinPointPair = crownChinPointPair;
    }

    createPrint() {
        console.log('Creating print output');
        const req = new TiledPhotoRequest(
            this.imageLoadResult.imgKey,
            this.photoStandard.dimensions,
            this.canvas,
            this.crownChinPointPair
        );
        console.log(req);

        this.beService.getTiledPrint(req).then(outputDataUrl => {
            this.outImgSrc = outputDataUrl;
        });
    }
}
