import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';
import { Routes, RouterModule } from '@angular/router';

import { IonicModule } from '@ionic/angular';

import { ComplianceChecksPage } from './compliance-checks.page';

const routes: Routes = [
  {
    path: '',
    component: ComplianceChecksPage
  }
];

@NgModule({
  imports: [
    CommonModule,
    FormsModule,
    IonicModule,
    RouterModule.forChild(routes)
  ],
  declarations: [ComplianceChecksPage]
})
export class ComplianceChecksPageModule {}
