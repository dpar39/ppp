import { Component, ElementRef, OnInit } from '@angular/core';
import { Router } from '@angular/router';
import { BackEndService } from '../services/backend.service';

@Component({
  selector: 'app-edit',
  template: `
    <ion-header>
      <ion-toolbar>
        <ion-buttons slot="end">
          <ion-menu-button></ion-menu-button>
        </ion-buttons>
        <ion-title id="title">Edit Photo</ion-title>
      </ion-toolbar>
    </ion-header>

    <ion-content>
      <ion-grid class="c-flex-grid">
        <ion-row style="flex-grow: 1">
          <ion-col>
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
            </div>

            <!-- fab placed to the bottom start -->
          </ion-col>
        </ion-row>

        <ion-row>
          <ion-col class="ion-no-padding">Work in progress </ion-col>
        </ion-row>
        <ion-row>
          <ion-col>
            <ion-button expand="block" color="dark" (click)="goPrev()">
              <ion-icon name="chevron-back" class="ion-padding-end" slot="start"></ion-icon>
              <span>Crop</span>
              <ion-icon name="crop-outline" class="ion-padding-start" slot="end"></ion-icon>
            </ion-button>
          </ion-col>
          <ion-col>
            <ion-button expand="block" (click)="goNext()">
              <ion-icon name="print-outline" class="ion-padding-end"></ion-icon>
              <span>Prepare</span>
              <ion-icon name="chevron-forward" class="ion-padding-start" slot="end"></ion-icon>
            </ion-button>
          </ion-col>
        </ion-row>
      </ion-grid>
    </ion-content>
  `,
  styles: [],
})
export class EditPage implements OnInit {
  constructor(public el: ElementRef, public beService: BackEndService, private router: Router) {}

  ngOnInit() {}

  goNext() {
    this.router.navigate(['/prints']);
  }

  goPrev() {
    this.router.navigate(['/crop']);
  }
}
