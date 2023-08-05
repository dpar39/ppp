import { Component, ElementRef, ViewChild } from '@angular/core';
import { BackEndService } from '../services/backend.service';
import { Router } from '@angular/router';
import { PrepareSelectionComponent } from './prepare-selection.component';
import { LandmarkEditorComponent } from './landmark-editor.component';

@Component({
  selector: 'app-crop',
  template: `
    <ion-header>
      <ion-toolbar>
        <ion-buttons slot="end">
          <ion-menu-button></ion-menu-button>
        </ion-buttons>
        <ion-title id="title">Crop picture</ion-title>
      </ion-toolbar>
    </ion-header>

    <ion-content>
      <ion-grid class="c-flex-grid">
        <ion-row>
          <ion-col>
            <ion-label>Adjust crown and chin marks if needed</ion-label>
          </ion-col>
        </ion-row>
        <ion-row style="flex-grow: 1">
          <ion-col>
            <!-- Landmarks editor-->

            <div style="position:relative; height:100%; z-index:1">
              <ion-fab vertical="bottom" class="ion-no-padding" horizontal="end" slot="fixed" show="true">
                <ion-fab-button size="small" color="medium">
                  <ion-icon name="ellipsis-vertical-outline"></ion-icon>
                </ion-fab-button>
                <ion-fab-list side="start">
                  <ion-fab-button><ion-icon name="sunny"></ion-icon></ion-fab-button>
                  <ion-fab-button><ion-icon name="contrast"></ion-icon></ion-fab-button>
                </ion-fab-list>
              </ion-fab>

              <app-landmark-editor #lmEditor> </app-landmark-editor>
            </div>

            <!-- fab placed to the bottom start -->
          </ion-col>
        </ion-row>

        <ion-row>
          <ion-col class="ion-no-padding"
            ><!-- Photo standard selection -->
            <app-photo-standard-selector (click)="goBack()"> </app-photo-standard-selector>
          </ion-col>
        </ion-row>
        <ion-row>
          <ion-col>
            <ion-button expand="block" color="dark" (click)="goToStartPage()">
              <ion-icon name="chevron-back" class="ion-padding-end" slot="start"></ion-icon>
              <span>Start</span>
              <ion-icon name="camera-outline" class="ion-padding-start" slot="end"></ion-icon>
            </ion-button>
          </ion-col>
          <ion-col>
            <ion-button expand="block" (click)="goNext()">
              <ion-icon name="pencil-outline" class="ion-padding-end"></ion-icon>
              <span>Edit</span>
              <ion-icon name="chevron-forward" class="ion-padding-start" slot="end"></ion-icon>
            </ion-button>
          </ion-col>
        </ion-row>
      </ion-grid>
    </ion-content>
  `,
  styles: [],
})
export class CropPage {
  constructor(public el: ElementRef, public beService: BackEndService, private router: Router) {}

  @ViewChild('lmEditor') lmEditor: LandmarkEditorComponent;

  goBack() {
    this.router.navigate(['/standards']);
  }

  ionViewDidEnter() {
    this.lmEditor.onInputPhotoReady();
  }

  goToStartPage() {
    this.router.navigate(['/start']);
  }

  async goNext() {
    this.router.navigate(['/edit']);
  }
}
