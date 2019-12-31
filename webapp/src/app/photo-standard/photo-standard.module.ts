import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';
import { Routes, RouterModule } from '@angular/router';

import { IonicModule } from '@ionic/angular';

import { PhotoStandardPage } from './photo-standard.page';
import { PhotoStandardService } from '../services/photo-standard.service';

const routes: Routes = [
  {
    path: '',
    component: PhotoStandardPage
  }
];

@NgModule({
  imports: [
    CommonModule,
    FormsModule,
    IonicModule,
    RouterModule.forChild(routes)
  ],
  declarations: [PhotoStandardPage],
  providers: []
})
export class PhotoStandardPageModule {}
