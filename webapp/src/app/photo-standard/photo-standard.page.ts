import { Component, OnInit } from '@angular/core';
import { PhotoStandardService } from '../services/photo-standard.service';
import { PhotoStandard } from '../model/datatypes';
import { Router } from '@angular/router';
import { Location } from '@angular/common';

@Component({
  selector: 'app-photo-standard',
  template: `
    <ion-header>
      <ion-toolbar>
        <ion-buttons slot="start">
          <ion-back-button defaultHref="/"></ion-back-button>
        </ion-buttons>
        <ion-buttons slot="end">
          <ion-menu-button></ion-menu-button>
        </ion-buttons>
        <ion-title>Available Photo Standards</ion-title>
      </ion-toolbar>
    </ion-header>

    <ion-content>
      <ion-list>
        <ion-list-header class="ion-no-padding">
          <ion-searchbar
            placeholder="Search photo standards"
            (ionInput)="filterPhotoStandard($event)"
          ></ion-searchbar>
        </ion-list-header>
        <ion-item *ngFor="let ps of selectableStandards" (click)="setSelected(ps)">
          <span [class]="getFlagClass(ps)"></span>
          <ion-label class="ion-margin-start"> {{ ps.text }} </ion-label>
          <ion-button icon-only><ion-icon name="create"></ion-icon></ion-button>
        </ion-item>
      </ion-list>
    </ion-content>
  `,
  styles: [``]
})
export class PhotoStandardPage implements OnInit {
  public photoStandard: PhotoStandard = new PhotoStandard('__unknown__', 'Loading ...');

  public _allStandards: PhotoStandard[];

  public _selectableStandards: PhotoStandard[];

  constructor(private location: Location, private psService: PhotoStandardService) {
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
    const selectableStandards =
      this._selectableStandards != null ? this._selectableStandards : this._allStandards;

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
    this.psService.setSelectedStandard(ps);
    this.location.back();
  }

  getFlagClass(ps: PhotoStandard): string {
    return 'flag-icon flag-icon-' + this.getCountryCode(ps);
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
    this._selectableStandards = this._allStandards.filter(
      ps => ps.country.toLowerCase().includes(text) || ps.text.toLowerCase().includes(text)
    );
  }
}
