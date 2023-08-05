import { CUSTOM_ELEMENTS_SCHEMA, NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';
import { Routes, RouterModule } from '@angular/router';
import { IonicModule } from '@ionic/angular';

import { PhotoStandardPage } from './photo-standard.page';
import { SelectorsModule } from '../selectors/selectors.module';

const routes: Routes = [
  {
    path: '',
    component: PhotoStandardPage,
  },
];

@NgModule({
  imports: [CommonModule, FormsModule, IonicModule, RouterModule.forChild(routes), SelectorsModule],
  declarations: [PhotoStandardPage],
  providers: [],
  schemas: [CUSTOM_ELEMENTS_SCHEMA],
})
export class PhotoStandardPageModule {}
