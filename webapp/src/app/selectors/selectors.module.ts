import { CommonModule } from '@angular/common';
import { CUSTOM_ELEMENTS_SCHEMA, NgModule } from '@angular/core';
import { IonicModule } from '@ionic/angular';
import { PhotoStandardSelectorComponent } from './photo-standard-selector.component';

@NgModule({
  imports: [CommonModule, IonicModule],
  declarations: [PhotoStandardSelectorComponent],
  entryComponents: [],
  providers: [],
  exports: [PhotoStandardSelectorComponent],
  schemas: [CUSTOM_ELEMENTS_SCHEMA],
})
export class SelectorsModule {}
