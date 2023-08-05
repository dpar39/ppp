import { Component } from '@angular/core';
import { DomSanitizer, SafeHtml } from '@angular/platform-browser';
import { Router } from '@angular/router';
import { ModalController, NavParams } from '@ionic/angular';
import { BackEndService } from '../services/backend.service';

@Component({
  selector: 'app-print-preview',
  template: `
    <ion-header>
      <ion-toolbar>
        <ion-buttons slot="end">
          <ion-menu-button></ion-menu-button>
        </ion-buttons>
        <ion-title>Output preview</ion-title>
      </ion-toolbar>
    </ion-header>
    <ion-content>
      <ion-grid class="c-flex-grid">
        <ion-row style="flex-grow: 1">
          <ion-col>
            <app-pan-zoom-image [imageUrl]="imageUrl"></app-pan-zoom-image>
          </ion-col>
        </ion-row>
        <ion-row>
          <ion-col>
            <ion-button color="primary" expand="block" (click)="save()">
              <ion-icon name="save" class="ion-padding-end"></ion-icon>
              <span> Save</span>
            </ion-button>
          </ion-col>
          <ion-col *ngIf="canShare()">
            <ion-button color="primary" expand="block" (click)="share()">
              <ion-icon name="share-social" class="ion-padding-end"></ion-icon>
              <span> Share</span>
            </ion-button>
          </ion-col>
          <ion-col *ngIf="false">
            <ion-button color="primary" expand="block" (click)="print()">
              <ion-icon name="print" class="ion-padding-end"></ion-icon>
              <span> Print</span>
            </ion-button>
          </ion-col>
        </ion-row>
        <ion-row>
          <ion-col></ion-col>
        </ion-row>
        <ion-row>
          <ion-col>
            <ion-button expand="block" color="dark" (click)="goBack()">
              <ion-icon name="images-outline" class="ion-padding-start" slot="end"></ion-icon>
              <span>Prints</span>
              <ion-icon name="chevron-back" class="ion-padding-end" slot="start"></ion-icon>
            </ion-button>
          </ion-col>
          <ion-col>
            <ion-button expand="block" (click)="goHome()">
              <span>Done</span>
              <ion-icon name="home" class="ion-padding-start" slot="end"></ion-icon>
            </ion-button>
          </ion-col>
        </ion-row>
      </ion-grid>
    </ion-content>
  `,
  styles: [],
})
export class PreviewPrintComponent {
  imageUrl: any;
  outputFile: File;
  constructor(private beService: BackEndService, private router: Router, private sanitizer: DomSanitizer) {}

  ionViewDidEnter() {
    this.outputFile = this.beService.getPrintOutput();
    if (!this.outputFile) return;
    const urlCreator = window.URL || window.webkitURL;
    const unsafeUrl = urlCreator.createObjectURL(this.outputFile);
    this.imageUrl = this.sanitizer.bypassSecurityTrustResourceUrl(unsafeUrl);
  }

  ionViewDidLeave() {
    const urlCreator = window.URL || window.webkitURL;
    urlCreator.revokeObjectURL(this.imageUrl);
  }

  canShare() {
    return !!(window.navigator as any).canShare;
  }

  share() {
    const navigator = window.navigator as any;
    if (this.canShare()) {
      const file = this.outputFile;
      if (navigator.canShare({ files: [file] })) {
        navigator
          .share({
            files: [file],
            title: file.name,
            text: 'Print Output',
          })
          .then(() => console.log('Share was successful.'))
          .catch((error) => console.log('Sharing failed', error));
      } else {
        console.log(`Your system doesn't support sharing files.`);
      }
    }
  }

  save() {
    const file = this.outputFile;
    const a = document.createElement('a');
    const url = URL.createObjectURL(file);
    a.setAttribute('href', url);
    a.setAttribute('download', file.name);
    document.body.appendChild(a);
    a.click();
    URL.revokeObjectURL(url);
    document.body.removeChild(a);
  }

  print() {
    console.error('Not implemented yet!');
  }

  goBack() {
    this.router.navigate(['/prints']);
  }
  goHome() {
    this.router.navigate(['/start']);
  }
}
