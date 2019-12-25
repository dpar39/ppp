import { Component, Output, Input } from '@angular/core';
import { PhotoStandard } from '../model/datatypes';
import { PhotoStandardService } from '../services/photo-standard.service';

@Component({
  selector: 'app-photo-standard-selector',
  template: `
    <ion-card *ngIf="collapsed" (click)="collapsed = false">
      <ion-card-header>
        Selected Photo Standard
      </ion-card-header>
      <ion-card-content>
        <span [class]="getFlagClass(photoStandard)"></span>
        <ion-label class="ion-margin-start"> {{ photoStandard.text }} </ion-label>
      </ion-card-content>
    </ion-card>
    <ion-list *ngIf="!collapsed">
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
  `,
  styles: [
    `
      ion-list {
        min-height: 250px;
        max-height: 25vh;
        overflow-y: scroll;
      }
    `
  ]
})
export class PhotoStandardSelectorComponent {
  public photoStandard: PhotoStandard = new PhotoStandard('__unknown__', 'Loading ...');

  public _allStandards: PhotoStandard[];

  public _selectableStandards: PhotoStandard[];

  constructor(private psService: PhotoStandardService) {
    this._allStandards = psService.getAllPhotoStandards();
    this.photoStandard = psService.getSelectedStandard();

    psService.photoStandardSelected.subscribe((ps: PhotoStandard) => {
      if (ps !== this.photoStandard) {
        this.photoStandard = ps;
      }
    });
  }

  private _collapsed = true;
  public get collapsed() {
    return this._collapsed;
  }
  public set collapsed(value: boolean) {
    if (value === this._collapsed) {
      return;
    }
    this._collapsed = value;
    this._selectableStandards = this._allStandards;
    this.sortByImportance();
  }

  public get selectableStandards() {
    const selectableStandards = this._selectableStandards != null ? this._selectableStandards : this._allStandards;

    const cc = this.psService.getCountryCode().toLowerCase();
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
    this.collapsed = true;
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
    this.sortByImportance();
  }

  sortByImportance() {

  }
}
