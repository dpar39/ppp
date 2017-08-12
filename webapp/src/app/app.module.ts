import { BrowserModule } from '@angular/platform-browser';
import { NgModule } from '@angular/core';

import { AppComponent } from './app.component';
import { LandmarkEditorComponent } from './landmark-editor/landmark-editor.component';

@NgModule({
  declarations: [
    AppComponent,
    LandmarkEditorComponent
  ],
  imports: [
    BrowserModule
  ],
  providers: [],
  bootstrap: [AppComponent]
})
export class AppModule { }
