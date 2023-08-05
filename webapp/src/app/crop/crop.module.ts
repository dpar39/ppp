import { NgModule, CUSTOM_ELEMENTS_SCHEMA } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';
import { IonicModule } from '@ionic/angular';
import { RouterModule, Routes } from '@angular/router';

import { CropPage } from './crop.page';
import { LandmarkEditorComponent } from './landmark-editor.component';
import { OnlyNumberDirective } from '../shared/only-numbers.directive';

import { SelectorsModule } from '../selectors/selectors.module';
import { PrepareSelectionComponent } from './prepare-selection.component';

const routes: Routes = [
  {
    path: '',
    component: CropPage,
  },
];

@NgModule({
  imports: [CommonModule, FormsModule, IonicModule, RouterModule.forChild(routes), SelectorsModule],
  declarations: [CropPage, LandmarkEditorComponent, OnlyNumberDirective, PrepareSelectionComponent],
  entryComponents: [],
  providers: [],
  schemas: [CUSTOM_ELEMENTS_SCHEMA],
})
export class CropPageModule {}
