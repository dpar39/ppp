import {Component, OnInit, ElementRef} from '@angular/core';
import {CrownChinPointPair, PrintDefinition, PhotoStandard, TiledPhotoRequest} from '../model/datatypes';
import {BackEndService, ImageLoadResult} from '../services/backend.service';
import {PhotoStandardService} from '../services/photo-standard.service';
import {PrintDefinitionService} from '../services/print-definition.service';

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
                    <ion-col size-xs="12" size-sm="12" size-lg="6" size-xl="4">
                        <!-- Load image and edit landmarks -->
                        <ion-grid class="ion-no-padding">
                            <ion-row>
                                <ion-col>
                                    <app-landmark-editor
                                        style="margin: 0 auto;"
                                        [inputPhoto]="imageLoadResult"
                                        [crownChinPointPair]="crownChinPointPair"
                                        (edited)="onLandmarksEdited($event)"
                                        [photoDimensions]="photoStandard?.dimensions"
                                    >
                                    </app-landmark-editor>
                                </ion-col>
                            </ion-row>
                            <ion-row>
                                <ion-col>
                                    <ion-button
                                        expand="block"
                                        class="ion-no-padding"
                                        color="primary"
                                        (click)="el.nativeElement.querySelector('#selectImage').click()"
                                    >
                                        Choose photo
                                    </ion-button>
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
                    </ion-col>
                    <ion-col size-xs="12" size-sm="12" size-lg="6" size-xl="4" class="ion-no-padding">
                        <app-photo-standard-selector> </app-photo-standard-selector>
                        <app-print-definition-selector> </app-print-definition-selector>
                    </ion-col>
                    <ion-col size-xs="12" size-sm="12" size-lg="6" size-xl="4">
                        <a
                            *ngIf="outImgSrc != '#'"
                            [href]="outImgSrc"
                            download="print.png"
                            class="text-center col-lg-8 col-sm-12"
                        >
                            <img [src]="outImgSrc" *ngIf="outImgSrc != '#'" class="print-results" />
                        </a>
                    </ion-col>
                </ion-row>
            </ion-grid>
        </ion-content>
    `,
    styles: [
        `
            .print-results {
                max-width: 99%;
                max-height: 99%;
            }
        `
    ]
})
export class HomePage implements OnInit {
    appReady = false;
    appDataLoadingProgress = 0.02;
    pendingFile: File;

    imageLoadResult: ImageLoadResult;
    outImgSrc: string;

    // Model data
    crownChinPointPair: CrownChinPointPair;
    photoStandard: PhotoStandard;
    printDefinition: PrintDefinition;

    constructor(
        public el: ElementRef,
        public beService: BackEndService,
        psService: PhotoStandardService,
        pdService: PrintDefinitionService
    ) {
        this.appReady = beService.getRuntimeInitialized();
        if (this.appReady) {
            this.appDataLoadingProgress = 1.0;
        }
        beService.runtimeInitialized.subscribe((success: boolean) => {
            this.appReady = success;
            this.appDataLoadingProgress = 1.0;
            this.processInputImage();
        });

        beService.appLoadingProgressReported.subscribe((progressPct: number) => {
            this.appDataLoadingProgress = progressPct / 100.0;
        });

        this.photoStandard = psService.getSelectedStandard();
        psService.photoStandardSelected.subscribe((ps: PhotoStandard) => {
            this.onPhotoStandardSelected(ps);
        });

        this.printDefinition = pdService.getSelectedPrintDefinition();
        pdService.printDefinitionSelected.subscribe(pd => {
            this.onPrintDefinitionSelected(pd);
        });

        this.imageLoadResult = beService.getCacheImageLoadResult();
        this.crownChinPointPair = beService.getCacheLandmarks();
        this.outImgSrc = beService.getCachePrintResult();
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
        console.debug(`Retrieving landmarks for image ${this.imageLoadResult.imgKey}`);
        this.beService.retrieveLandmarks(this.imageLoadResult.imgKey).then(landmarks => {
            if (landmarks.errorMsg) {
                console.error(landmarks.errorMsg);
            } else {
                if (landmarks.crownPoint && landmarks.chinPoint) {
                    console.debug('Landmarks calculated.');
                    this.crownChinPointPair = landmarks;
                    this.createPrint();
                }
            }
        });
    }

    onPhotoStandardSelected(ps: PhotoStandard) {
        this.photoStandard = ps;
        this.createPrint();
    }

    onPrintDefinitionSelected(canvas: PrintDefinition) {
        this.printDefinition = canvas;
        this.createPrint();
    }

    onLandmarksEdited(crownChinPointPair: CrownChinPointPair) {
        this.crownChinPointPair = crownChinPointPair;
        this.createPrint();
    }

    createPrint() {
        if (!this.imageLoadResult || !this.printDefinition || !this.crownChinPointPair || !this.photoStandard) {
            return;
        }
        const req = new TiledPhotoRequest(
            this.imageLoadResult.imgKey,
            this.photoStandard.dimensions,
            this.printDefinition,
            this.crownChinPointPair
        );

        console.debug(`Creating print output. Request = ${req}`);

        this.beService.getTiledPrint(req).then(outputDataUrl => {
            if (this.outImgSrc) {
                URL.revokeObjectURL(this.outImgSrc);
            }
            this.outImgSrc = outputDataUrl;
        });
    }
}
