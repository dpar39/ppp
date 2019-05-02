import {BrowserModule} from '@angular/platform-browser';
import {NgModule} from '@angular/core';
import {HttpModule} from '@angular/http';
import {FormsModule} from '@angular/forms';

import {TooltipModule} from 'ngx-bootstrap/tooltip';
import {ModalModule} from 'ngx-bootstrap/modal';
import {SelectModule} from 'ng2-select';

import {AppComponent} from './app.component';
import {LandmarkEditorComponent} from './components/landmark-editor.component';
import {PassportStandardSelectorComponent} from './components/passport-standard-selector.component';
import {PrintDefinitionSelectorComponent} from './components/print-definition-selector.component';

import {BackEndService} from './services/back-end.service';
import {OnlyNumberDirective} from './directives/onlynumbers-directive';
import {Platform} from '@ionic/angular';
import {HttpClientModule} from '@angular/common/http';

@NgModule({
    declarations: [
        AppComponent,
        LandmarkEditorComponent,
        PassportStandardSelectorComponent,
        PrintDefinitionSelectorComponent,
        OnlyNumberDirective
    ],
    imports: [
        FormsModule,
        BrowserModule,
        HttpModule,
        HttpClientModule,
        TooltipModule.forRoot(),
        ModalModule.forRoot(),
        SelectModule
    ],
    providers: [Platform, BackEndService],
    bootstrap: [AppComponent]
})
export class AppModule {}
