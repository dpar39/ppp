import {BrowserModule} from '@angular/platform-browser';
import {NgModule} from '@angular/core';
import {FormsModule} from '@angular/forms';
import {HttpClientModule} from '@angular/common/http';
import {TooltipModule} from 'ngx-bootstrap/tooltip';
import {ModalModule} from 'ngx-bootstrap/modal';
import {SelectModule} from 'ng2-select';

import {AppComponent} from './app.component';
import {LandmarkEditorComponent} from './components/landmark-editor.component';
import {PassportStandardSelectorComponent} from './components/photo-standard-selector.component';
import {PrintDefinitionSelectorComponent} from './components/print-definition-selector.component';

import {BackEndService} from './services/back-end.service';
import {OnlyNumberDirective} from './directives/onlynumbers-directive';
import {environment} from '../environments/environment';
import {ServiceWorkerModule} from '@angular/service-worker';

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
        HttpClientModule,
        TooltipModule.forRoot(),
        ModalModule.forRoot(),
        SelectModule,
        ServiceWorkerModule.register('ngsw-worker.js', {enabled: environment.production})
    ],
    providers: [BackEndService],
    bootstrap: [AppComponent]
})
export class AppModule {}
