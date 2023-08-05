import { Component, OnInit } from '@angular/core';
import { PhotoStandard } from '../model/datatypes';
import { PhotoStandardService } from '../services/photo-standard.service';

@Component({
  selector: 'app-photo-standard-selector',
  template: `
    <ion-card routerLink="/standards" class="margin-vertical">
      <ion-card-content>
        <ion-item lines="none">
          <ion-img class="ion-no-padding" [src]="photoStandard.thumbnail"></ion-img>
          <ion-label class="ion-margin-start"> {{ photoStandard.text }} </ion-label>
        </ion-item>
      </ion-card-content>
    </ion-card>
  `,
  styles: [
    `
      ion-img {
        width: 3em;
        height: 3em;
      }
    `,
  ],
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
