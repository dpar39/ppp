import { CUSTOM_ELEMENTS_SCHEMA, NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { Routes, RouterModule } from '@angular/router';
import { IonicModule } from '@ionic/angular';

import { PreviewPrintComponent } from './preview-print.page';
import { PanZoomImageComponent } from '../shared/pan-zoom-image.component';

const routes: Routes = [
  {
    path: '',
    component: PreviewPrintComponent,
  },
];

@NgModule({
  imports: [CommonModule, IonicModule, RouterModule.forChild(routes)],
  declarations: [PreviewPrintComponent, PanZoomImageComponent],
  providers: [],
  schemas: [CUSTOM_ELEMENTS_SCHEMA],
})
export class PreviewPrintPageModule {}
