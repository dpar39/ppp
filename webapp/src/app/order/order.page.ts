import { Component } from '@angular/core';
import { DomSanitizer, SafeHtml, SafeResourceUrl } from '@angular/platform-browser';
import { Router } from '@angular/router';

import { PrintDefinition } from '../model/datatypes';
import { PhotoStandardService } from '../services/photo-standard.service';
import { PrintDefinitionService } from '../services/print-definition.service';

@Component({
  selector: 'app-print-definition',
  template: `
    <ion-header>
      <ion-toolbar>
        <ion-buttons slot="end">
          <ion-menu-button></ion-menu-button>
        </ion-buttons>
        <ion-title>Order copies by mail</ion-title>
      </ion-toolbar>
    </ion-header>

    <ion-content>
      <ion-grid class="c-flex-grid">
        <ion-row>
          <ion-col class="ion-text-center"> </ion-col>
        </ion-row>
        <ion-row>
          <ion-col>
            <ion-card>
              <ion-card-content>
                <ion-grid class="ion-no-padding">
                  <ion-row>
                    <ion-col id="col1">
                      <div *ngIf="croppedImage" style="overflow: hidden;">
                        <img class="digital overlay" [src]="croppedImage" />
                        <img class="digital" [src]="croppedImage" />
                      </div>
                    </ion-col>
                  </ion-row>
                  <ion-row>
                    <ion-col class="ion-align-self-end ion-text-end ion-no-padding">
                      <ion-item lines="none">
                        <ion-select (value)="(selectedNumCopies)" interface="popover">
                          <ion-select-option *ngFor="let c of copiesList" value="{{ c.copies }}"
                            >{{ c.copies }} Copies - $<span>{{ c.price }}</span></ion-select-option
                          >
                        </ion-select>
                      </ion-item>

                      <ion-button>
                        <ion-icon slot="end" name="cart-outline"></ion-icon>
                        <span>Add to Cart</span>
                      </ion-button>
                    </ion-col>
                  </ion-row>
                </ion-grid>
              </ion-card-content>
            </ion-card>
          </ion-col>
        </ion-row>
        <ion-row style="flex-grow: 1"></ion-row>
        <ion-row>
          <ion-col>
            <ion-button expand="block" color="dark" (click)="goBack()">
              <ion-icon name="crop-outline" class="ion-padding-start" slot="end"></ion-icon>
              <span>Edit</span>
              <ion-icon name="chevron-back" class="ion-padding-end" slot="start"></ion-icon>
            </ion-button>
          </ion-col>
          <ion-col>
            <ion-button expand="block" (click)="goBack()">
              <span>Place Order</span>
              <ion-icon name="chevron-forward" class="ion-padding-start" slot="end"></ion-icon>
            </ion-button>
          </ion-col>
        </ion-row>
      </ion-grid>
    </ion-content>
  `,
  styles: [
    `
      .digital {
        height: 80vmin;
        background-color: transparent !important;
        overflow: hidden;
        border: 2px solid gray;
        border-radius: 5px;
      }

      .overlay {
        position: absolute;
        top: 0.6em;
        left: 0.6em;
      }

      ion-select {
        text-align: right;
        width: 100%;
        justify-content: center;
      }
    `,
  ],
})
export class OrderPage {
  private _xmlSerializer = new XMLSerializer();

  printsSvg = new Map<PrintDefinition, SafeHtml>();
  printDefinitions: PrintDefinition[];
  digitalSvg: SafeHtml;

  copiesList = [
    { copies: '2', price: '2.99' },
    { copies: '3', price: '3.99' },
  ];

  selectedNumCopies: string = '2';
  croppedImage: SafeResourceUrl = null;
  constructor(
    private psService: PhotoStandardService,
    private sanitizer: DomSanitizer,
    private pdService: PrintDefinitionService,
    private router: Router
  ) {}

  ngOnInit() {
    this.psService.croppedImageChanged.subscribe((src) => {
      this.croppedImage = src ? this.sanitizer.bypassSecurityTrustResourceUrl(src) : null;
    });
    const src = this.psService.getCroppedImageBlobUrl();
    this.croppedImage = src ? this.sanitizer.bypassSecurityTrustResourceUrl(src) : null;
  }

  ngOnDestroy() {}

  getPrintSvg(pd: PrintDefinition) {
    const img = this.psService.getCroppedImageBlobUrl();
    if (!img) return null;
    const ps = this.psService.getSelectedStandard();
    const svg = this.pdService.renderPrintSvg(ps.dimensions, img, pd);
    svg.setAttribute('width', '100%');
    svg.setAttribute('height', '100%');
    svg.setAttribute('preserveAspectRatio', 'xMidYMax meet');
    const xml = this._xmlSerializer.serializeToString(svg);
    return this.sanitizer.bypassSecurityTrustHtml(xml);
  }

  goBack() {
    this.router.navigate(['/crop']);
  }
}
