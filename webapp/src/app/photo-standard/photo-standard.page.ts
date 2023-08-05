import { Component, OnInit } from '@angular/core';
import { PhotoStandardService } from '../services/photo-standard.service';
import { PhotoStandard } from '../model/datatypes';
import { Location } from '@angular/common';
import { Router } from '@angular/router';

@Component({
  selector: 'app-photo-standard',
  template: `
    <ion-header>
      <ion-toolbar>
        <ion-buttons slot="end">
          <ion-menu-button></ion-menu-button>
        </ion-buttons>
        <ion-title>Select Photo ID Standard</ion-title>
      </ion-toolbar>
    </ion-header>

    <ion-content>
      <ion-grid class="c-flex-grid">
        <ion-row>
          <ion-col class="ion-no-padding">
            <ion-searchbar
              placeholder="Search photo standards"
              (ionInput)="filterPhotoStandard($event)"
            ></ion-searchbar>
          </ion-col>
        </ion-row>
        <ion-row style="flex-grow: 1">
          <ion-col>
            <ion-content>
              <ion-virtual-scroll [items]="selectableStandards" approxItemHeight="3.5em">
                <ion-item
                  lines="none"
                  *virtualItem="let ps; let itemBounds = bounds"
                  detail
                  (click)="setSelected(ps)"
                  [class]="getItemClass(ps)"
                >
                  <ion-img [src]="ps.thumbnail"></ion-img>
                  <ion-label class="ion-margin-start"> {{ ps.text }} </ion-label>
                  <!--ion-button fill="outline" icon-only>
                    <ion-icon name="ellipsis-horizontal"></ion-icon>
                  </ion-button-->
                </ion-item>
              </ion-virtual-scroll>
            </ion-content>
          </ion-col>
        </ion-row>
        <ion-row>
          <ion-col class="ion-no-padding justify-content-center ion-padding-top">
            <app-photo-standard-selector></app-photo-standard-selector>
          </ion-col>
        </ion-row>
        <ion-row>
          <ion-col>
            <ion-button expand="block" color="dark" (click)="goBack()">
              <ion-icon name="chevron-back" class="ion-padding-end" slot="start"></ion-icon>
              <span>Start</span>
              <ion-icon name="camera" class="ion-padding-start" slot="end"></ion-icon>
            </ion-button>
          </ion-col>
          <ion-col>
            <ion-button expand="block" (click)="goNext()">
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
      .ps-item {
        --border-style: solid;
        --border-width: 1px;
        --border-radius: 5px;
      }
      .unselected-item {
      }
      .selected-item {
        --border-color: var(--ion-color-primary);
        --background: var(--ion-color-secondary);
      }
      ion-item {
        padding: 1px;
      }
      ion-img {
        width: 2.5em;
        height: 3.5em;
        padding: 3px;
      }
    `,
  ],
})
export class PhotoStandardPage implements OnInit {
  public photoStandard: PhotoStandard = new PhotoStandard('__unknown__', 'Loading ...');

  public _allStandards: PhotoStandard[];

  public _selectableStandards: PhotoStandard[];

  constructor(private psService: PhotoStandardService, private router: Router) {
    this._allStandards = psService.getAllPhotoStandards();
    this.photoStandard = psService.getSelectedStandard();

    // psService.photoStandardSelected.subscribe((ps: PhotoStandard) => {
    //   if (ps !== this.photoStandard) {
    //     this.photoStandard = ps;
    //   }
    // });
  }

  ngOnInit() {
    this._selectableStandards = this._allStandards;
  }

  public get selectableStandards() {
    const selectableStandards = this._selectableStandards != null ? this._selectableStandards : this._allStandards;

    let cc = this.psService.getCountryCode();
    if (!cc) {
      return selectableStandards;
    }
    cc = cc.toLowerCase();
    if (cc) {
      selectableStandards.sort((a: PhotoStandard, b: PhotoStandard) => {
        const ccA = this.getCountryCode(a);
        if (ccA === cc) {
          return -1;
        }
        const ccB = this.getCountryCode(b);
        if (ccB === cc) {
          return +1;
        }
        return 0;
      });
    }
    return selectableStandards;
  }

  public setSelected(ps: PhotoStandard): void {
    this.photoStandard = ps;
    this.psService.setSelectedStandard(ps);
  }

  getCountryCode(ps: PhotoStandard): string {
    return ps.id.substr(0, 2);
  }

  filterPhotoStandard(event) {
    const text = event.target.value.toLowerCase();
    if (!text) {
      this._selectableStandards = this._allStandards;
      return;
    }

    const words = text.split(' ');
    this._selectableStandards = this._allStandards.filter(
      (ps) =>
        words.every((w) => ps.country.toLowerCase().includes(w)) ||
        words.every((w) => ps.text.toLowerCase().includes(w))
    );
    this._selectableStandards.sort((a: PhotoStandard, b: PhotoStandard) => {
      if (a.text.toLowerCase().startsWith(text)) {
        return a.docType === 'Passport' ? -2 : -1;
      }
      if (b.text.toLowerCase().startsWith(text)) {
        return b.docType == 'Passport' ? +2 : +1;
      }
      return 0;
    });
  }

  getItemClass(ps: PhotoStandard) {
    return 'ps-item ' + (ps == this.photoStandard ? 'selected-item' : 'unselected-item');
  }

  goBack() {
    this.router.navigate(['/start']);
  }

  goNext() {
    this.router.navigate(['/crop']);
  }
}
