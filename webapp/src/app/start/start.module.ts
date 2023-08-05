import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';
import { RouterModule, Routes } from '@angular/router';

import { IonicModule } from '@ionic/angular';

import { StartPage } from './start.page';
import { BackEndService } from '../services/backend.service';
import { DragDropDirective } from '../shared/drag-drop.directive';

const routes: Routes = [
  {
    path: '',
    component: StartPage,
  },
];

@NgModule({
  imports: [CommonModule, FormsModule, IonicModule, RouterModule.forChild(routes)],
  declarations: [StartPage, DragDropDirective],
  providers: [],
})
export class StartPageModule {}
