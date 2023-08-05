import { Component } from '@angular/core';
import { Router } from '@angular/router';
import { PopoverController } from '@ionic/angular';

@Component({
  selector: 'app-edit-paper-size',
  template: `
    <ion-content>
      <ion-grid>
        <ion-row>
          <ion-col>
            <ion-card>
              <ion-card-content>
                <ion-button (click)="navigate(['/prints'])">Order Online</ion-button>
              </ion-card-content>
            </ion-card>
          </ion-col>
          <ion-col>
            <ion-card>
              <ion-button>Do It Yourself</ion-button>
            </ion-card>
          </ion-col>
        </ion-row>
      </ion-grid>
    </ion-content>
  `,
  styles: [],
})
export class PrepareSelectionComponent {
  constructor(public popoverController: PopoverController, public router: Router) {}

  async navigate(page: string[]) {
    this.router.navigate(page);
    await this.popoverController.dismiss();
  }
}
