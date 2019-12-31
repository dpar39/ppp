import { Component, OnInit } from '@angular/core';
import { PhotoStandard } from '../model/datatypes';
import { PhotoStandardService } from '../services/photo-standard.service';

@Component({
  selector: 'app-photo-standard-selector',
  template: `
    <ion-card routerLink="/standards">
      <ion-card-header>
        Selected Photo Standard
      </ion-card-header>
      <ion-card-content>
        <span [class]="getFlagClass(photoStandard)"></span>
        <ion-label class="ion-margin-start"> {{ photoStandard.text }} </ion-label>
      </ion-card-content>
    </ion-card>
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
export class PhotoStandardSelectorComponent implements OnInit {
  photoStandard: PhotoStandard = new PhotoStandard('__unknown__', 'Loading ...');

  constructor(private psService: PhotoStandardService) {
    psService.photoStandardSelected.subscribe((ps: PhotoStandard) => {
      if (ps !== this.photoStandard) {
        this.photoStandard = ps;
      }
    });
  }

  ngOnInit() {
    this.photoStandard = this.psService.getSelectedStandard();
  }

  getFlagClass(ps: PhotoStandard): string {
    return 'flag-icon flag-icon-' + ps.id.substr(0, 2);
  }
}
