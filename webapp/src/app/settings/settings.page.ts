import { Component, OnInit } from '@angular/core';
import { SettingsService } from '../services/settings.service';

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
      <ion-item-group>
        <ion-item>
          <ion-label>Dark Theme</ion-label>
          <ion-toggle
            slot="end"
            name="darkTheme"
            [checked]="settings.getDarkMode()"
            (ionChange)="changeTheme($event)"
          ></ion-toggle>
          <ion-button *ngIf='false' slot="end" (click)="resetThemePreference()"
            ><ion-icon name="undo"></ion-icon
          ></ion-button>
        </ion-item>
      </ion-item-group>
    </ion-content>
  `,
  styles: [``]
})
export class SettingsPage implements OnInit {
  constructor(public settings: SettingsService) {}

  ngOnInit() {}

  setDarkTheme(darkMode: boolean) {
    this.settings.setDarkMode(darkMode);
    document.body.classList.toggle('dark', darkMode);
  }

  changeTheme(event) {
    this.setDarkTheme(event.target.checked);
  }
  resetThemePreference() {
    this.settings.resetThemePreferences();
    this.setDarkTheme(this.settings.getDarkMode());
  }
}
