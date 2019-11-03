import { Component, OnInit } from '@angular/core';
import { AppBuildInfo, appBuildInfo } from 'src/environments/app-build-info';

@Component({
  selector: 'app-about',
  template: `
    <ion-header>
      <ion-toolbar>
        <ion-buttons slot="start">
          <ion-back-button defaultHref="/"></ion-back-button>
        </ion-buttons>
        <ion-buttons slot="end">
          <ion-menu-button></ion-menu-button>
        </ion-buttons>
        <ion-title>App Info</ion-title>
      </ion-toolbar>
    </ion-header>

    <ion-content>
      <!-- Item Dividers in a List -->
      <ion-list>
        <ion-item-divider>
          <ion-label class="ion-bold">
            About
          </ion-label>
        </ion-item-divider>
        <ion-item>
          <ion-text color="medium ion-padding"
            >This app allows you to prepare photos for passport applications, and many other photo ID
            documents such as visas, employment authorization documents, etc.</ion-text
          >
        </ion-item>
        <ion-item-divider>
          <ion-label class="ion-bold">
            App Build Info
          </ion-label>
        </ion-item-divider>

        <ion-grid>
          <ion-row>
            <ion-col class="key">Version</ion-col>
            <ion-col>{{ appBuildInfo.versionString }}</ion-col>
          </ion-row>
          <ion-row>
            <ion-col class="key">Build Time</ion-col>
            <ion-col>{{ getBuildDateTime() }}</ion-col>
          </ion-row>
          <ion-row>
            <ion-col class="key">Commit ID</ion-col>
            <ion-col>{{ appBuildInfo.commitId }} ({{ appBuildInfo.branch }})</ion-col>
          </ion-row>
        </ion-grid>

        <ion-item-divider>
          <ion-label>
            Developer Information
          </ion-label>
        </ion-item-divider>

        <ion-grid>
          <ion-row>
            <ion-col class="key">Developer</ion-col>
            <ion-col>Darien Pardinas Diaz</ion-col>
          </ion-row>
          <ion-row>
            <ion-col class="key">
              Contact
            </ion-col>
            <ion-col>dpar39@gmail.com</ion-col>
          </ion-row>
          <ion-row>
            <ion-col class="ion-text-center">
              <a href="https://www.linkedin.com/in/darien-pardinas-diaz-2ab47579/" class="socialMediaIcon">
                <ion-icon color="secondary" name="logo-linkedin"></ion-icon>
              </a>
              <a href="https://github.com/dpar39/" class="socialMediaIcon">
                <ion-icon color="dark" name="logo-github"></ion-icon>
              </a>
            </ion-col>
          </ion-row>
          <ion-row>
            <ion-col>
              <ion-button expand="block"
                ><ion-icon name="bug" class="ion-padding-end"></ion-icon>Submit bug</ion-button
              >
            </ion-col>
            <ion-col>
              <a
                href="https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=darien.pardinas%40gmail.com&currency_code=USD&source=url"
              >
                <ion-button expand="block" color="success">
                  <ion-icon name="cafe" class="ion-padding-end"></ion-icon>Donate
                </ion-button>
              </a>
            </ion-col>
            <ion-row>
              <ion-col> </ion-col>
            </ion-row>
          </ion-row>
        </ion-grid>
      </ion-list>
    </ion-content>
  `,
  styles: [
    `
      .key {
        padding-right: 10px;
        text-align: right;
        color: var(--ion-color-primary);
      }

      .socialMediaIcon {
        font-size: 48px;
        padding: 0px 10px;
      }
    `
  ]
})
export class AboutPage implements OnInit {
  constructor() {}

  appBuildInfo: AppBuildInfo;

  ngOnInit() {
    this.appBuildInfo = appBuildInfo;
  }

  getBuildDateTime() {
    const myDate = new Date(appBuildInfo.timeStamp);
    return myDate.toLocaleString();
  }
}
