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
          <ion-toggle slot="end" name="darkTheme" [checked]="settings.getDarkMode()"
          (ionChange)="changeTheme($event)"></ion-toggle>
        </ion-item>
      </ion-item-group>
    </ion-content>
  `,
  styles: [``]
})
export class SettingsPage implements OnInit {
  constructor(public settings: SettingsService) {}

  ngOnInit() {}

  changeTheme(event) {
    const darkTheme = event.target.checked;
    this.settings.setDarkMode(darkTheme);
    document.body.classList.toggle('dark', darkTheme);

    // const prefersDark = window.matchMedia('(prefers-color-scheme: dark)');
    // toggleDarkTheme(prefersDark.matches);

    // // Listen for changes to the prefers-color-scheme media query
    // prefersDark.addListener(mediaQuery => toggleDarkTheme(mediaQuery.matches));

    // // Add or remove the "dark" class based on if the media query matches
    // function toggleDarkTheme(shouldAdd: boolean) {
    //     document.body.classList.toggle('dark', shouldAdd);
    // }
  }
}
