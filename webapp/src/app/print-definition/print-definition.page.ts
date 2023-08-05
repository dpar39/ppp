import { Component } from '@angular/core';
import { DomSanitizer, SafeHtml } from '@angular/platform-browser';
import { Router } from '@angular/router';
import { ModalController } from '@ionic/angular';

import { PrintDefinition, UnitType } from '../model/datatypes';
import { PhotoStandardService } from '../services/photo-standard.service';
import { PrintDefinitionService } from '../services/print-definition.service';
import { renderAsPng } from '../model/svg-builder';
import { SettingsService } from '../services/settings.service';
import { PaperSizeEditComponent } from './paper-size-edit.component';
import { Subscription } from 'rxjs';
import { BackEndService } from '../services/backend.service';

@Component({
  selector: 'app-print-definition',
  template: `
    <ion-header>
      <ion-toolbar>
        <ion-buttons slot="end">
          <ion-menu-button></ion-menu-button>
        </ion-buttons>
        <ion-title>Create your own prints</ion-title>
      </ion-toolbar>
    </ion-header>

    <ion-content>
      <div class="spinner-overlay" [style.visibility]="isBusy ? 'visible' : 'hidden'">
        <ion-spinner name="lines" color="medium"></ion-spinner>
      </div>
      <ion-grid class="c-flex-grid">
        <ion-row style="flex-grow: 1">
          <ion-col>
            <ion-content>
              <ion-grid>
                <ion-row>
                  <ion-col
                    size-xs="6"
                    size-sm="4"
                    size-lg="4"
                    size-xl="3"
                    [ngClass]="getSelectionClass(pd)"
                    *ngFor="let pd of printDefinitions"
                    (click)="setSelected(pd)"
                  >
                    <div style="position: relative">
                      <div class="thumbnail" [innerHTML]="printsSvg.get(pd)"></div>
                      <!-- <ion-img
                        *ngIf="pd.walgreens"
                        src="/assets/walgreens.png"
                        slot="end"
                        class="walgreens-icon"
                      ></ion-img> -->
                    </div>
                    <div class="ion-text-center subtitle">
                      <ion-label color="secondary" class="ion-margin-start ion-margin-top">
                        <span>{{ pd.title }}</span>
                      </ion-label>
                      <div *ngIf="pd.custom" style="position: absolute; top:2px;right:5px">
                        <ion-button
                          class="ion-no-padding square"
                          size="small"
                          icon-only
                          color="success"
                          (click)="edit(pd)"
                        >
                          <ion-icon slot="icon-only" name="settings"></ion-icon>
                        </ion-button>
                        <ion-button
                          class="ion-no-padding square"
                          size="small"
                          icon-only
                          color="danger"
                          (click)="delete(pd)"
                        >
                          <ion-icon slot="icon-only" name="trash"></ion-icon>
                        </ion-button>
                      </div>
                    </div>
                  </ion-col>
                  <ion-col class="ion-align-self-center">
                    <ion-fab-button color="primary" style="margin: auto" (click)="createNew()">
                      <ion-icon name="add-outline"></ion-icon>
                    </ion-fab-button>
                    <div class="ion-text-center subtitle">
                      <ion-label class="ion-text-wrap" color="medium">Add custom print size</ion-label>
                    </div>
                  </ion-col>
                </ion-row>
              </ion-grid>
            </ion-content>
          </ion-col>
        </ion-row>
        <ion-row>
          <ion-col>
            <ion-button expand="block" color="dark" (click)="goBack()">
              <ion-icon name="crop-outline" class="ion-padding-start" slot="end"></ion-icon>
              <span>Crop</span>
              <ion-icon name="chevron-back" class="ion-padding-end" slot="start"></ion-icon>
            </ion-button>
          </ion-col>
          <ion-col>
            <ion-button expand="block" (click)="goNext()">
              <ion-icon name="eye-sharp" class="ion-padding-end" slot="start"></ion-icon>
              <span>Preview</span>
              <ion-icon name="chevron-forward" class="ion-padding-start" slot="end"></ion-icon>
            </ion-button>
          </ion-col>
        </ion-row>
      </ion-grid>
    </ion-content>
  `,
  styles: [
    `
      .subtitle {
        margin-top: 8px;
      }
      .thumbnail {
        height: 40vmin;
        padding: 5px !important;
        background-color: var(--ion-color-light);
      }
      .selected {
        border-width: 2px;
        border-style: solid;
        border-color: var(--ion-color-primary);
        border-radius: 8px;
      }
      .normal {
        border: 2px solid transparent;
        border-radius: 8px;
      }
      .square {
        width: 1.5em !important;
        height: 1.5em !important;
      }

      ion-note {
        font-size: 1.2em;
      }
      ion-select {
        width: 100%;
        justify-content: center;
      }
      .spinner-overlay {
        position: absolute;
        height: 100%;
        width: 100%;
        z-index: 1;
      }
      ion-spinner {
        transform: scale(5);
        position: absolute;
        left: 50%;
        top: 50%;
      }
    `,
  ],
})
export class PrintDefinitionPage {
  private _xmlSerializer = new XMLSerializer();

  printsSvg = new Map<PrintDefinition, SafeHtml>();
  printDefinitions: PrintDefinition[];
  digitalSvg: SafeHtml;

  isBusy = false;

  croppedImageChangedSub: Subscription;
  printerSettingsChangedSub: Subscription;

  constructor(
    private sanitizer: DomSanitizer,
    private pdService: PrintDefinitionService,
    private psService: PhotoStandardService,
    private settingsService: SettingsService,
    private beService: BackEndService,
    private router: Router,
    private modalController: ModalController
  ) {}

  ngOnInit() {
    this.croppedImageChangedSub = this.psService.croppedImageChanged.subscribe(() => this.computeViews());
    this.printerSettingsChangedSub = this.settingsService.printSettingsChanged.subscribe(() => this.computeViews());
    this.beService.landmarksUpdated.subscribe((x) => console.log('Updating'));
    this.computeViews();
  }

  ngOnDestroy() {
    this.croppedImageChangedSub.unsubscribe();
    this.printerSettingsChangedSub.unsubscribe();
  }

  computeViews() {
    this.printDefinitions = this.pdService.getAllPrintDefinitions();
    this.printsSvg.clear();
    for (const pd of this.printDefinitions) {
      let svg = this.getPrintSvg(pd);
      if (!(pd.width > 0 && pd.height > 0 && pd.units)) {
        this.digitalSvg = svg;
      }
      this.printsSvg.set(pd, svg);
    }
  }

  getPrintSvg(pd: PrintDefinition) {
    const img = this.psService.getCroppedImageBlobUrl();
    if (!img) return null;
    const ps = this.psService.getSelectedStandard();
    const svg = this.pdService.renderPrintSvg(ps.dimensions, img, pd);
    svg.setAttribute('width', '100%');
    svg.setAttribute('height', '100%');
    svg.setAttribute('preserveAspectRatio', 'xMidYMid meet');
    const xml = this._xmlSerializer.serializeToString(svg);
    return this.sanitizer.bypassSecurityTrustHtml(xml);
  }

  setSelected(pd: PrintDefinition) {
    this.pdService.setSelectedPrintDefinition(pd);
    //this._location.back();
  }

  getSelected() {
    return this.pdService.getSelectedPrintDefinition();
  }

  getSelectionClass(pd) {
    return pd === this.pdService.getSelectedPrintDefinition() ? 'selected' : 'normal';
  }

  goBack() {
    this.router.navigate(['/crop']);
  }

  getStringRepr(pd: PrintDefinition): string {
    if (pd.title) {
      return pd.title;
    }
    const units = pd.units === 'inch' ? '″' : pd.units;
    return `${pd.height} x ${pd.width}${units} [${pd.resolution}dpi]`;
  }

  prepareOutput(callback: (file: File) => void) {
    const ps = this.psService.getSelectedStandard();
    const pd = this.pdService.getSelectedPrintDefinition();

    const blobUrl = this.psService.getCroppedImageBlobUrl();
    const dataUrl = this.psService.getCroppedImageDataUrl();
    const dpi = this.psService.getSelectedStandard().dimensions.dpi;

    const cb = (blob: Blob) => {
      const filename =
        pd.height > 0 && pd.width > 0 && pd.units
          ? `${pd.height}x${pd.width}${pd.units}-print.png`
          : pd.title.toLocaleLowerCase().replace(' ', '-') + '.png';
      let file: any = blob;
      const ff = new File([file], filename, { type: 'image/png' });
      callback(ff);
    };

    const svg = this.pdService.renderPrintSvg(ps.dimensions, dataUrl);
    renderAsPng(svg as SVGSVGElement, dpi, (blob) => cb(blob), blobUrl, dataUrl);
  }

  async createNew() {
    const pd = this.pdService.getSelectedPrintDefinition();
    this.edit(pd);
  }

  async edit(pd: PrintDefinition) {
    const modal = await this.modalController.create({
      component: PaperSizeEditComponent,
      componentProps: pd,
    });
    await modal.present();

    const { data } = await modal.onDidDismiss();
    if (data) {
      const pd = data as PrintDefinition;
      this.pdService.update(pd);
      this.computeViews();
    }
  }

  goNext() {
    this.isBusy = true;
    this.prepareOutput(async (pr: File) => {
      this.beService.setPrintOutput(pr);
      this.isBusy = false;
      await this.router.navigate(['/preview']);
    });
  }

  delete(pd: PrintDefinition) {
    this.pdService.delete(pd);
    this.computeViews();
  }
}
