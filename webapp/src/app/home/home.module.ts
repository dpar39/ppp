import {NgModule} from '@angular/core';
import {CommonModule} from '@angular/common';
import {FormsModule} from '@angular/forms';
import {IonicModule} from '@ionic/angular';
import {RouterModule, Routes} from '@angular/router';

import {HttpClientModule} from '@angular/common/http';

import {HomePage} from './home.page';
import {LandmarkEditorComponent} from './landmark-editor.component';
import {PrintDefinitionSelectorComponent} from './print-definition-selector.component';
import {PhotoStandardSelectorComponent} from './photo-standard-selector.component';
import {BackEndService} from '../services/backend.service';
import {OnlyNumberDirective} from '../directives/onlynumbers-directive';

const routes: Routes = [
    {
        path: '',
        component: HomePage
    }
];

@NgModule({
    imports: [CommonModule, FormsModule, IonicModule, RouterModule.forChild(routes), HttpClientModule],
    declarations: [
        HomePage,
        LandmarkEditorComponent,
        PhotoStandardSelectorComponent,
        PrintDefinitionSelectorComponent,
        OnlyNumberDirective
    ],
    providers: [BackEndService]
})
export class HomePageModule {}
