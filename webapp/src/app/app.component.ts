import { Component } from '@angular/core';

import { Platform } from '@ionic/angular';
import { SplashScreen } from '@ionic-native/splash-screen/ngx';
import { StatusBar } from '@ionic-native/status-bar/ngx';

@Component({
  selector: 'app-root',
  template: `
    <ion-app>
      <ion-split-pane when="xl">
        <ion-menu type="overlay" side="end" contentId="my-content">
          <ion-header>
            <ion-toolbar>
              <ion-title>Menu</ion-title>
            </ion-toolbar>
          </ion-header>
          <ion-content>
            <ion-list>
              <ion-menu-toggle auto-hide="false" *ngFor="let p of appPages">
                <ion-item [routerDirection]="'root'" [routerLink]="[p.url]">
                  <ion-icon slot="start" [name]="p.icon"></ion-icon>
                  <ion-label>
                    {{ p.title }}
                  </ion-label>
                </ion-item>
              </ion-menu-toggle>
            </ion-list>
          </ion-content>
        </ion-menu>
        <ion-router-outlet id="my-content"></ion-router-outlet>
      </ion-split-pane>
    </ion-app>
  `,
  styleUrls: []
})
export class AppComponent {
  public appPages = [
    {
      title: 'Home',
      url: '/home',
      icon: 'home'
    },
    {
      title: 'Settings',
      url: '/settings',
      icon: 'settings'
    },
    {
      title: 'App Info',
      url: '/about',
      icon: 'information-circle-outline'
    }
  ];

  constructor(private platform: Platform, private splashScreen: SplashScreen, private statusBar: StatusBar) {
    this.initializeApp();
  }

  initializeApp() {
    this.platform.ready().then(() => {
      this.statusBar.styleDefault();
      this.splashScreen.hide();
    });
  }
}
