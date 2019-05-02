import {BrowserModule} from '@angular/platform-browser';
import {NgModule} from '@angular/core';
import {HttpModule} from '@angular/http';

import {BsDropdownModule} from 'ngx-bootstrap/dropdown';
import {TooltipModule} from 'ngx-bootstrap/tooltip';
import {ModalModule} from 'ngx-bootstrap/modal';

import {AppComponent} from './app.component';
import {LandmarkEditorComponent} from './components/landmark-editor.component';
import {PassportStandardSelectorComponent} from './components/passport-standard-selector.component';
import {BackEndService} from './services/back-end.service';
import {OnlyNumberDirective} from './directives/onlynumbers-directive';
import {Platform} from '@ionic/angular';
import {HttpClientModule} from '@angular/common/http';

@NgModule({
    declarations: [AppComponent, LandmarkEditorComponent, PassportStandardSelectorComponent, OnlyNumberDirective],
    imports: [
        BrowserModule,
        HttpModule,
        HttpClientModule,
        BsDropdownModule.forRoot(),
        TooltipModule.forRoot(),
        ModalModule.forRoot()
    ],
    providers: [Platform, BackEndService],
    bootstrap: [AppComponent]
})
export class AppModule {}
