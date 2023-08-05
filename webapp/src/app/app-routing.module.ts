import { NgModule } from '@angular/core';
import { PreloadAllModules, RouterModule, Routes } from '@angular/router';

const routes: Routes = [
  {
    path: '',
    redirectTo: 'start',
    pathMatch: 'full',
  },
  {
    path: 'start',
    loadChildren: () => import('./start/start.module').then((m) => m.StartPageModule),
  },
  {
    path: 'standards',
    loadChildren: () => import('./photo-standard/photo-standard.module').then((m) => m.PhotoStandardPageModule),
  },
  {
    path: 'crop',
    loadChildren: () => import('./crop/crop.module').then((m) => m.CropPageModule),
  },
  {
    path: 'edit',
    loadChildren: () => import('./edit/edit.module').then((m) => m.EditPageModule),
  },
  {
    path: 'settings',
    loadChildren: () => import('./settings/settings.module').then((m) => m.SettingsPageModule),
  },
  {
    path: 'prints',
    loadChildren: () => import('./print-definition/print-definition.module').then((m) => m.PrintDefinitionPageModule),
  },
  {
    path: 'preview',
    loadChildren: () => import('./preview-print/preview-print.module').then((m) => m.PreviewPrintPageModule),
  },
  {
    path: 'about',
    loadChildren: () => import('./about/about.module').then((m) => m.AboutPageModule),
  },
  {
    path: 'compliance',
    loadChildren: () =>
      import('./compliance-checks/compliance-checks.module').then((m) => m.ComplianceChecksPageModule),
  },
  {
    path: 'photo-standard',
    loadChildren: () => import('./photo-standard/photo-standard.module').then((m) => m.PhotoStandardPageModule),
  },
  {
    path: 'print-definition',
    loadChildren: () => import('./print-definition/print-definition.module').then((m) => m.PrintDefinitionPageModule),
  },
  {
    path: 'start',
    loadChildren: () => import('./start/start.module').then((m) => m.StartPageModule),
  },
  {
    path: 'order',
    loadChildren: () => import('./order/order.module').then((m) => m.OrderPageModule),
  },
];

@NgModule({
  imports: [RouterModule.forRoot(routes, { preloadingStrategy: PreloadAllModules, relativeLinkResolution: 'legacy' })],
  exports: [RouterModule],
})
export class AppRoutingModule {}
