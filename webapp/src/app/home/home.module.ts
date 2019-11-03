import { NgModule, CUSTOM_ELEMENTS_SCHEMA } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';
import { IonicModule } from '@ionic/angular';
import { RouterModule, Routes } from '@angular/router';

import { HomePage } from './home.page';
import { LandmarkEditorComponent } from './landmark-editor.component';
import { PrintDefinitionSelectorComponent } from './print-definition-selector.component';
import { PhotoStandardSelectorComponent } from './photo-standard-selector.component';
import { OnlyNumberDirective } from '../directives/onlynumbers-directive';

import { PhotoStandardService } from '../services/photo-standard.service';
import { BackEndService } from '../services/backend.service';

const routes: Routes = [
  {
    path: '',
    component: HomePage
  }
];

@NgModule({
  imports: [CommonModule, FormsModule, IonicModule, RouterModule.forChild(routes)],
  declarations: [
    HomePage,
    LandmarkEditorComponent,
    PhotoStandardSelectorComponent,
    PrintDefinitionSelectorComponent,
    OnlyNumberDirective
  ],
  providers: [BackEndService, PhotoStandardService],
  schemas: [CUSTOM_ELEMENTS_SCHEMA]
})
export class HomePageModule {}
