import { BrowserModule } from '@angular/platform-browser';
import { NgModule } from '@angular/core';
import { NgbModule } from '@ng-bootstrap/ng-bootstrap';

import { HttpModule } from '@angular/http';

import { AppComponent } from './app.component';
import { LandmarkEditorComponent } from './landmark-editor/landmark-editor.component';
import { PassportStandardSelectorComponent } from './passport-standard-selector/passport-standard-selector.component';



@NgModule({
  declarations: [
    AppComponent,
    LandmarkEditorComponent,
    PassportStandardSelectorComponent
  ],
  imports: [
    BrowserModule,
    NgbModule,
    HttpModule
  ],
  providers: [
  ],
  bootstrap: [AppComponent]
})
export class AppModule { }
