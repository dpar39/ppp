import { BrowserModule } from '@angular/platform-browser';
import { NgModule } from '@angular/core';
import { HttpModule } from '@angular/http';


import { MatButtonModule, MatCardModule} from '@angular/material';

import { AppComponent } from './app.component';
import { LandmarkEditorComponent } from './landmark-editor/landmark-editor.component';
import { PassportStandardSelectorComponent } from './passport-standard-selector/passport-standard-selector.component';
import { BackEndService } from './services/back-end.service';



@NgModule({
  declarations: [
    AppComponent,
    LandmarkEditorComponent,
    PassportStandardSelectorComponent
  ],
  imports: [
    BrowserModule,
    HttpModule,
    MatButtonModule,
    MatCardModule
  ],
  providers: [
    BackEndService
  ],
  bootstrap: [AppComponent]
})
export class AppModule { }
