import { Component, OnInit } from '@angular/core';

import { Platform, ToastController } from '@ionic/angular';
import { SettingsService } from './services/settings.service';
import { SwUpdate } from '@angular/service-worker';

@Component({
  selector: 'app-root',
  template: `
    <ion-app>
      <ion-split-pane when="(min-width: 1920px)" contentId="main-content">
        <ion-menu type="overlay" side="end" contentId="main-content">
          <ion-header>
            <ion-toolbar>
              <ion-title>Menu</ion-title>
            </ion-toolbar>
          </ion-header>
          <ion-content>
            <ion-list>
              <ion-menu-toggle auto-hide="false" *ngFor="let p of appPages">
                <ion-item [routerDirection]="'forward'" [routerLink]="[p.url]">
                  <ion-icon slot="start" [name]="p.icon"></ion-icon>
                  <ion-label>
                    {{ p.title }}
                  </ion-label>
                </ion-item>
              </ion-menu-toggle>
            </ion-list>
          </ion-content>
        </ion-menu>
        <ion-router-outlet id="main-content">Loading ...</ion-router-outlet>
      </ion-split-pane>
    </ion-app>
  `,
  styles: [],
})
export class AppComponent implements OnInit {
  public appPages = [
    {
      title: 'Start',
      url: '/start',
      icon: 'home',
    },
    {
      title: 'Settings',
      url: '/settings',
      icon: 'settings',
    },
    {
      title: 'App Info',
      url: '/about',
      icon: 'information-circle-outline',
    },
  ];

  constructor(
    private platform: Platform,
    private settings: SettingsService,
    private swUpdate: SwUpdate,
    private toastController: ToastController
  ) {
    this.initializeApp();
  }

  ngOnInit(): void {
    if (this.swUpdate.isEnabled) {
      this.swUpdate.available.subscribe(() => {
        this.presentUpdatePrompt();
      });
      this.swUpdate.activated.subscribe((event) => {
        console.log('Old version was', event.previous);
        console.log('New version is', event.current);
      });
    }
  }

  async presentUpdatePrompt() {
    const toast = await this.toastController.create({
      header: 'Update Available',
      message: 'Reload to get the latest and greatest?',
      position: 'top',
      buttons: [
        {
          icon: 'refresh',
          text: 'Reload',
          handler: () => {
            window.location.reload();
          },
        },
        {
          text: 'Dismiss',
          role: 'cancel',
          icon: 'cancel',
          handler: () => {
            console.log('Dismiss button clicked');
          },
        },
      ],
    });
    toast.present();
  }

  initializeApp() {
    this.platform.ready().then(() => {});

    const darkMode = this.settings.getDarkMode();
    document.body.classList.toggle('dark', darkMode);
  }
}
