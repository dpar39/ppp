import { Component } from '@angular/core';
import { FormBuilder, FormControl, FormGroup, Validators } from '@angular/forms';
import { DomSanitizer, SafeHtml } from '@angular/platform-browser';
import { ModalController, NavParams } from '@ionic/angular';

import { PrintDefinition, UnitType } from '../model/datatypes';
import { PhotoStandardService } from '../services/photo-standard.service';
import { PrintDefinitionService } from '../services/print-definition.service';

@Component({
  selector: 'app-edit-paper-size',
  template: `
    <ion-header>
      <ion-toolbar>
        <ion-title>Edit Print Layout</ion-title>
        <ion-buttons slot="primary">
          <ion-button (click)="onCancel()">
            <ion-icon name="close"></ion-icon>
          </ion-button>
        </ion-buttons>
      </ion-toolbar>
    </ion-header>
    <ion-content>
      <form [formGroup]="ionicForm" (ngSubmit)="submitForm()" style="height:100%;">
        <ion-grid class="c-flex-grid">
          <ion-row>
            <ion-col>
              <ion-item class="ion-no-padding-start">
                <ion-label>Name</ion-label>
                <ion-input formControlName="title"></ion-input>
              </ion-item>
            </ion-col>
          </ion-row>
          <ion-row>
            <ion-col>
              <ion-item class="ion-no-padding-start">
                <ion-label color="medium">Width</ion-label>
                <ion-input
                  class="ion-text-end"
                  inputmode="decimal"
                  min="0"
                  step="0.1"
                  type="number"
                  formControlName="width"
                ></ion-input>
              </ion-item>
            </ion-col>
            <ion-col>
              <ion-item>
                <ion-label color="medium">Height</ion-label>
                <ion-input
                  class="ion-text-end"
                  inputmode="decimal"
                  min="0"
                  step="0.1"
                  type="number"
                  formControlName="height"
                ></ion-input>
              </ion-item>
            </ion-col>
          </ion-row>
          <ion-row>
            <ion-col>
              <ion-item class="ion-no-padding-start">
                <ion-label color="medium">Gutter</ion-label>
                <ion-input
                  class="ion-text-end"
                  inputmode="decimal"
                  min="0"
                  step="0.01"
                  type="number"
                  formControlName="gutter"
                ></ion-input>
              </ion-item>
            </ion-col>
            <ion-col>
              <ion-item>
                <ion-label color="medium">Padding</ion-label>
                <ion-input
                  class="ion-text-end"
                  inputmode="decimal"
                  min="0"
                  step="0.01"
                  type="number"
                  formControlName="padding"
                ></ion-input>
              </ion-item>
            </ion-col>
          </ion-row>
          <ion-row>
            <ion-col>
              <ion-item>
                <ion-label color="medium">Measurement units</ion-label>
                <ion-select class="ion-text-end" formControlName="units">
                  <ion-select-option *ngFor="let item of ['inch', 'cm', 'mm']" value="{{ item }}">
                    {{ item }}
                  </ion-select-option>
                </ion-select>
              </ion-item>
            </ion-col>
          </ion-row>
          <ion-row>
            <ion-col>
              <ion-item>
                <ion-label color="medium">Resolution (dpi)</ion-label>
                <ion-select class="ion-text-end" formControlName="resolution">
                  <ion-select-option *ngFor="let dpi of [150, 200, 300, 400, 600, 800, 1000, 1200, 2400]" [value]="dpi">
                    {{ dpi }}
                  </ion-select-option>
                </ion-select>
              </ion-item>
            </ion-col>
          </ion-row>
          <ion-row>
            <ion-col>
              <ion-item>
                <ion-label color="medium">Print base color</ion-label>
                <ion-input
                  [colorPicker]="ionicForm.get('paperColor').value"
                  [style.background]="ionicForm.get('paperColor').value"
                  (colorPickerChange)="ionicForm.get('paperColor').setValue($event)"
                  class="ion-text-end"
                  [cpUseRootViewContainer]='true'
                  cpAlphaChannel="disabled"
                  [cpOKButton]="true"
                  style="margin:3px"
                  id="paper-color"
                  readonly
                ></ion-input>
              </ion-item>
            </ion-col>
          </ion-row>
          <ion-row style="flex-grow: 1">
            <ion-col>
              <ion-content>
                <div class="parent" [innerHTML]="preview"></div>
              </ion-content>
            </ion-col>
          </ion-row>
          <ion-row>
            <ion-col> </ion-col>
            <ion-col>
              <ion-button type="submit" expand="block" [disabled]="!ionicForm.valid">OK</ion-button>
            </ion-col>
          </ion-row>
        </ion-grid>
      </form>
    </ion-content>
  `,
  styles: [
    `
      #paper-color {
        position: absolute;
        right: 0px;
        max-width: 6em;
        border-radius: 5px;
        border-width: 0px;
        min-height: 2.5em;
        readonly: 1;
      }

      ion-input {
        z-index: 2 !important;
      }

      .parent {
        height: 100%;
        border-radius: 5px;
      }

      ion-item {
        --background: transparent;
      }
    `,
  ],
})
export class PaperSizeEditComponent {
  ionicForm: FormGroup;
  preview: SafeHtml;
  private _xmlSerializer = new XMLSerializer();

  constructor(
    private navParams: NavParams,
    private formBuilder: FormBuilder,
    private modalController: ModalController,
    private _psService: PhotoStandardService,
    private _pdService: PrintDefinitionService,
    private _sanitizer: DomSanitizer
  ) {}

  ngOnInit() {
    let pd = this.navParams.data as PrintDefinition;
    if (pd.width == 0 || pd.height == 0) {
      let pdNew = new PrintDefinition();
      pdNew.title = 'Custom paper size';
      pdNew.width = 6;
      pdNew.height = 4;
      pdNew.gutter = 0.1;
      pdNew.paperColor = pd.paperColor;
      pdNew.units = UnitType.inch;
      pdNew.resolution = 600;
      pd = pdNew;
    }

    const gutter = pd.gutter || 0;
    const padding = pd.padding || 0;

    this.ionicForm = this.formBuilder.group({
      title: new FormControl(pd.title),
      width: new FormControl(pd.width, [Validators.required, Validators.min(0.000000001)]),
      height: new FormControl(pd.height, [Validators.required, Validators.min(0.000000001)]),
      gutter: new FormControl(gutter, [Validators.required, Validators.min(0)]),
      padding: new FormControl(padding, [Validators.required, Validators.min(0)]),
      units: new FormControl(pd.units),
      resolution: new FormControl(pd.resolution, [Validators.required, Validators.min(0)]),
      paperColor: new FormControl(pd.paperColor || '#eeeeee'),
    });

    this.ionicForm.valueChanges.subscribe((s) => this.updateView());
    this.updateView();
  }

  getPrintSvg(pd: PrintDefinition) {
    const img = this._psService.getCroppedImageBlobUrl();
    if (!img) return null;
    const ps = this._psService.getSelectedStandard();
    const svg = this._pdService.renderPrintSvg(ps.dimensions, img, pd);
    svg.setAttribute('width', '100%');
    svg.setAttribute('height', '100%');
    svg.setAttribute('preserveAspectRatio', 'xMidYMid meet');
    const xml = this._xmlSerializer.serializeToString(svg);
    return this._sanitizer.bypassSecurityTrustHtml(xml);
  }

  getPrintDefinition() {
    if (!this.ionicForm.valid) {
      return null;
    }
    const pd = this.ionicForm.value as PrintDefinition;
    pd.resolution = +pd.resolution;
    pd.custom = true;
    return pd;
  }

  updateView() {
    const pd = this.getPrintDefinition();
    if (pd) {
      this.preview = this.getPrintSvg(pd) as SafeHtml;
    }
  }

  submitForm() {
    // update
    this.modalController.dismiss(this.getPrintDefinition());
  }

  onCancel() {
    this.modalController.dismiss();
  }
}
