import { Component, OnInit } from '@angular/core';
import { SettingsService, PrintSettings } from '../services/settings.service';

@Component({
  selector: 'app-settings',
  template: `
    <ion-header>
      <ion-toolbar>
        <ion-buttons slot="start">
          <ion-back-button defaultHref="/"></ion-back-button>
        </ion-buttons>
        <ion-buttons slot="end">
          <ion-menu-button></ion-menu-button>
        </ion-buttons>
        <ion-title>Settings</ion-title>
      </ion-toolbar>
    </ion-header>

    <ion-content>
      <ion-card>
        <ion-card-header>
          <ion-card-subtitle>Application</ion-card-subtitle>
        </ion-card-header>
        <ion-card-content>
          <ion-item-group>
            <ion-item>
              <ion-label>Dark Theme</ion-label>
              <ion-toggle
                slot="end"
                name="darkTheme"
                [checked]="settings.getDarkMode()"
                (ionChange)="changeTheme($event)"
              ></ion-toggle>
              <ion-button *ngIf="false" slot="end" (click)="resetThemePreference()"
                ><ion-icon name="undo"></ion-icon
              ></ion-button>
            </ion-item>
            <ion-item>
              <ion-button expand="block" fill="outline" slot="end" color="primary" (click)="resetAll()">
                <ion-icon name="refresh-outline"></ion-icon>
                <span>Reset all</span>
              </ion-button>
            </ion-item>
          </ion-item-group>
        </ion-card-content>
      </ion-card>
      <ion-card>
        <ion-card-header>
          <ion-card-subtitle>Print layout</ion-card-subtitle>
        </ion-card-header>
        <ion-card-content>
          <ion-item class="ion-no-padding">
            <ion-label>Space between photos</ion-label>
            <ion-input
              class="ion-no-margin"
              style="max-width: 48px"
              slot="end"
              placeholder=""
              maxlength="4"
              min="0.0"
              max="1.0"
              step="0.1"
              type="number"
              [(ngModel)]="printSettings.gutterValue"
              (change)="storePrintSettings()"
            ></ion-input>
            <ion-note slot="end" class="ion-no-margin" style="padding-right:0px">mm</ion-note>
          </ion-item>

          <ion-item class="ion-no-padding">
            <ion-radio-group
              [value]="this.printSettings.centerAlign ? 'CENTER' : 'TOP_LEFT'"
              (ionChange)="alignmentChanged($event)"
            >
              <ion-list-header class="ion-no-margin ion-no-padding">
                <ion-label>Picture alignment</ion-label>
              </ion-list-header>

              <ion-item class="ion-no-padding">
                <ion-label>Center</ion-label>
                <ion-radio slot="start" value="CENTER"></ion-radio>
              </ion-item>
              <ion-item class="ion-no-padding">
                <ion-label>Top Left</ion-label>
                <ion-radio slot="start" value="TOP_LEFT"></ion-radio>
              </ion-item>
            </ion-radio-group>
          </ion-item>
          <ion-item class="ion-no-padding">
            <ion-label>Print base color</ion-label>
            <input
              style="margin:3px"
              id="paperColor"
              [style.background]="printSettings.paperColor"
              [colorPicker]="printSettings.paperColor"
              (colorPickerChange)="onColorChanged($event)"
              [cpUseRootViewContainer]="true"
              cpAlphaChannel="disabled"
              [cpOKButton]="true"
              cpPresetLabel=""
              [cpPresetColors]="['#fff', '#ddd', '#aaa', '#111']"
            />
          </ion-item>
        </ion-card-content>
      </ion-card>
    </ion-content>
  `,
  styles: [
    `
      #paperColor {
        max-width: 6em;
        border-radius: 5px;
        border-width: 0px;
        min-height: 2.5em;
        readonly: 1;
      }
    `,
  ],
})
export class SettingsPage implements OnInit {
  pictureAlignment: string;
  printSettings: PrintSettings;
  printBaseColor: string;

  constructor(public settings: SettingsService) {
    this.readSettings();
  }

  readSettings() {
    this.printSettings = this.settings.getPrintSettings();
    this.pictureAlignment = this.printSettings.centerAlign ? 'CENTER' : 'TOP_LEFT';
  }

  ngOnInit() {}

  setDarkTheme(darkMode: boolean) {
    this.settings.setDarkMode(darkMode);
    document.body.classList.toggle('dark', darkMode);
  }

  changeTheme(event) {
    this.setDarkTheme(event.target.checked);
  }
  resetThemePreference() {
    this.settings.resetAllSettings();
    this.setDarkTheme(this.settings.getDarkMode());
  }

  storePrintSettings(): void {
    this.settings.setPrintSettings(this.printSettings);
  }

  alignmentChanged(event) {
    this.printSettings.centerAlign = event.detail.value == 'CENTER';
    this.storePrintSettings();
  }

  resetAll() {
    this.settings.resetAllSettings();
    this.readSettings();
  }

  onColorChanged(color) {
    this.printSettings.paperColor = color;
    this.storePrintSettings();
  }
}
